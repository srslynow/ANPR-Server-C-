#include <thread>
#include <ctime>

#include "Server.h"

#include "SFML/Network.hpp"

#define BUF_LEN 1472
#define ENCODE_QUALITY 80

Server::Server()
{
	_logger = spdlog::get("console");
	_logger->info("Starting server");
	std::thread receiveThread([this] { this->receiveLoop(); });
	receiveThread.detach();
	//receiveThread.join();
}


Server::~Server()
{
}

void Server::receiveLoop()
{
	sf::UdpSocket sock;

	// bind the socket to a port
	if (sock.bind(6112) != sf::Socket::Done)
	{
		_logger->critical("Could not bind on UDP port 6112");
	}

	char buffer[BUF_LEN]; // Buffer for echo string
	std::string sourceAddress; // Address of datagram source
	unsigned short sourcePort; // Port of datagram source
	std::size_t recvMsgSize;
	sf::IpAddress sender;

	_logger->info("Started UDP service");

	while (1) {
		if (sock.receive(buffer, BUF_LEN, recvMsgSize, sender, sourcePort) != sf::Socket::Done)
		{
			_logger->warn("Failed receiving from UDP sock");
		}
		sourceAddress = sender.toString();

		if (recvMsgSize == sizeof(int))
		{
			int total_size = ((int *)buffer)[0];
			// Sanity check, image # of bytes should be > 0
			if (total_size <= 0)
			{
				_logger->warn("Invalid expected packet size: {}, from ip {} and port {}", total_size, sourceAddress, sourcePort);
			}
			// Add ip/port to database
			else
			{
				_logger->debug("Expecting image of length: {}, from ip {} and port {}", total_size, sourceAddress, sourcePort);
				setImgData(sourceAddress, sourcePort, total_size);
			}
		}
		else
		{
			ImageData * imgData = getImgData(sourceAddress, sourcePort);
			if (!imgData)
			{
				_logger->warn("Ip {} and port {} combination was not yet known", sourceAddress, sourcePort);
			}
			else
			{
				if (imgData->currentLength + recvMsgSize > imgData->expectedLength)
				{
					_logger->warn("Exceeding earlier stated image length ({} vs {}), aborting.. ", (imgData->currentLength + recvMsgSize), imgData->expectedLength);
				}
				else
				{
					//_logger->debug("Add {} bytes to imagedata", recvMsgSize);
					imgData->imgData.insert(imgData->imgData.end(), buffer, buffer + recvMsgSize);
					imgData->currentLength += recvMsgSize;
					// Check if we completed an image
					if (imgData->currentLength == imgData->expectedLength)
					{
						_logger->debug("Completed receiving an image of {} bytes", imgData->expectedLength);
						cv::Mat frame = cv::imdecode(imgData->imgData, CV_LOAD_IMAGE_COLOR);
						if (frame.empty())
						{
							_logger->warn("Could not decode image from ip {} and port {}", sourceAddress, sourcePort);
						}
						else
						{
							image_ready.emit(frame);
						}
					}
				}
			}
		}
	}
}

ImageData * Server::getImgData(std::string ipaddr, int port)
{
	// Search if we created this datacontainer already
	for (int i = 0; i < dataContainer.size(); i++)
	{
		if (dataContainer[i].ipAddr == ipaddr && dataContainer[i].port == port)
			return &dataContainer[i];
	}
	// Nope, doesn't exist, let's create a new one
	return nullptr;
}

void Server::setImgData(std::string ipaddr, int port, int expectedLength)
{
	// Perform garbage collection every 1.0 second at most
	if (double(std::clock() - last_gc) / CLOCKS_PER_SEC > 1.0)
	{
		doGarbageCollect();
		last_gc = std::clock();
	}

	auto currData = getImgData(ipaddr, port);
	// Found existing data container
	if (currData)
	{
		currData->imgData.clear();
		currData->currentLength = 0;
		currData->expectedLength = expectedLength;
		currData->imgData.reserve(expectedLength);
	}
	// Did not exist, create new entry in container
	else
	{
		currData = new ImageData;
		currData->ipAddr = ipaddr;
		currData->port = port;
		currData->expectedLength = expectedLength;
		currData->imgData.reserve(expectedLength);
		currData->initTime = std::clock();
		dataContainer.push_back(*currData);
	}
}

void Server::doGarbageCollect()
{
	for (auto it = dataContainer.begin(); it != dataContainer.end();)
	{
		if (double(std::clock() - it->initTime) / CLOCKS_PER_SEC > 1.0)
		{
			it = dataContainer.erase(it);
		}
		else
			it++;
	}
}

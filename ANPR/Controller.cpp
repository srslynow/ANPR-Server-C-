#include <thread>
#include <random>
#include <ctime>
#include <cstdlib>
#include <fstream>

#include <experimental/filesystem>

#include "Controller.h"

namespace fs = std::experimental::filesystem;

Controller::Controller(std::shared_ptr<Model> model, std::shared_ptr<View> view)
{
	// initialize private variables
	_model = model;
	_view = view;
	_logger = spdlog::get("console");
	_server = std::make_shared<Server>();
	_buf = std::make_shared<ImageBuffer>(20, true);
	// bind events
	_server->image_ready.bind(&Controller::serverReceiveImage, this);
	// start image processing threads
	// these block the main application thread until exit
	startProcessingThreads();
}


Controller::~Controller()
{
}


void Controller::startProcessingThreads()
{
	// lookup how much threads this processor can handle
	unsigned int threadNum = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;

	// create and start our new threads
	for (unsigned int i = 0; i < threadNum; i++)
	{
		// processing thread, shares buffer with all other threads
		ProcessingThread * temp = new ProcessingThread(_buf);
		// bind callback function
		temp->new_flash.bind(&Controller::plateFlashed, this);
		// start processing
		threads.push_back(std::thread([&] {
			temp->run();
		}));
	}

	for (unsigned int i = 0; i < threadNum; i++)
	{
		threads[i].join();
	}
}

void Controller::serverReceiveImage(cv::Mat image)
{
	_buf->addFrame(image);
}

void Controller::plateFlashed(std::shared_ptr<Flash> flash)
{
	fs::path output_dir("output/unconfirmed");

	for (int i = 0; i < flash->plates.size(); i++)
	{
		std::string rnd_dir = generateRandomString(6);
		output_dir /= rnd_dir;

		_logger->debug("Writing result to dir {} ", output_dir.string());

		if (!fs::exists(output_dir))
		{
			fs::create_directories(output_dir);
			// write to txt file
			std::ofstream plateTxt(output_dir / ("result.txt"));
			plateTxt << flash->plates[i]->plateStr;
			plateTxt.close();
			// write images
			cv::imwrite(cv::String((output_dir / "orig.png").string()), flash->origImage);
			cv::imwrite(cv::String((output_dir / "lp_feedback.png").string()), flash->feedBackImage);
			cv::imwrite(cv::String((output_dir / "lp.png").string()), flash->plates[i]->origImage);
			cv::imwrite(cv::String((output_dir / "seg.png").string()), flash->plates[i]->segImage);
			cv::imwrite(cv::String((output_dir / "char_feedback.png").string()), flash->plates[i]->feedBackImage);
			for (int j = 0; j < flash->plates[i]->charLocs.size(); j++)
			{
				cv::imwrite(cv::String((output_dir / ("char_" + std::to_string(j) + ".png")).string()), flash->plates[i]->charImages[j]);
			}
		}
	}
}

std::string generateRandomString(size_t length)
{
	// seed the random number generator
	srand(time(0));
	auto randchar = []() -> char
	{
		const char charset[] =
			"0123456789"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz";
		const size_t max_index = (sizeof(charset) - 1);
		return charset[rand() % max_index];
	};
	std::string str(length, 0);
	std::generate_n(str.begin(), length, randchar);
	return str;
}

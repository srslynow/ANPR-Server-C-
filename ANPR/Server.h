#pragma once

#include "spdlog/spdlog.h"
#include "opencv2/opencv.hpp"
#include "signal.h"

struct ImageData
{
	std::string ipAddr;
	int port;
	std::vector<char> imgData;
	int expectedLength = 0;
	int currentLength = 0;
	clock_t initTime;
};

class Server
{
public:
	Server();
	~Server();
	Signal<cv::Mat> image_ready;

private:
	void receiveLoop();
	ImageData * getImgData(std::string ipaddr, int port);
	void setImgData(std::string ipaddr, int port, int expectedLength);
	void doGarbageCollect();

private:
	std::vector<ImageData> dataContainer;
	std::shared_ptr<spdlog::logger> _logger;
	clock_t last_gc = std::clock();
};


#pragma once
#include <memory>

#include "signal.h"
#include "spdlog/spdlog.h"
#include "Model.h"
#include "View.h"
#include "Server.h"
#include "ImageBuffer.h"
#include "ProcessingThread.h"

#include "opencv2/core.hpp"

class Controller : public SigSlotBase
{
public:
	Controller(std::shared_ptr<Model> model, std::shared_ptr<View> view);
	~Controller();

private:
	std::shared_ptr<Model> _model;
	std::shared_ptr<View> _view;
	std::shared_ptr<Server> _server;
	std::shared_ptr<ImageBuffer> _buf;
	std::shared_ptr<spdlog::logger> _logger;
	
public:
	void startProcessingThreads();
	void serverReceiveImage(cv::Mat image);
	void plateFlashed(std::shared_ptr<Flash> flash);
};

std::string generateRandomString(size_t length);
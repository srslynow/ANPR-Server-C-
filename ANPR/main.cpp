#include <iostream>
#include <memory>

#include "spdlog/spdlog.h"
#include "opencv2/core.hpp"
#include "signal.h"

#include "Model.h"
#include "Controller.h"
#include "View.h"
#include "VariableManager.h"

int main(int, char*[])
{
	// instantiate console logger
	auto console = spdlog::stdout_logger_mt("console");

	// change logging level if we're in debug mode
	if (VariableManager::instance().getDebugMode())
	{
		spdlog::set_level(spdlog::level::debug);
		console->info("Debug mode is ON");
	}

	// tell the client that we're starting the execution
	// and if we're using a GPU or not
	console->info("Starting ANPR");
	if (VariableManager::instance().useGPU())
		console->info("Using GPU calculations");
	else
		console->info("Using CPU calculations");

	// start model/view/controller classes
	auto view = std::make_shared<View>();
	auto model = std::make_shared<Model>();
	Controller controller(model, view);

	// return success on exit
	return 0;
}
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
	// log more if we're in release mode
	auto console = spdlog::stdout_color_mt("console");
	//spdlog::set_level(spdlog::level::debug);


	console->info("Starting ANPR");
	if (VariableManager::instance().useGPU())
		console->info("Using GPU calculations");
	else
		console->info("Using CPU calculations");

	auto view = std::make_shared<View>();
	auto model = std::make_shared<Model>();
	Controller controller(model, view);

	return 0;
}
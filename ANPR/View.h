#pragma once

#include "spdlog/spdlog.h"


class View
{
public:
	View();
	~View();

private:
	std::shared_ptr<spdlog::logger> _logger;
};


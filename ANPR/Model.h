#pragma once

#include "spdlog/spdlog.h"


class Model
{
public:
	Model();
	~Model();

private:
	std::shared_ptr<spdlog::logger> _logger;
};


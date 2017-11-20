#pragma once

#include <vector>

#include "opencv2/core.hpp"

class LPDetection
{
public:
	LPDetection();
	~LPDetection();
	virtual std::vector<cv::Mat> detect(cv::Mat img) { return std::vector<cv::Mat>(); };
};


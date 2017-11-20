#pragma once
#include <vector>

#include "LPDetection.h"
class LPCascade :
	public LPDetection
{
public:
	LPCascade();
	~LPCascade();
	virtual std::vector<cv::Mat> detect(cv::Mat img);
};


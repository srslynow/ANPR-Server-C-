#pragma once
#include <string>

#include "ImageBuffer.h"
#include "signal.h"
#include "spdlog/spdlog.h"
#include "opencv2/core.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/dnn.hpp"

struct Plate
{
	std::string plateStr;
	cv::Mat origImage;
	cv::Mat segImage;
	cv::Mat feedBackImage;
	cv::Rect plateLoc;
	std::vector<cv::Rect> charLocs;
	std::vector<cv::Mat> charImages;
};

struct Flash
{
	cv::Mat origImage;
	cv::Mat feedBackImage;
	std::vector<std::shared_ptr<Plate>> plates;
};

class ProcessingThread
{
public:
	ProcessingThread(std::shared_ptr<ImageBuffer> buf);

	~ProcessingThread() {};

	void run();

private:
	template<typename T>
	std::vector<cv::Rect> findPlateCandidates(T frame);

	template<typename T>
	T segmentCandidatePlate(T candidatePlate);

	template<typename T>
	std::vector<cv::Rect> isolateCandidateCharacters(T segmentedCandidatePlate);

	template<typename T>
	T imgToFixedSize(T img);

	static std::vector<char> readClassNames(const char * filename);

	static void createFeedBackImage(std::shared_ptr<Flash> flash);

public:
	Signal<std::shared_ptr<Flash>> new_flash;

private:
	std::shared_ptr<spdlog::logger> _logger;
	std::shared_ptr<ImageBuffer> _buf;
	cv::CascadeClassifier cascadeClassifier;
	cv::dnn::Net characterClassifier;
	std::vector<char> _classlabels;
};


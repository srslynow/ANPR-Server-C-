#include <ctime>
#include <fstream>

#include "ProcessingThread.h"
#include "VariableManager.h"
#include "opencv2/highgui.hpp"


ProcessingThread::ProcessingThread(std::shared_ptr<ImageBuffer> buf)
{
	_logger = spdlog::get("console");
	_buf = buf;
	cascadeClassifier.load("cascade.xml");
	std::string modelTxt = "characterNet.prototxt";
	std::string modelBin = "characterModel.caffemodel";
	characterClassifier = cv::dnn::readNetFromCaffe(modelTxt, modelBin);
	_classlabels = readClassNames("labels.txt");
}

void ProcessingThread::run()
{
	while (true)
	{
		cv::Mat frame = _buf->getFrame();
		if (frame.channels() > 1)
			cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);

		std::vector<cv::Rect> plateRectCandidates;

		// ############################## find plate candidates ###################################
		// use GPU computation
		clock_t beginFind = std::clock();
		if (VariableManager::instance().useGPU())
		{
			cv::UMat uFrame;
			frame.copyTo(uFrame);
			plateRectCandidates = findPlateCandidates(uFrame);
		}
		// use CPU computation
		else
		{
			plateRectCandidates = findPlateCandidates(frame);
		}
		clock_t endFind = std::clock();
		if (plateRectCandidates.size() > 0)
		{
			_logger->debug("Found {} candidate number plates in {} sec", plateRectCandidates.size(), double(endFind - beginFind) / CLOCKS_PER_SEC);
			// we have some candidates, prepare for output
			std::shared_ptr<Flash> flash = std::make_shared<Flash>();
			flash->origImage = frame;
			for (auto plateRectCandidate : plateRectCandidates)
			{
				// ############################## segment plate candidate ###################################
				clock_t beginSegment = std::clock();
				cv::Mat plateCandidate;
				frame(plateRectCandidate).copyTo(plateCandidate);
				cv::resize(plateCandidate, plateCandidate, cv::Size(512, 112));
				cv::equalizeHist(plateCandidate, plateCandidate);
				auto segmentedPlateCandidate = segmentCandidatePlate(plateCandidate.clone());
				clock_t endSegment = std::clock();
				_logger->debug("Segmented candidate plate in {} sec", double(endSegment - beginSegment) / CLOCKS_PER_SEC);

				// ############################## isolate characters ###################################
				clock_t beginIsolate = std::clock();
				std::vector<cv::Rect> candidateCharBoundRects = isolateCandidateCharacters(segmentedPlateCandidate);
				// sort rectangles by their x-position
				std::sort(candidateCharBoundRects.begin(), candidateCharBoundRects.end(),
					[](const cv::Rect & a, const cv::Rect & b) -> bool
				{
					return a.x < b.x;
				}
				);
				//for (auto i : candidateCharBoundRects)
				//{
				//	_logger->debug("Rect width: {}, height: {}", i.width, i.height);
				//	cv::imshow("img", segmentedPlateCandidate(i));
				//	cv::waitKey(0);
				//}
				std::vector<cv::Rect> charBoundRects;
				std::copy_if(candidateCharBoundRects.begin(), candidateCharBoundRects.end(), std::back_inserter(charBoundRects),
					[](const cv::Rect & a) -> bool
				{
					return a.width > 15 && a.width < 80 && a.height > 40 && a.height < 100;
				}
				);

				// skip this licence plate in case there are less than 5 or more than 12 characters detected
				if (charBoundRects.size() <= 5 || charBoundRects.size() > 12)
					continue;

 				clock_t endIsolate = std::clock();
				_logger->debug("Isolated {} chars in {} sec", charBoundRects.size(), double(endIsolate - beginIsolate) / CLOCKS_PER_SEC);

				// we know we're going to output a plate
				std::shared_ptr<Plate> plate = std::make_shared<Plate>();
				plate->origImage = plateCandidate;
				plate->segImage = segmentedPlateCandidate;
				plate->plateLoc = plateRectCandidate;
				plate->charLocs = charBoundRects;

				// ############################## predict characters ###################################
				clock_t beginPredict = std::clock();
				int sz[] = { 12, 1, 28, 28 };
				cv::Mat blob = cv::Mat(4, sz, CV_32F);
				for (int i = 0; i < charBoundRects.size(); i++)
				{
					auto charBoundRect = charBoundRects[i];
					auto charMat = segmentedPlateCandidate(charBoundRect).clone();
					auto resizedCharMat = imgToFixedSize(charMat);
					plate->charImages.push_back(resizedCharMat.clone());
					resizedCharMat.convertTo(resizedCharMat, CV_32F);
					resizedCharMat.copyTo(cv::Mat(resizedCharMat.rows, resizedCharMat.cols, CV_32F, blob.ptr(i, 0)));
				}
				characterClassifier.setInput(blob, "data");
				auto prob = characterClassifier.forward("softmax");
				std::string plateStr = "";
				for (int i = 0; i < charBoundRects.size(); i++)
				{
					const float * rowBeginPtr = prob.ptr<float>(i);
					const float * rowEndPtr = rowBeginPtr + prob.cols;
					std::vector<float> predData(rowBeginPtr, rowEndPtr);
					auto max_elem_ptr = std::max_element(predData.begin(), predData.end());
					auto classIdx = std::distance(predData.begin(), max_elem_ptr);
					auto prob = predData[classIdx];
					plateStr.append(_classlabels.at(classIdx));
				}
				plate->plateStr = plateStr;
				flash->plates.push_back(plate);

				
				clock_t endPredict = std::clock();
				_logger->debug("Predicted characters in {} sec", double(endPredict - beginPredict) / CLOCKS_PER_SEC);
				_logger->info("Predicted licence plate \"{}\" in a total of {} sec", plateStr, double(endPredict - beginFind) / CLOCKS_PER_SEC);
			}

			createFeedBackImage(flash);
			new_flash.emit(flash);
		}
	}
}

template<typename T>
inline std::vector<cv::Rect> ProcessingThread::findPlateCandidates(T frame)
{
	std::vector<cv::Rect> plateRects;
	cascadeClassifier.detectMultiScale(frame, plateRects, 1.1, 20, 0, cv::Size(75, 15), cv::Size(512, 112));
	return plateRects;
	
}

template<typename T>
T ProcessingThread::segmentCandidatePlate(T candidatePlate)
{
	T candidatePlate2;
	cv::bilateralFilter(candidatePlate, candidatePlate2, 5, 15, 15);
	cv::GaussianBlur(candidatePlate2, candidatePlate, cv::Size(5, 5), 0.0);
	cv::adaptiveThreshold(candidatePlate, candidatePlate, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, CV_THRESH_BINARY, 87, 6);
	return candidatePlate;
}

template<typename T>
std::vector<cv::Rect> ProcessingThread::isolateCandidateCharacters(T segmentedCandidatePlate)
{
	// required because cv::fondContours messes up the input
	T segmentedCandidatePlate2;
	segmentedCandidatePlate.copyTo(segmentedCandidatePlate2);
	// findContours requires white foreground (we use black)
	cv::bitwise_not(segmentedCandidatePlate2, segmentedCandidatePlate2);
	// some parameters for findContours
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	// find all external contours
	cv::findContours(segmentedCandidatePlate2, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	// convert contours to bounding rectangles
	std::vector<cv::Rect> boundRects;
	for (auto contourPoints : contours)
	{
		boundRects.push_back(cv::boundingRect(contourPoints));
	}
	return boundRects;
}

template<typename T>
T ProcessingThread::imgToFixedSize(T img)
{
	int target_height = VariableManager::instance().getCharSize().height - VariableManager::instance().getCharBorder();
	int half_border = (int)(VariableManager::instance().getCharBorder() / 2);

	int width = img.cols,
		height = img.rows;

	auto square = cv::Mat(target_height, target_height, img.type(), cv::Scalar(255));

	int max_dim = (width >= height) ? width : height;
	float scale = ((float)target_height) / max_dim;
	cv::Rect roi;
	if (width >= height)
	{
		roi.width = target_height;
		roi.x = 0;
		roi.height = height * scale;
		roi.y = (target_height - roi.height) / 2;
	}
	else
	{
		roi.y = 0;
		roi.height = target_height;
		roi.width = width * scale;
		roi.x = (target_height - roi.width) / 2;
	}

	cv::resize(img, square(roi), roi.size());
	cv::copyMakeBorder(square, square, half_border, half_border, half_border, half_border, IPL_BORDER_CONSTANT, cv::Scalar(255));
	img = square;
	return img;
}

std::vector<std::string> ProcessingThread::readClassNames(const char *filename = "labels.txt")
{
	std::vector<std::string> classNames;

	std::ifstream fp(filename);
	if (fp.is_open())
	{
		std::string name;
		while (!fp.eof())
		{
			std::getline(fp, name);
			if (name.length())
				classNames.push_back(name.substr(name.find(' ') + 1));
		}

		fp.close();
	}
	return classNames;
}

void ProcessingThread::createFeedBackImage(std::shared_ptr<Flash> flash)
{
	flash->feedBackImage = flash->origImage.clone();
	cv::cvtColor(flash->feedBackImage, flash->feedBackImage, cv::COLOR_GRAY2BGR);
	for (auto plate : flash->plates)
	{
		auto cleanSeg = cv::Mat(plate->segImage.rows, plate->segImage.cols, CV_8U, cv::Scalar(255));
		plate->feedBackImage = plate->origImage.clone();
		cv::cvtColor(plate->feedBackImage, plate->feedBackImage, cv::COLOR_GRAY2BGR);
		cv::rectangle(flash->feedBackImage, plate->plateLoc, cv::Scalar(255, 0, 0), 3);
		for (auto charLoc : plate->charLocs)
		{
			plate->segImage(charLoc).copyTo(cleanSeg(charLoc));
			cv::rectangle(plate->feedBackImage, charLoc, cv::Scalar(255,0,0), 3);
		}
		plate->segImage = cleanSeg;
	}
}

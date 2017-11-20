#ifndef IMAGEBUFFER_H
#define IMAGEBUFFER_H

// Qt header files
#include <thread>
#include <mutex>
#include <deque>
// OpenCV header files
#include <opencv2/core.hpp>
#include "Semaphore.h"

class ImageBuffer
{

public:
    ImageBuffer(int size, bool dropFrame);
    void addFrame(const cv::Mat& frame);
    cv::Mat getFrame();
private:
    std::mutex imageQueueProtect;
    std::deque<cv::Mat> imageQueue;
    Semaphore *freeSlots;
    Semaphore *usedSlots;
    int bufferSize;
    bool dropFrame;
};

#endif // IMAGEBUFFER_H

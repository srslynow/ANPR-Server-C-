#include "ImageBuffer.h"


ImageBuffer::ImageBuffer(int bufferSize, bool dropFrame)
{
	// Semaphore initializations
	freeSlots = new Semaphore(bufferSize);
	usedSlots = new Semaphore(0);
	// Save value of dropFrame to private member
	this->dropFrame = dropFrame;
} // ImageBuffer constructor

void ImageBuffer::addFrame(const cv::Mat& frame)
{
	// If frame dropping is enabled, do not block if buffer is full
	if (dropFrame)
	{
		// Try and acquire semaphore to add frame
		if (freeSlots->tryAcquire())
		{
			// Add frame to queue
			imageQueueProtect.lock();
			imageQueue.push_back(frame);
			imageQueueProtect.unlock();
			// Release semaphore
			usedSlots->notify();
		}
	}
	// If buffer is full, wait on semaphore
	else
	{
		// Acquire semaphore
		freeSlots->wait();
		// Add frame to queue
		imageQueueProtect.lock();
		imageQueue.push_back(frame);
		imageQueueProtect.unlock();
		// Release semaphore
		usedSlots->notify();
	}
} // addFrame()


cv::Mat ImageBuffer::getFrame()
{
	// Acquire semaphores
	usedSlots->wait();
	// Temporary data
	cv::Mat tempFrame;
	// Take frame from queue
	imageQueueProtect.lock();
	tempFrame = imageQueue[0];
	imageQueue.pop_front();
	imageQueueProtect.unlock();
	// Release semaphores
	freeSlots->notify();
	// Return frame to caller
	return tempFrame;
}
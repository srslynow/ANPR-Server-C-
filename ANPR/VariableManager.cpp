#include "VariableManager.h"



VariableManager::VariableManager()
{
	fileStore.open("config.yml", cv::FileStorage::READ);
	if (fileStore.isOpened())
	{
		// Config file was already created, get some parameters
		fileStore["debugMode"] >> debugMode;
		fileStore["plateW"] >> plateW;
		fileStore["plateH"] >> plateH;
		fileStore["charhW"] >> charW;
		fileStore["charH"] >> charH;
		fileStore["charB"] >> charB;
		fileStore["useGPU"] >> _useGPU;
	}
	else
		this->write();
}

void VariableManager::write()
{
	// Config file does not exist yet, create it.
	fileStore.open("config.yml", cv::FileStorage::WRITE);
	fileStore << "debugMode" << debugMode;
	fileStore << "plateW" << plateW;
	fileStore << "plateH" << plateH;
	fileStore << "charW" << charW;
	fileStore << "charH" << charH;
	fileStore << "charB" << charB;
	fileStore << "useGPU" << _useGPU;
	fileStore.release();
}

void VariableManager::setDebugMode(bool mode)
{
	debugMode = mode;
	this->write();
}

void VariableManager::setPlateSize(int width, int height)
{
	plateW = width;
	plateH = height;
	this->write();
}

cv::Size VariableManager::getPlateSize()
{
	return cv::Size(plateW, plateH);
}

void VariableManager::setCharSize(int width, int height)
{
	charW = width;
	charH = height;
	this->write();
}

cv::Size VariableManager::getCharSize()
{
	return cv::Size(charW, charH);
}


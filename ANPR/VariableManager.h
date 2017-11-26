#pragma once
#include "opencv2/core.hpp"

class VariableManager
{
public:
	static VariableManager& instance()
	{
		static VariableManager instance; // Guaranteed to be destroyed. Instantiated on first use.
		return instance;
	}

	void setDebugMode(bool mode);
	bool getDebugMode();

	void setPlateSize(int width, int height);
	cv::Size getPlateSize();

	void setCharSize(int width, int height);
	cv::Size getCharSize();

	bool useGPU() { return _useGPU; }

	int getCharBorder() { return charB; };
private:
	VariableManager();                   // Constructor
	VariableManager(VariableManager const&);              // Don't Implement
	void operator=(VariableManager const&); // Don't implement
	void write();
private:
	cv::FileStorage fileStore;
	bool debugMode = false;
	int plateW = 512;
	int plateH = 112;
	int charW = 64;
	int charH = 64;
	int charB = 6;
	bool _useGPU = true;
};

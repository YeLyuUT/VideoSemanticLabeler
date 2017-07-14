#pragma once
#include "DataType.h"
class ProcessControl
{
private:
	const int PROCESS_TYPE_NONE = 0;
	const int PROCESS_TYPE_IMAGES = 1;
	const int PROCESS_TYPE_VIDEO = 2;
public:
	ProcessControl(string filePath,string outputDir, LabelList& labelList);
	~ProcessControl();
public:
	void process();
private:
	bool checkCreateOutputDir();
	int checkForProcessType();
	void processImages();
	void processVideo();
private:
	int _type;
	string _filePath;
	string _outputDir;
	LabelList _labelList;
};


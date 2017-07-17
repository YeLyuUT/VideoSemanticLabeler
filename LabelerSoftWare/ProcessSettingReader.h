#pragma once
#include <opencv.hpp>
#include <string>
#include <tuple>
#include <vector>
#include <DataType.h>
using std::string;
using cv::FileStorage;




class ProcessSettingReader:public FileStorage
{
public:
	struct MetaData _data;
public:
	explicit ProcessSettingReader(string filePath);
	virtual ~ProcessSettingReader();
public:
	MetaData getMetaData();
	bool parse();

};


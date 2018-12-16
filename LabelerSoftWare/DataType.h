#pragma once
#include <string>
#include <tuple>
#include <vector>
using std::tuple;
using std::vector;
using std::string;

typedef tuple<string, int, int, int> Label;//LabelText,R,G,B
typedef vector<Label> LabelList;
typedef vector<int> SuperpixelScales;//SLIC superpixel scales

struct MetaData
{
public:
	string filePath;
	string outputDir;
	LabelList labelList;
	int skipFrameNum;
  string imgExtension;
  int extractBoundary;
  SuperpixelScales superpixel_scales;
};
#pragma once
#include <opencv.hpp>
namespace CV_Utils
{
	cv::Rect trimRect(cv::Rect rect, int min_x, int min_y, int max_x, int max_y);//max_x is the image width,max_y is the image height
}

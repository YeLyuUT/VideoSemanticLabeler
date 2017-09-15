#pragma once
#include "opencv.hpp"
#include <QImage>
using cv::Mat;
namespace ImageConversion
{
	class StaticCVMatPool
	{
	private:
		StaticCVMatPool();
		~StaticCVMatPool();
	public:
		static Mat& MatCV_8UC4();
		static Mat& MatCV_8UC3();
		static Mat& MatCV_8UC1();
	public:
		static void releaseMat(int type);
		static void releaseAll();
	};

	QImage cvMat_to_QImage(const cv::Mat &mat, bool revertRGB = true);
	cv::Mat QImage_to_cvMat(const QImage &image, bool inCloneImageData = true);
}


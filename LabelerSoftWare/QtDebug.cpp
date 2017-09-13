#include <QtDebug.h>
#include <ImageConversion.h>
#include <opencv.hpp>
namespace qt_debug
{
	void showQImage(QImage& img)
	{
		cv::Mat Img = ImageConversion::QImage_to_cvMat(img, false);
		cv::imshow("qimage", Img);
		cv::waitKey(500);
	}
}
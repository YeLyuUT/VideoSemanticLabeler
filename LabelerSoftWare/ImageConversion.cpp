#include <ImageConversion.h>
#include <QDebug>
namespace ImageConversion
{
	Mat& StaticCVMatPool::MatCV_8UC4()
	{
		static Mat m;
		return m;
	}
	Mat& StaticCVMatPool::MatCV_8UC3()
	{
		static Mat m;
		return m;
	}
	Mat& StaticCVMatPool::MatCV_8UC1()
	{
		static Mat m;
		return m;
	}
	void StaticCVMatPool::releaseMat(int type)
	{
		switch (type)
		{
		case CV_8UC4:MatCV_8UC4() = Mat(); break;
		case CV_8UC3:MatCV_8UC3() = Mat(); break;
		case CV_8UC1:MatCV_8UC1() = Mat(); break;
		default:
			break;
		}
	}
	void StaticCVMatPool::releaseAll()
	{
		MatCV_8UC4() = Mat();
		MatCV_8UC3() = Mat();
		MatCV_8UC1() = Mat();
	}


	//##### cv::Mat ---> QImage #####
	QImage cvMat_to_QImage(const cv::Mat &mat, bool revertRGB) {
		switch (mat.type())
		{
			// 8-bit, 4 channel
		case CV_8UC4:
		{
			QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB32);
			return image;
		}

		// 8-bit, 3 channel
		case CV_8UC3:
		{			
			Mat& temp = StaticCVMatPool::MatCV_8UC3();
			if (revertRGB)
			{
				cv::cvtColor(mat, temp, CV_BGR2RGB);
				QImage image(temp.data, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
				return image;
			}
			else
			{
				QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
				return image;
			}
		}

		// 8-bit, 1 channel
		case CV_8UC1:
		{
			static QVector<QRgb>  sColorTable;
			// only create our color table once
			if (sColorTable.isEmpty())
			{
				for (int i = 0; i < 256; ++i)
					sColorTable.push_back(qRgb(i, i, i));
			}
			QImage image(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
			image.setColorTable(sColorTable);
			return image;
		}

		default:
			qDebug()<<QString("Image format is not supported: depth=%0 and %0 channels").arg(mat.depth()).arg(mat.channels());
			break;
		}
		return QImage();
	}


	//##### QImage ---> cv::Mat #####
	cv::Mat QImage_to_cvMat(const QImage &image, bool inCloneImageData) {
		switch (image.format())
		{
			// 8-bit, 4 channel
		case QImage::Format_RGB32:
		{
			cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			return (inCloneImageData ? mat.clone() : mat);
		}

		// 8-bit, 3 channel
		case QImage::Format_RGB888:
		{
			qWarning() << "ASM::QImageToCvMat() - conversion color order may be different";		
			cv::Mat mat(image.height(), image.width(), CV_8UC3, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			return (inCloneImageData ? mat.clone() : mat);
		}

		// 8-bit, 1 channel
		case QImage::Format_Indexed8:
		{
			cv::Mat  mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			return (inCloneImageData ? mat.clone() : mat);
		}
		case QImage::Format_Grayscale8:
		{
			cv::Mat  mat(image.height(), image.width(), CV_8UC1, const_cast<uchar*>(image.bits()), image.bytesPerLine());
			return (inCloneImageData ? mat.clone() : mat);
		}
		default:
			qDebug()<<QString("Image format is not supported: depth=%0 and %0 format\n").arg(image.depth()).arg(image.format());
			break;
		}

		return cv::Mat();
	}
}

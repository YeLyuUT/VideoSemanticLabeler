#include "DrawTaskControl.h"
#include <QImage>
#include <QPainter>
#include "ImageConversion.h"
//#define CHECK_RETRIEVE_PAINTERPATH
//#define CHECK_MASK_OUTPUTIMAGE
DrawTaskControl::DrawTaskControl(QImage& Img, ClassSelection* selection)
{
	_InputImg = Img.copy();
	_selection = selection;
	setupOtherImg();
	_surfaceSegmentation = new Surface(_segImg());
	_surfaceOriginal = new Surface(_InputImg);
	_surfaceOutPut = new Surface(_outPutImg());

	_surfaceSegmentation->setWindowTitle("Segmentation");
	_surfaceOriginal->setWindowTitle("Original");
	_surfaceOutPut->setWindowTitle("OutPut");

	QObject::connect(_surfaceSegmentation, SIGNAL(openClassSelection()), selection, SLOT(show()));
	QObject::connect(_surfaceSegmentation, SIGNAL(closeClassSelection()), selection, SLOT(hide()));
	QObject::connect(selection, SIGNAL(classChanged(QString, QColor)), _surfaceSegmentation, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOriginal, SIGNAL(openClassSelection()), selection, SLOT(show()));
	QObject::connect(_surfaceOriginal, SIGNAL(closeClassSelection()), selection, SLOT(hide()));
	QObject::connect(selection, SIGNAL(classChanged(QString, QColor)), _surfaceOriginal, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOutPut, SIGNAL(openClassSelection()), selection, SLOT(show()));
	QObject::connect(_surfaceOutPut, SIGNAL(closeClassSelection()), selection, SLOT(hide()));
	QObject::connect(selection, SIGNAL(classChanged(QString, QColor)), _surfaceOutPut, SLOT(changeClass(QString, QColor)));
	
	QObject::connect(_surfaceOriginal, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	QObject::connect(_surfaceSegmentation, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	QObject::connect(_surfaceOutPut, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));

	_surfaceSegmentation->show();
	_surfaceOriginal->show();
	_surfaceOutPut->show();
}

DrawTaskControl* DrawTaskControl::getDrawControl(QImage& Img, ClassSelection* selection)
{
	static DrawTaskControl ctrl(Img, selection);
	return &ctrl;
}

DrawTaskControl::~DrawTaskControl()
{
	releaseAll();
}

QImage& DrawTaskControl::_segImg()
{
	static QImage img;
	return img;
}
Mat& DrawTaskControl::_labelImg()
{
	static Mat img;
	return img;
}
QImage& DrawTaskControl::_outPutImg()
{
	static QImage img;
	return img;
}
QImage& DrawTaskControl::_painterPathImage()
{
	static QImage img;
	return img;
}
void DrawTaskControl::releaseAll()
{
	_segImg() = QImage();
	_labelImg() = Mat();
	_outPutImg() = QImage();
	_painterPathImage() = QImage();
}

void DrawTaskControl::setupOtherImg()
{
	_segImg() = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
	_outPutImg() = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
	_painterPathImage() = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_Grayscale8);
	_labelImg() = Mat(_InputImg.height(), _InputImg.width(), CV_32S);
	_segImg().fill(0);
	_outPutImg().fill(0);
	_painterPathImage().fill(0);
	_labelImg().setTo(0);
}

void DrawTaskControl::retrievePainterPath(int PenWidth, QPainterPath& paintPath)
{
	_painterPathImage().fill(0);
	QPainter painter(&_painterPathImage());
	painter.setPen(QPen(QColor(255,255,255), PenWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	painter.drawPath(paintPath);
	painter.end();
#ifdef CHECK_RETRIEVE_PAINTERPATH
	cv::Mat tempImg = ImageConversion::QImage_to_cvMat(_painterPathImage(), false);
	cv::imshow("CHECK_RETRIEVE_PAINTERPATH", tempImg);
	cv::waitKey(1);
#endif
	updateImgByTouchedSegments(_painterPathImage());
	_boundingRect = paintPath.boundingRect().toRect().adjusted(-1, -1, 1, 1);
	updateOutPutImg(_boundingRect, _painterPathImage());
}

void DrawTaskControl::updateOutPutImg(QRect boundingRect,QImage& mask)
{
	Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg(), false);
	Mat maskImg = ImageConversion::QImage_to_cvMat(mask, false);
#ifdef CHECK_MASK_OUTPUTIMAGE
	cv::imshow("CHECK_MASK_OUTPUTIMAGE outPutImg", outPutImg);
	cv::imshow("CHECK_MASK_OUTPUTIMAGE maskImg", maskImg);
	cv::waitKey(1);
#endif
	QColor clr = _selection->getCurrentColor();
	outPutImg.setTo(Vec3b(clr.red(), clr.green(), clr.blue()), maskImg);
	updateSurface(_surfaceOutPut);
}

void DrawTaskControl::updateImgByTouchedSegments(QImage& Img)
{
	//TODO how to floodfill with segmentation
}

void DrawTaskControl::updateSurface(Surface *sf)
{
	if(sf)
		sf->update();
}
#include "LabelingTaskControl.h"
#include <QImage>
#include <QPainter>
#include "ImageConversion.h"
//#define CHECK_RETRIEVE_PAINTERPATH
//#define CHECK_MASK_OUTPUTIMAGE


LabelingTaskControl::LabelingTaskControl(QImage& Img, ClassSelection* selection, QObject* parent) :QObject(parent)
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

	_surfaceOriginal->setReferenceImage(&_outPutImg());
	_surfaceSegmentation->setReferenceImage(&_InputImg);
	_surfaceOutPut->setReferenceImage(&_InputImg);

	_surfaceOriginal->setEditable(true);
	_surfaceSegmentation->setEditable(true);
	_surfaceOutPut->setEditable(true);


	_SA1 = new SmartScrollArea();
	_SA2 = new SmartScrollArea();
	_SA3 = new SmartScrollArea();

	/*_SA1->setWidget(_surfaceOriginal);
	_SA2->setWidget(_surfaceSegmentation);
	_SA3->setWidget(_surfaceOutPut);*/

	QObject::connect(_surfaceOriginal, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA1, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	QObject::connect(_surfaceSegmentation, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA2, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	QObject::connect(_surfaceOutPut, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA3, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));

	_surfaceSegmentation->show();
	_surfaceOriginal->show();
	_surfaceOutPut->show();
	/*_SA1->show();
	_SA2->show();
	_SA3->show();*/
}

//LabelingTaskControl* LabelingTaskControl::getDrawControl(QImage& Img, ClassSelection* selection, bool newDraw)
//{
//	static shared_ptr<LabelingTaskControl> pCtrl;
//	if(newDraw)
//		pCtrl.reset(new LabelingTaskControl(Img, selection));
//	return pCtrl.get();
//}

LabelingTaskControl::~LabelingTaskControl()
{
	_surfaceSegmentation->deleteLater();
	_surfaceOriginal->deleteLater();
	_surfaceOutPut->deleteLater();
	_SA1->deleteLater();
	_SA2->deleteLater();
	_SA3->deleteLater();
	releaseAll();
}

QImage& LabelingTaskControl::_segImg()
{
	static QImage img;
	return img;
}
Mat& LabelingTaskControl::_labelImg()
{
	static Mat img;
	return img;
}
QImage& LabelingTaskControl::_outPutImg()
{
	static QImage img;
	return img;
}
QImage& LabelingTaskControl::_painterPathImage()
{
	static QImage img;
	return img;
}
void LabelingTaskControl::releaseAll()
{
	_segImg() = QImage();
	_labelImg() = Mat();
	_outPutImg() = QImage();
	_painterPathImage() = QImage();
}

void LabelingTaskControl::setupOtherImg()
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

void LabelingTaskControl::retrievePainterPath(int PenWidth, QPainterPath& paintPath)
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

void LabelingTaskControl::updateOutPutImg(QRect boundingRect,QImage& mask)
{
	Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg(), false);
	Mat maskImg = ImageConversion::QImage_to_cvMat(mask, false);

	QColor clr = _selection->getCurrentColor();
	outPutImg.setTo(Vec3b(clr.red(), clr.green(), clr.blue()), maskImg);
#ifdef CHECK_MASK_OUTPUTIMAGE
	cv::imshow("CHECK_MASK_OUTPUTIMAGE outPutImg", outPutImg);
	cv::imshow("CHECK_MASK_OUTPUTIMAGE maskImg", maskImg);
	cv::waitKey(1);
#endif
	updateSurface(_surfaceOutPut);
}

void LabelingTaskControl::updateImgByTouchedSegments(QImage& Img)
{
	//TODO how to floodfill with segmentation
}

void LabelingTaskControl::updateSurface(Surface *sf)
{
	if (sf)
	{
		sf->updateImage();
		sf->update();
	}
}


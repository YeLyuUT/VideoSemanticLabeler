#include "LabelingTaskControl.h"
#include <QImage>
#include <QPainter>
#include "ImageConversion.h"
#include <QFileInfo>
#include <QMessageBox>
#include <QDebug>
#include <QDesktopServices>
#include <exception>
#include <QFileInfo>
#include <thread>

//#define CHECK_RETRIEVE_PAINTERPATH
//#define CHECK_MASK_OUTPUTIMAGE
//#define CHECK_QIMAGE
#ifdef CHECK_QIMAGE
#include <QtDebug.h>
#endif // CHECK_QIMAGE


LabelingTaskControl::LabelingTaskControl(ProcessControl* pProcCtrl,VideoControl* pCtrl, ClassSelection* selection, QString outPutDir,bool autoLoadResult, QObject* parent) :QObject(parent)
{
	_segmentation_control = NULL;
	_autoLoadResult = autoLoadResult;
	_pVidCtrl = pCtrl;
	int index = _pVidCtrl->getPosFrames();
	_frameIdx = index;
	cv::Mat matFrame;
	if (!_pVidCtrl->getFrame(matFrame, index))
	{ 
		QMessageBox::critical(NULL, "Frame Cannot Read", "This frame cannot be read", QMessageBox::Cancel);
		//throw std::exception("cannot load Image to be labeled"); 
	};
	QImage Img = ImageConversion::cvMat_to_QImage(matFrame);
	_modified = false;
	_outPutDir = outPutDir;
	_InputImg = Img.copy();
	_selection = selection;
	_segmentation_control = new SegmentationControl(matFrame);
	//_segmentation_control->doSlicSegmentation();
	std::thread t(&SegmentationControl::doSlicSegmentation, _segmentation_control);
	_segmentation_control->setSegmentationType(SegmentationControl::SLIC_);
	setupOtherImg();
	_surfaceSegmentation = new Surface(_segImg);
	_surfaceOriginal = new Surface(_InputImg);
	_surfaceOutPut = new Surface(_outPutImg);
	_surfaceSegmentation->setDrawType(Surface::DRAW_TYPE::SUPER_PIXEL_WISE);
	QObject::connect(_segmentation_control, SIGNAL(signalSendPts(vector<Point>*)), _surfaceSegmentation, SLOT(slotPixelCovered(vector<Point>*)));
	QObject::connect(_surfaceSegmentation, SIGNAL(signalPixelCovered(vector<Point>*)), _segmentation_control, SLOT(slotReceivePts(vector<Point>*)));
	
	_SA1 = new SmartScrollArea();
	_SA2 = new SmartScrollArea();
	_SA3 = new SmartScrollArea();

	/*set up relation between scrollarea and surface*/
	_SA1->setWidget(_surfaceOriginal);
	_SA2->setWidget(_surfaceSegmentation);
	_SA3->setWidget(_surfaceOutPut);

	_surfaceOriginal->setScrollArea(_SA1);
	_surfaceSegmentation->setScrollArea(_SA2);
	_surfaceOutPut->setScrollArea(_SA3);	

	/*install event filters for scroll area*/
	_SA1->installEventFilter(pProcCtrl);
	_SA2->installEventFilter(pProcCtrl);
	_SA3->installEventFilter(pProcCtrl);

	_SA1->installEventFilter(_surfaceOriginal);
	_SA2->installEventFilter(_surfaceSegmentation);
	_SA3->installEventFilter(_surfaceOutPut);

	/*_surfaceSegmentation->setWindowTitle("Segmentation");
	_surfaceOriginal->setWindowTitle("Original");
	_surfaceOutPut->setWindowTitle("OutPut");*/
	_SA1->setWindowTitle("Original");
	_SA2->setWindowTitle("Segmentation");
	_SA3->setWindowTitle("OutPut");

	/************************************************/
	_surfaceOriginal->setEditable(true);
	_surfaceSegmentation->setEditable(true);
	_surfaceOutPut->setEditable(true);

	_surfaceOriginal->setReferenceImage(&_outPutImg);
	_surfaceSegmentation->setReferenceImage(&_outPutImg);
	_surfaceOutPut->setReferenceImage(&_InputImg);

	setupConnections();

	//_surfaceSegmentation->show();
	//_surfaceOriginal->show();
	//_surfaceOutPut->show();
	_SA3->move(QPoint(000,300));	
	_SA1->move(QPoint(600,300));	
	_SA2->move(QPoint(1200,300));	

	

	_SA1->show();
	_SA3->show();
	t.join();
	QImage segImg = createQImageByMat(_segmentation_control->getSlicSegResultRef(), true);
	resetSurfaceSource(_surfaceSegmentation, &segImg);
	_SA2->show();
}

LabelingTaskControl::~LabelingTaskControl()
{
	if (_segmentation_control) { delete _segmentation_control; _segmentation_control = NULL; }

	closeAllSubWindows();
	releaseAll();
}

QImage LabelingTaskControl::createQImageByMat(Mat& Img,bool revertRGB)
{
	return ImageConversion::cvMat_to_QImage(Img, revertRGB);
}

void LabelingTaskControl::setupConnections()
{
	setupColorSelectionConnections();
	setupScrollAreaConnections();
	setupSurfacePainterPathConnections();
	setupSurfaceHotKeyConnections();
	setupSurfaceWindowCloseConnections();
}

void LabelingTaskControl::setupSurfaceWindowCloseConnections()
{
	//QObject::connect(_SA1, SIGNAL(hide()), this, SLOT(deleteLater()));
	//QObject::connect(_SA2, SIGNAL(hide()), this, SLOT(deleteLater()));
	//QObject::connect(_SA3, SIGNAL(hide()), this, SLOT(deleteLater()));
}

void LabelingTaskControl::setupSurfaceHotKeyConnections()
{
	QObject::connect(_surfaceSegmentation, SIGNAL(clearResult()), this, SLOT(clearResult()));
	QObject::connect(_surfaceOriginal, SIGNAL(clearResult()), this, SLOT(clearResult()));
	QObject::connect(_surfaceOutPut, SIGNAL(clearResult()), this, SLOT(clearResult()));
}

void LabelingTaskControl::setupColorSelectionConnections()
{
	//connect surface and selection panel
	QObject::connect(_surfaceSegmentation, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	QObject::connect(_surfaceSegmentation, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceSegmentation, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOriginal, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	QObject::connect(_surfaceOriginal, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceOriginal, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOutPut, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	QObject::connect(_surfaceOutPut, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceOutPut, SLOT(changeClass(QString, QColor)));
}

void LabelingTaskControl::setupScrollAreaConnections()
{
	QObject::connect(_surfaceOriginal, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA1, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	QObject::connect(_surfaceSegmentation, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA2, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	QObject::connect(_surfaceOutPut, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA3, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
}

void LabelingTaskControl::setupSurfacePainterPathConnections()
{
	QObject::connect(_surfaceOriginal, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	QObject::connect(_surfaceSegmentation, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	QObject::connect(_surfaceOutPut, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
}

//LabelingTaskControl* LabelingTaskControl::getDrawControl(QImage& Img, ClassSelection* selection, bool newDraw)
//{
//	static shared_ptr<LabelingTaskControl> pCtrl;
//	if(newDraw)
//		pCtrl.reset(new LabelingTaskControl(Img, _selection));
//	return pCtrl.get();
//}



void LabelingTaskControl::closeAllSubWindows()
{
	if(_surfaceSegmentation)
	{
		_surfaceSegmentation->deleteLater();
		_surfaceSegmentation = NULL;
	}
	if(_surfaceOriginal)
	{
		_surfaceOriginal->deleteLater();
		_surfaceOriginal = NULL;
	}
	if (_surfaceOutPut)
	{
		_surfaceOutPut->deleteLater();
		_surfaceOutPut = NULL;
	}
	if (_SA1)
	{
		_SA1->deleteLater();
		_SA1 = NULL;
	}
	if (_SA2)
	{
		_SA2->deleteLater();
		_SA2 = NULL;
	}
	if (_SA3)
	{
		_SA3->deleteLater();
		_SA3 = NULL;
	}
}

bool LabelingTaskControl::maySaveResult(QString filePath)
{
	QFileInfo checkFile(filePath);
	if (checkFile.exists() && checkFile.isFile())
	{
		QMessageBox::StandardButton button = 
		QMessageBox::warning(NULL,"Saving File...","This file already exists, do you want to rewrite it?",
			QMessageBox::StandardButton::Ok|QMessageBox::StandardButton::Cancel);
		if (button == QMessageBox::StandardButton::Ok)
		{
			return saveResult(filePath);
		}
		else
			return false;
	}
	return saveResult(filePath);
}

bool LabelingTaskControl::saveResult(QString filePath)
{
	QImage pImg = _surfaceOutPut->getImage();
	Mat Img = ImageConversion::QImage_to_cvMat(pImg, false);
	/*The RGB color order is different, need to switch R and B*/
	Mat ImgRvt;
	cv::cvtColor(Img, ImgRvt, CV_RGB2BGR);
	bool bsave = cv::imwrite(filePath.toStdString(), ImgRvt);
	if(!bsave)
		QMessageBox::warning(NULL, "Fail to Save", "Cannot save the result!!", QMessageBox::StandardButton::Close);
	return bsave;
}

void LabelingTaskControl::saveLabelResult()
{
	QString filePath = getResultSavingPathName();
	if (maySaveResult(filePath))
	{
		qDebug() << "Save Successful";
	}
	else
	{
		qDebug() << "Save Failed";
	}
}

QString LabelingTaskControl::getResultSavingPathName()
{
	return QString(this->_outPutDir + QString("/%1.bmp")).arg(_frameIdx);
}

void LabelingTaskControl::openSaveDir()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(this->_outPutDir));
}

bool LabelingTaskControl::checkModified()
{
	//TODO need to check correction
	QImage pImg = _surfaceOutPut->getImage();
	Mat Img = ImageConversion::QImage_to_cvMat(pImg, false);
	for (size_t j = 0; j < Img.rows; j++)
	{
		assert(Img.type() == CV_8UC3);
		Vec3b* pVec = Img.ptr<Vec3b>(j);
		for (size_t i = 0; i < Img.cols; i++)
		{
			if (pVec[i][0] != 0) return true;
			if (pVec[i][1] != 0) return true;
			if (pVec[i][2] != 0) return true;
		}
	}
	return false;
}

//QImage& LabelingTaskControl::_segImg()
//{
//	static QImage img;
//	return img;
//}
//Mat& LabelingTaskControl::_labelImg()
//{
//	static Mat img;
//	return img;
//}
//QImage& LabelingTaskControl::_outPutImg()
//{
//	static QImage img;
//	return img;
//}
//QImage& LabelingTaskControl::_painterPathImage()
//{
//	static QImage img;
//	return img;
//}
void LabelingTaskControl::releaseAll()
{
	_segImg = QImage();
	_labelImg = Mat();
	_outPutImg = QImage();
	_painterPathImage = QImage();
}

void LabelingTaskControl::setupOtherImg()
{
	_segImg = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
	_painterPathImage = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_Grayscale8);
	_labelImg = Mat(_InputImg.height(), _InputImg.width(), CV_32S);
	_segImg.fill(0);
	_painterPathImage.fill(0);
	_labelImg.setTo(0);
	
	if (_autoLoadResult)
	{
		loadResultFromDir();//This has already set _outPutImg
	}
	else
	{
		_outPutImg = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
		_outPutImg.fill(0);
	}
}

void LabelingTaskControl::retrievePainterPath(int PenWidth, QPainterPath& paintPath)
{
	_painterPathImage.fill(0);
	QPainter painter(&_painterPathImage);
	painter.setPen(QPen(QColor(255,255,255), PenWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	painter.drawPath(paintPath);
	painter.end();
#ifdef CHECK_RETRIEVE_PAINTERPATH
	cv::Mat tempImg = ImageConversion::QImage_to_cvMat(_painterPathImage, false);
	cv::imshow("CHECK_RETRIEVE_PAINTERPATH", tempImg);
	cv::waitKey(1);
#endif
	updateImgByTouchedSegments(_painterPathImage);
	_boundingRect = paintPath.boundingRect().toRect().adjusted(-1, -1, 1, 1);
	updateOutPutImg(_boundingRect, _painterPathImage);
}

void LabelingTaskControl::updateOutPutImg(QRect boundingRect,QImage& mask)
{
	Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg, false);
	Mat maskImg = ImageConversion::QImage_to_cvMat(mask, false);

	QColor clr = _selection->getCurrentColor();
	outPutImg.setTo(Vec3b(clr.red(), clr.green(), clr.blue()), maskImg);
#ifdef CHECK_MASK_OUTPUTIMAGE
	cv::imshow("CHECK_MASK_OUTPUTIMAGE outPutImg", outPutImg);
	cv::imshow("CHECK_MASK_OUTPUTIMAGE maskImg", maskImg);
	cv::waitKey(500);
#endif
	updateSurface(_surfaceOutPut);
}

void LabelingTaskControl::updateImgByTouchedSegments(QImage& Img)
{
	//TODO how to floodfill with segmentation
}

void LabelingTaskControl::updateAllSurfaces()
{
	/*if (_surfaceSegmentation) _surfaceSegmentation->update();
	if (_surfaceOriginal) _surfaceOriginal->update();
	if (_surfaceOutPut) _surfaceOutPut->update();*/
	updateSurface(_surfaceSegmentation);
	updateSurface(_surfaceOriginal);
	updateSurface(_surfaceOutPut);
}

void LabelingTaskControl::updateSurface(Surface *sf)
{
	if (sf)
	{
		sf->updateImage();
		sf->update();
	}
}

void LabelingTaskControl::clearResult()
{
	QMessageBox::StandardButton button = 
	QMessageBox::warning(NULL, "Reset Result", "Do you really want to reset the result?", QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
	if (QMessageBox::StandardButton::Yes == button)
	{
#ifdef CHECK_QIMAGE
		qt_debug::showQImage(_outPutImg);
#endif
		_outPutImg.fill(0);
		_painterPathImage.fill(0);
		_labelImg.setTo(0);
		_surfaceOutPut->setOriginalImage(_outPutImg);
		updateAllSurfaces();
#ifdef CHECK_QIMAGE
		qt_debug::showQImage(_outPutImg);
#endif
	}
}

void LabelingTaskControl::loadResultFromDir()
{
	QString filePath = getResultSavingPathName();
	cv::Mat IMG = cv::imread(filePath.toStdString());
	QImage qIMG = ImageConversion::cvMat_to_QImage(IMG);
	this->_outPutImg = qIMG;
}

void LabelingTaskControl::setAutoLoadResult(bool b)
{
	_autoLoadResult = b;
	if (_autoLoadResult)
	{
		QFileInfo info(getResultSavingPathName());
		if(info.exists())
		{ 
			loadResultFromDir();
			reAttachOutPutImage();
			_surfaceOutPut->update();
		}
	}
	qDebug() << QString("setAutoLoadResult:%1").arg(b);
}

void LabelingTaskControl::resetSurfaceSource(Surface* surface, QImage* source)
{
	surface->setOriginalImage(*source);
}

void LabelingTaskControl::reAttachOutPutImage()
{
	resetSurfaceSource(_surfaceOutPut, &_outPutImg);
}

void LabelingTaskControl::changeTransparency(int value)
{
	double v = value / 100.0;
	_surfaceOriginal->setBlendAlpha(v, 1.0 - v);
	_surfaceOutPut->setBlendAlpha(v, 1.0 - v);
	_surfaceSegmentation->setBlendAlpha(v, 1.0 - v);
}
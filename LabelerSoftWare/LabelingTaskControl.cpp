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
#include <QtUtils.h>
#include <limits.h>
#include <QScrollBar>
#include <QProgressBar>
#include <QString>
#include <QtConcurrent>
#include <numeric>

//#define CHECK_RETRIEVE_PAINTERPATH
//#define CHECK_MASK_OUTPUTIMAGE
//#define CHECK_QIMAGE
#ifdef CHECK_QIMAGE
#include <QtDebug.h>
#endif // CHECK_QIMAGE


LabelingTaskControl::LabelingTaskControl(ProcessControl* pProcCtrl,VideoControl* pCtrl, ClassSelection* selection, QString outPutDir,bool autoLoadResult, QObject* parent) :QObject(parent)
{
  _pCtrl = pProcCtrl;
	//_segmentation_control = NULL;
	_autoLoadResult = autoLoadResult;
	_pVidCtrl = pCtrl;
	_segSurfaceSet = false;
	_curSegmentation_control = nullptr;
  this->_canvas_idx = 0;
	/*_pVidCtrl->reset(_pVidCtrl->getPosFrames());*/
	int index = _pVidCtrl->getPosFrames();
	qDebug() <<"index:" <<_pVidCtrl->getPosFrames();
	_frameIdx = index;
	cv::Mat matFrame;
	if (!_pVidCtrl->getFrameWithoutIncreaseFrameIdx(matFrame, index))
	{ 
		QMessageBox::critical(NULL, "Frame Cannot Read", "This frame cannot be read", QMessageBox::Cancel);
		//throw std::exception("cannot load Image to be labeled"); 
	};
qDebug() <<"index:" <<_pVidCtrl->getPosFrames();
	QImage Img = ImageConversion::cvMat_to_QImage(matFrame,true, true);
	_modified = false;
	_outPutDir = outPutDir;
	_InputImg = Img.copy();
	_selection = selection;
  auto& sp_scales = _pCtrl->get_superpixel_scales();
	_segmentation_controls.push_back(new SegmentationControl(matFrame, sp_scales[0]));
	_segmentation_controls.push_back(new SegmentationControl(matFrame, sp_scales[1]));
	_segmentation_controls.push_back(new SegmentationControl(matFrame, sp_scales[2]));
	_segmentation_controls.push_back(new SegmentationControl(matFrame, sp_scales[3]));
	//_segmentation_control->doSlicSegmentation();
	//std::thread t(&SegmentationControl::doSlicSegmentation, _segmentation_control);
	//_segmentation_control->setSegmentationType(SegmentationControl::SLIC_);
	

	//t.join();
	//_segImg = createQImageByMat(_segmentation_control->getSlicSegResultRef(), true);//set segImg
qDebug() <<"index:" <<_pVidCtrl->getPosFrames();
	setupOtherImg();
	_surfaceOriginal = new Surface(_InputImg);
	_surfaceOutPut = new Surface(_outPutImg);
	_surfaceSegmentation = nullptr;
	//_surfaceSegmentation = new Surface(_segImg);

	if (_autoLoadResult)
	{
		QFileInfo info(getResultSavingPathName());
		if (info.exists())
		{
			loadResultFromDir();//This has already set _outPutImg
			reAttachOutPutImage();
			updateAllSurfaces();
		}
	}
	_surfaceOriginal->setReferenceImage(&_surfaceOutPut->getOriImage());//_outPutImg
	_surfaceOutPut->setReferenceImage(&_surfaceOriginal->getOriImage());//_InputImg
	//_surfaceSegmentation->setReferenceImage(&_surfaceOutPut->getOriImage());//_outPutImg
	//_surfaceSegmentation->setReferenceOriginalImage(&_surfaceOriginal->getOriImage());//_InputImg
	//_surfaceOutPut->setReferenceOriginalImage(&_surfaceOriginal->getOriImage());//_InputImg
qDebug() <<"index:" <<_pVidCtrl->getPosFrames();
	_surfaceOriginal->setAllowPolyMode(true);
	//_surfaceSegmentation->setDrawType(Surface::DRAW_TYPE::SUPER_PIXEL_WISE);
	//QObject::connect(_segmentation_control, SIGNAL(signalSendPts(vector<PtrSegmentPoints>*)), _surfaceSegmentation, SLOT(slotPixelCovered(vector<PtrSegmentPoints>*)));
	//QObject::connect(_surfaceSegmentation, SIGNAL(signalPixelCovered(vector<Point>*)), _segmentation_control, SLOT(slotReceivePts(vector<Point>*)));
	//QObject::connect(_surfaceSegmentation, SIGNAL(signalDrawPixelsToResult(vector<PtrSegmentPoints>*, QColor)), this, SLOT(retrieveSegmentsDraw(vector<PtrSegmentPoints>*, QColor)));
	QObject::connect(_surfaceOriginal, SIGNAL(signalSendPolygonDraw(vector<Point>, QColor)), this, SLOT(retrievePolygonDraw(vector<Point>, QColor)));
	
	_SA1 = new SmartScrollArea();
	_SA2 = new SmartScrollArea();
	_SA3 = new SmartScrollArea();

	/*set up relation between scrollarea and surface*/
	_SA1->setWidget(_surfaceOriginal);
	//_SA2->setWidget(_surfaceSegmentation);
	_SA3->setWidget(_surfaceOutPut);

	_surfaceOriginal->setScrollArea(_SA1);
	//_surfaceSegmentation->setScrollArea(_SA2);
	_surfaceOutPut->setScrollArea(_SA3);	

	/*install event filters for scroll area*/
	_SA1->installEventFilter(pProcCtrl);
	_SA2->installEventFilter(pProcCtrl);
	_SA3->installEventFilter(pProcCtrl);

	_SA1->installEventFilter(_surfaceOriginal);
	//_SA2->installEventFilter(_surfaceSegmentation);
	_SA3->installEventFilter(_surfaceOutPut);

	/*_surfaceSegmentation->setWindowTitle("Segmentation");
	_surfaceOriginal->setWindowTitle("Original");
	_surfaceOutPut->setWindowTitle("OutPut");*/
	_SA1->setWindowTitle("Original");
	_SA2->setWindowTitle("Segmentation");
	_SA3->setWindowTitle("OutPut");

	/************************************************/
	_surfaceOriginal->setEditable(true);
	//_surfaceSegmentation->setEditable(true);
	_surfaceOutPut->setEditable(true);

	setupConnections();
qDebug() <<"index:" <<_pVidCtrl->getPosFrames();
	//_surfaceSegmentation->show();
	//_surfaceOriginal->show();
	//_surfaceOutPut->show();
	_SA3->move(QPoint(000,300));	
	_SA1->move(QPoint(500,300));	
	_SA2->move(QPoint(1000,300));	

	_SA1->show();
	_SA3->show();
	//_SA2->show();
}

LabelingTaskControl::~LabelingTaskControl()
{
	//if (_segmentation_control) { delete _segmentation_control; _segmentation_control = NULL; }
	for (size_t i = 0; i < _segmentation_controls.size(); i++)
	{
		delete _segmentation_controls[i];
	}
	_segmentation_controls.resize(0);
	closeAllSubWindows();
	releaseAll();
}

void LabelingTaskControl::setupSegmentationSurface(int level)
{
	level = qMax<int>(0, level);
	level = qMin<int>(_segmentation_controls.size(), level);
	if (!_segSurfaceSet)
	{
		vector<QFuture<void>> vecThreads;
    //QLabel* progress_widget = new QLabel();
    //progress_widget->setWindowTitle("SLIC segmentation");
    //progress_widget->setMinimumHeight(50);
    //progress_widget->setFixedWidth(200);
    QProgressBar* progress = new QProgressBar();
    progress->setWindowTitle("Please wait for SLIC segmentation.");
    progress->show();
    progress->setMaximum(4);
    progress->setMinimum(0);
    progress->setValue(0);
    progress->setMinimumWidth(500);
    progress->setMinimumHeight(50);
    progress->setAlignment(Qt::AlignCenter);
    progress->setFormat(QString("Processing SLIC segmentation..."));

		for (size_t i = 0; i < _segmentation_controls.size(); i++)
		{
			_segmentation_controls[i]->setSegmentationType(SegmentationControl::SLIC_);
			QFuture<void> t = QtConcurrent::run(_segmentation_controls[i],&SegmentationControl::doSlicSegmentation);
			vecThreads.push_back(std::move(t));
		}
    std::vector<int> isReady(4,0);
    int readyNum = 0;
    int newreadyNum = 0;
    while (true)
    {
      bool allTrue = true;
      for (int i = 0; i < 4; i++)
      {
        if (vecThreads[i].isFinished() == false) allTrue = false;
        else isReady[i] = 1;
      }
      if (allTrue) break;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      newreadyNum = std::accumulate(isReady.begin(),isReady.end(),0);
      if (readyNum != newreadyNum) { readyNum = newreadyNum; progress->setValue(readyNum); }
      QCoreApplication::processEvents();
    }
		
    progress->setFormat(QString("Done!"));
    progress->setValue(4);
    progress->deleteLater();
    
		//t.join();
		_curSegmentation_control = _segmentation_controls[level];
		_segImg = createQImageByMat(_curSegmentation_control->getSlicSegResultRef(), true);//set segImg
		//_surfaceSegmentation = new Surface(_segImg);
   
    _surfaceSegmentation = new Surface(_InputImg);
		_surfaceSegmentation->setReferenceImage(&_surfaceOutPut->getOriImage());//_outPutImg
    //store segmentation result image instead
		//_surfaceSegmentation->setReferenceOriginalImage(&_surfaceOriginal->getOriImage());//_InputImg
    _surfaceSegmentation->setReferenceOriginalImage(&_segImg);
		_surfaceSegmentation->setDrawType(Surface::DRAW_TYPE::SUPER_PIXEL_WISE);
		QObject::connect(_curSegmentation_control, SIGNAL(signalSendPts(vector<PtrSegmentPoints>*)), _surfaceSegmentation, SLOT(slotPixelCovered(vector<PtrSegmentPoints>*)));
		QObject::connect(_surfaceSegmentation, SIGNAL(signalPixelCovered(vector<Point>*)), _curSegmentation_control, SLOT(slotReceivePts(vector<Point>*)));
		QObject::connect(_surfaceSegmentation, SIGNAL(signalDrawPixelsToResult(vector<PtrSegmentPoints>*, QColor)), this, SLOT(retrieveSegmentsDraw(vector<PtrSegmentPoints>*, QColor)));
		_SA2->setWidget(_surfaceSegmentation);
		_surfaceSegmentation->setScrollArea(_SA2);
		_SA2->installEventFilter(_surfaceSegmentation);
		_surfaceSegmentation->setEditable(true);
		QObject::connect(_surfaceSegmentation, SIGNAL(clearResult()), this, SLOT(clearResult()));
		QObject::connect(_surfaceSegmentation, SIGNAL(openClassSelection()), _selection, SLOT(show()));
		QObject::connect(_surfaceSegmentation, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
		QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceSegmentation, SLOT(changeClass(QString, QColor)));
		QObject::connect(_surfaceSegmentation, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA2, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
		QObject::connect(_surfaceSegmentation, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
		QObject::connect(_surfaceSegmentation,SIGNAL(signalChangeSPSegLevel(int)),this,SIGNAL(signalChangeLevelByDiff(int)));
		_SA2->show();
		_segSurfaceSet = true;
	}
	else
	{
		if (_surfaceSegmentation)
		{
			bool show_ref = _surfaceSegmentation->_bShowRef;
			int barX = _SA2->horizontalScrollBar()->value();
			int barY = _SA2->verticalScrollBar()->value();
			int scaleRank = _surfaceSegmentation->getScaleRatioRank();
			double blendRatioRef = _surfaceSegmentation->getBlendAlphaReference();
			double blendRatioSrc = _surfaceSegmentation->getBlendAlphaSource();
			_surfaceSegmentation->deleteLater();
	
			_curSegmentation_control = _segmentation_controls[level];
			_segImg = createQImageByMat(_curSegmentation_control->getSlicSegResultRef(), true);//set segImg
			//_surfaceSegmentation = new Surface(_segImg);
      _surfaceSegmentation = new Surface(_InputImg);
			_surfaceSegmentation->setBlendAlpha(blendRatioSrc, blendRatioRef);
			_surfaceSegmentation->setReferenceImage(&_surfaceOutPut->getOriImage());//_outPutImg
			//_surfaceSegmentation->setReferenceOriginalImage(&_surfaceOriginal->getOriImage());//_InputImg
      _surfaceSegmentation->setReferenceOriginalImage(&_segImg);
			_surfaceSegmentation->setDrawType(Surface::DRAW_TYPE::SUPER_PIXEL_WISE);
			QObject::connect(_curSegmentation_control, SIGNAL(signalSendPts(vector<PtrSegmentPoints>*)), _surfaceSegmentation, SLOT(slotPixelCovered(vector<PtrSegmentPoints>*)));
			QObject::connect(_surfaceSegmentation, SIGNAL(signalPixelCovered(vector<Point>*)), _curSegmentation_control, SLOT(slotReceivePts(vector<Point>*)));
			QObject::connect(_surfaceSegmentation, SIGNAL(signalDrawPixelsToResult(vector<PtrSegmentPoints>*, QColor)), this, SLOT(retrieveSegmentsDraw(vector<PtrSegmentPoints>*, QColor)));
			_SA2->setWidget(_surfaceSegmentation);
			_surfaceSegmentation->setScrollArea(_SA2);
			_SA2->installEventFilter(_surfaceSegmentation);
			_surfaceSegmentation->setEditable(true);
			QObject::connect(_surfaceSegmentation, SIGNAL(clearResult()), this, SLOT(clearResult()));
			QObject::connect(_surfaceSegmentation, SIGNAL(openClassSelection()), _selection, SLOT(show()));
			QObject::connect(_surfaceSegmentation, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
			QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceSegmentation, SLOT(changeClass(QString, QColor)));
			QObject::connect(_surfaceSegmentation, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA2, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
			QObject::connect(_surfaceSegmentation, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
			QObject::connect(_surfaceSegmentation, SIGNAL(signalChangeSPSegLevel(int)), this, SIGNAL(signalChangeLevelByDiff(int)));
			_surfaceSegmentation->setScaleRatioRank(scaleRank);
			_surfaceSegmentation->updateScaleRatioByRank();
			_surfaceSegmentation->applyScaleRatio();
			_surfaceSegmentation->resize(_surfaceSegmentation->_ImageDraw.width(), _surfaceSegmentation->_ImageDraw.height());
			_SA2->horizontalScrollBar()->setValue(barX);
			_SA2->verticalScrollBar()->setValue(barY);
			if (show_ref)
			{
				_surfaceSegmentation->showReferenceImg();
				_surfaceSegmentation->_bShowRef = true;
			}
			_SA2->show();
		}
		if (!_surfaceSegmentation)
			throw std::exception("surface segmentation empty");
	}
}

QImage LabelingTaskControl::createQImageByMat(Mat& Img,bool revertRGB)
{
	return ImageConversion::cvMat_to_QImage(Img, true, revertRGB);
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
	//QObject::connect(_surfaceSegmentation, SIGNAL(clearResult()), this, SLOT(clearResult()));
	QObject::connect(_surfaceOriginal, SIGNAL(clearResult()), this, SLOT(clearResult()));
	QObject::connect(_surfaceOutPut, SIGNAL(clearResult()), this, SLOT(clearResult()));
}

void LabelingTaskControl::setupColorSelectionConnections()
{
	//connect surface and selection panel
	//QObject::connect(_surfaceSegmentation, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	//QObject::connect(_surfaceSegmentation, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	//QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceSegmentation, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOriginal, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	QObject::connect(_surfaceOriginal, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceOriginal, SLOT(changeClass(QString, QColor)));

	QObject::connect(_surfaceOutPut, SIGNAL(openClassSelection()), _selection, SLOT(show()));
	QObject::connect(_surfaceOutPut, SIGNAL(closeClassSelection()), _selection, SLOT(hide()));
	QObject::connect(_selection, SIGNAL(classChanged(QString, QColor)), _surfaceOutPut, SLOT(changeClass(QString, QColor)));
}

void LabelingTaskControl::setupScrollAreaConnections()
{
	QObject::connect(_surfaceOriginal, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA1, SLOT(gentleShiftScrollAreaWhenScaled( QPoint, double, double)));
	//QObject::connect(_surfaceSegmentation, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA2, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	QObject::connect(_surfaceOutPut, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), _SA3, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
}

void LabelingTaskControl::setupSurfacePainterPathConnections()
{
	QObject::connect(_surfaceOriginal, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	//QObject::connect(_surfaceSegmentation, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
	QObject::connect(_surfaceOutPut, SIGNAL(painterPathCreated(int, QPainterPath&)), this, SLOT(retrievePainterPath(int, QPainterPath&)));
}


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
    bool isSaveSuccessful = false;
		if (button == QMessageBox::StandardButton::Ok)
		{
			return saveResult(filePath);
		}
		else
			return false;
	}
	return saveResult(filePath);
}

bool LabelingTaskControl::saveBoundaryFile(Mat& Image,QString filePath)
{
  string qfPath = (filePath + ".b").toStdString();
  FILE* pF = fopen(qfPath.c_str(), "wb+");
  if (pF == NULL)
  {
    QMessageBox::warning(NULL,QString("Error"),QString("Cannot create boundary file."),QMessageBox::Ok,QMessageBox::NoButton);
    return false;
  }
  //cv::findContours()
  return true;
}

bool LabelingTaskControl::saveResult(QString filePath, bool saveOriginalImg)
{
	const QImage& pImg = _surfaceOutPut->getOriImage();
	Mat Img = ImageConversion::QImage_to_cvMat(pImg, false);
	/*The RGB color order is different, need to switch R and B*/
	Mat ImgRvt;
	cv::cvtColor(Img, ImgRvt, CV_RGB2BGR);
	bool bsave = cv::imwrite(filePath.toStdString(), ImgRvt);
  int extractBoundary = _pCtrl->get_extractBoundary();
  if (extractBoundary != 0)
  {

  }
	qDebug() << "File Saved To: " << filePath;
	if (!bsave)
	{
		QMessageBox::warning(NULL, "Fail to Save", "Cannot save the result!!", QMessageBox::StandardButton::Close);
		return bsave;
	}
	if(saveOriginalImg)
	{
		QString oriIMG = getOriginalIMGSavingPathName();
		bsave = saveOriginalIMG(oriIMG);
		if (!bsave)
		{
			QMessageBox::warning(NULL, "Fail to Save", "Cannot save the Original Image!!", QMessageBox::StandardButton::Close);
		}
		return bsave;
	}
	return false;
}

bool LabelingTaskControl::saveOriginalIMG(QString filePath)
{
	QImage pImg = _surfaceOriginal->getOriImage();
	Mat Img = ImageConversion::QImage_to_cvMat(pImg, false);
	/*The RGB color order is different, need to switch R and B*/
	Mat ImgRvt;
	cv::cvtColor(Img, ImgRvt, CV_RGB2BGR);
	bool bsave = cv::imwrite(filePath.toStdString(), ImgRvt);
	qDebug() << "File Saved To: " << filePath;
	if (!bsave)
		QMessageBox::warning(NULL, "Fail to Save", "Cannot save the result!!", QMessageBox::StandardButton::Close);
	return bsave;
}

void LabelingTaskControl::saveLabelResult()
{
  bool isSaved = false;
  try
  {
    QString filePath = getResultSavingPathName();
    isSaved = maySaveResult(filePath);
  }
  catch (...)
  {
    _pCtrl->set_imgExtension(string("png"));
    QString filePath = getResultSavingPathName();
    isSaved = maySaveResult(filePath);
  }
	if (isSaved)
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
	qDebug() << "_frameIdx:" << _frameIdx;
  QString ext = QString::fromStdString(_pCtrl->get_imgExtension());
  char buff[10];
  sprintf(buff,"%06d", _frameIdx);
	return QString(this->_outPutDir + QString("/%1.")+ext).arg(QString(buff));
}

QString LabelingTaskControl::getOriginalIMGSavingPathName()
{
  QString ext = QString::fromStdString(_pCtrl->get_imgExtension());
  char buff[10];
  sprintf(buff,"%06d", _frameIdx);
  return QString(this->_outPutDir + QString("/%1_ori." + ext)).arg(QString(buff));
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

void LabelingTaskControl::releaseAll()
{
	_segImg = QImage();
	_labelImg = Mat();
	_outPutImg = QImage();
	_painterPathImage = QImage();
}

void LabelingTaskControl::setupOtherImg()
{
	//_segImg = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
	_painterPathImage = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_Grayscale8);
	_labelImg = Mat(_InputImg.height(), _InputImg.width(), CV_32S);
	//_segImg.fill(0);
	_painterPathImage.fill(0);
	_labelImg.setTo(0);
	
	_outPutImg = QImage(_InputImg.width(), _InputImg.height(), QImage::Format::Format_RGB888);
	_outPutImg.fill(0);

}

void LabelingTaskControl::retrievePainterPath(int PenWidth, QPainterPath& paintPath)
{
	qDebug() << "To retrievePainterPath";
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

void LabelingTaskControl::updateOutPutImg(QRect boundingRect, QImage& mask)
{
  Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg, false);
  Mat maskImg = ImageConversion::QImage_to_cvMat(mask, false);

  qDebug() << CV_8UC3 << "==" << maskImg.type();
  QColor clr = _selection->getCurrentColor();
  if (this->_canvas_idx > 0)
  {
    Mat mask = this->getClrMask(this->_canvasColor, outPutImg);
    maskImg = maskImg&mask;
    outPutImg.setTo(Vec3b(clr.red(), clr.green(), clr.blue()), maskImg);
  }
  else
  {
    outPutImg.setTo(Vec3b(clr.red(), clr.green(), clr.blue()), maskImg);
  }

#ifdef CHECK_MASK_OUTPUTIMAGE
	cv::imshow("CHECK_MASK_OUTPUTIMAGE outPutImg", outPutImg);
	cv::imshow("CHECK_MASK_OUTPUTIMAGE maskImg", maskImg);
	cv::waitKey(500);
#endif
	//qDebug() << "To get Rect";
	//qDebug() << "maskImg.rows:" << maskImg.rows;
	//qDebug() << "maskImg.cols:" << maskImg.cols;
	//qDebug() << "mask.rows:" << mask.height();
	//qDebug() << "mask.cols:" << mask.width();
	int min_x = maskImg.cols, min_y = maskImg.rows, max_x = 0, max_y = 0;
	for (int i = 0; i < maskImg.rows; i++)
	{
		uchar* ptr = maskImg.ptr<uchar>(i);
		for (int j = 0; j < maskImg.cols; j++)
		{
			if (ptr[j] > 0)
			{
				if (min_x > j) min_x = j;
				if (min_y > i) min_y = i;
				if (max_x < j) max_x = j;
				if (max_y < i) max_y = i;
			}
		}
		
	}
	
	cv::Rect r;
	if (min_x >= max_x || min_y >= max_y)
	{
		r = cv::Rect();
	}
	else
  { 
    r = cv::Rect(cv::Point(min_x, min_y), cv::Point(max_x + 1, max_y + 1));
    updateAllSurfaces(r);
  }
	//qDebug() << outPutImg.cols << ',' << outPutImg.rows;
	//qDebug() <<"Rect:"<< r.x << ',' << r.y << ';' << r.width << ',' << r.height;
	//updateSurface(_surfaceOutPut, r);
}

void LabelingTaskControl::updateImgByTouchedSegments(QImage& Img)
{
	//TODO how to floodfill with segmentation
}

void LabelingTaskControl::updateAllSurfaces(cv::Rect r)
{
	/*if (_surfaceSegmentation) _surfaceSegmentation->update();
	if (_surfaceOriginal) _surfaceOriginal->update();
	if (_surfaceOutPut) _surfaceOutPut->update();*/
  updateSurface(_surfaceSegmentation, r);
  updateSurface(_surfaceOriginal, r);
  updateSurface(_surfaceOutPut, r);
}

void LabelingTaskControl::updateSurface(Surface *sf, cv::Rect rect)
{
	if (sf)
	{
		sf->updateImage(rect);
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
	QImage qIMG = ImageConversion::cvMat_to_QImage(IMG, true, true);
	//copyQImageToQImage(qIMG, _outPutImg, false);
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
			//reAttachOutPutImage();
			updateAllSurfaces();
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
	if(_surfaceSegmentation)
		_surfaceSegmentation->setReferenceImage(&_surfaceOutPut->getOriImage());
	_surfaceOriginal->setReferenceImage(&_surfaceOutPut->getOriImage());
}

void LabelingTaskControl::changeTransparency(int value)
{
	double v = value / 100.0;
	_surfaceOriginal->setBlendAlpha(v, 1.0 - v);
	_surfaceOutPut->setBlendAlpha(v, 1.0 - v);
	if(_surfaceSegmentation)
		_surfaceSegmentation->setBlendAlpha(v, 1.0 - v);
}

void LabelingTaskControl::slotSetCanvasIndex(int index)
{
  this->_canvas_idx = index;
  this->_canvasColor = this->getColorByCanvasIndex(index);
}

cv::Rect LabelingTaskControl::getBoundingRectOfVecPts(vector<cv::Point>& vecPts)
{
	int min_x = INT_MAX, min_y = INT_MAX, max_x = 0, max_y = 0;
	Point pt;
	for (size_t i = 0; i < vecPts.size(); i++)
	{
		pt = vecPts[i];
		if (min_x > pt.x) min_x = pt.x;
		if (min_y > pt.y) min_y = pt.y;
		if (max_x < pt.x) max_x = pt.x;
		if (max_y < pt.y) max_y = pt.y;
	}
	return cv::Rect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1);
}

void LabelingTaskControl::retrieveSegmentsDraw(vector<PtrSegmentPoints>*vecPts, QColor color)
{
	Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg, false);
	int min_x = outPutImg.cols, min_y = outPutImg.rows, max_x = 0, max_y = 0;
	for (size_t i = 0; i < vecPts->size(); i++)
	{
		PtrSegmentPoints pSegPts = (*vecPts)[i];
		for (size_t j = 0; j < pSegPts->size(); j++)
		{
			Point pt = (*pSegPts)[j];
			if (min_x > pt.x) min_x = pt.x;
			if (min_y > pt.y) min_y = pt.y;
			if (max_x < pt.x) max_x = pt.x;
			if (max_y < pt.y) max_y = pt.y;
      if (_canvas_idx > 0)
      {
        if (outPutImg.at<cv::Vec3b>(pt.y, pt.x) == this->_canvasColor)
        {
          outPutImg.at<cv::Vec3b>(pt.y, pt.x) = Vec3b(color.red(), color.green(), color.blue());
        }
      }
      else
      {
        outPutImg.at<cv::Vec3b>(pt.y, pt.x) = Vec3b(color.red(), color.green(), color.blue());
      }
		}
	}
	cv::Rect r;
	if (min_x >= max_x || min_y >= max_y)
	{
		r = cv::Rect();
	}
	else
		r = cv::Rect(cv::Point(min_x, min_y), cv::Point(max_x + 1, max_y + 1));
	//updateSurface(_surfaceOutPut, r);
	updateAllSurfaces(r);
}

void LabelingTaskControl::retrievePolygonDraw(vector<Point> vecPts,QColor clr)
{
  if (vecPts.size() > 0)
  {
    Mat outPutImg = ImageConversion::QImage_to_cvMat(_outPutImg, false);
    vector<vector<Point> > vecvecPts;
    vecvecPts.push_back(vecPts);
    if (_canvas_idx > 0)
    {
      Mat temp;
      outPutImg.copyTo(temp);
      cv::drawContours(temp, vecvecPts, -1, cv::Scalar(clr.red(), clr.green(), clr.blue()), -1);
      Mat mask = this->getClrMask(_canvasColor, outPutImg);
      temp.copyTo(outPutImg, mask);
    }
    else
    {
      cv::drawContours(outPutImg, vecvecPts, -1, cv::Scalar(clr.red(), clr.green(), clr.blue()), -1);
    }
    
		cv::Rect r = getBoundingRectOfVecPts(vecPts);
		if (r.empty())
		{
			r = cv::Rect();
		}
		updateAllSurfaces(r);
	}
}

cv::Vec3b LabelingTaskControl::getColorByCanvasIndex(int idx)
{
  qDebug() << "LabelingTaskControl::getColorByCanvasIndex:" << idx;
  LabelList lst = _selection->getLabelList();
  if (idx != 0)
  {
    int R = std::get<1>(lst[idx - 1]);
    int G = std::get<2>(lst[idx - 1]);
    int B = std::get<3>(lst[idx - 1]);
    return cv::Vec3b(R, G, B);
  }
  else
    return cv::Vec3b(0, 0, 0);
}

cv::Mat LabelingTaskControl::getClrMask(cv::Vec3b clr, Mat& Img)
{
  static Mat mClr(Img.rows, Img.cols, Img.type());
  mClr.setTo(clr);
  vector<Mat> vec_clr;
  vector<Mat> vec_img;
  cv::split(Img, vec_img);
  cv::split(mClr, vec_clr);
  static Mat mask;
  mask = Mat(vec_img[0] == vec_clr[0]);
  for (int i = 1; i < vec_clr.size(); i++)
  {
    mask = mask&(Mat(vec_img[i] == vec_clr[i]));
  }

  return mask;
}

cv::Mat LabelingTaskControl::getDiffClrMask(cv::Vec3b clr,Mat& Img)
{
  Mat mClr(Img.rows, Img.cols, Img.type());
  mClr.setTo(clr);

  vector<Mat> vec_clr;
  vector<Mat> vec_img;
  cv::split(Img, vec_img);
  cv::split(mClr, vec_clr);

  Mat mask = Mat(vec_img[0] == vec_clr[0]);
  for (int i = 1; i < vec_clr.size(); i++)
  {
    mask = mask&(Mat(vec_img[i] == vec_clr[i]));
  }
  return mask;
}
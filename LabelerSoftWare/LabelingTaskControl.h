/*This is a singleton class, designed to control image labeling process*/
#pragma once
#include <qobject.h>
#include "Surface.h"
#include "ClassSelection.h"
#include <opencv.hpp>
#include <QPainterPath>
#include <memory>
#include <QScrollArea>
#include <SmartScrollArea.h>
#include <videocontrol.h>
#include <ProcessControl.h>
#include <SegmentationControl.h>
using cv::Mat;
using std::shared_ptr;
class ProcessControl;
class LabelingTaskControl:public QObject
{
	Q_OBJECT
public:
	LabelingTaskControl(ProcessControl* pProcCtrl, VideoControl* vidCtrl, ClassSelection* selection, QString outPutDir = QString(), bool autoLoadResult = false, QObject* parent = NULL);
	virtual ~LabelingTaskControl();
public:
	void setAutoLoadResult(bool);
	void reAttachOutPutImage();//reAttach the outputSurface with outPutImage
private:
	void closeAllSubWindows();
	void doSegmentation();//TODO

	void setupColorSelectionConnections();//set quick access to color selection panel
	void setupScrollAreaConnections();
	void setupSurfacePainterPathConnections();
	void setupSurfaceHotKeyConnections();
	void setupSurfaceWindowCloseConnections();
	void setupConnections();
	QImage createQImageByMat(Mat& Img, bool revertRGB = true);

	void updateSurface(Surface *sf);
	void updateAllSurfaces();
	void updateImgByTouchedSegments(QImage& Img);
	void updateOutPutImg(QRect boundingRect, QImage& mask);
	
	bool saveResult(QString filePath, bool saveOriginalImg = true);
	bool saveOriginalIMG(QString filePath);
	bool maySaveResult(QString filePath);
	bool checkModified();
	QString getResultSavingPathName();
	QString getOriginalIMGSavingPathName();
	void releaseAll();
	void setupOtherImg();
	void resetSurfaceSource(Surface* surface,QImage* source);
protected:

public slots:
void retrievePainterPath(int PenWidth, QPainterPath& paintPath);
void retrieveSegmentsDraw(vector<PtrSegmentPoints>*vecPts, QColor color);
void saveLabelResult();//save directory is set by constructor
void openSaveDir();//open the directory that will be used to hold saving files
void clearResult();
void loadResultFromDir();
void changeTransparency(int value);

private:
	/*Internal Images*/
	/*QImage& _segImg();
	Mat& _labelImg();
	QImage& _outPutImg();
	QImage& _painterPathImage();*/
	QImage _segImg;
	Mat _labelImg;
	QImage _outPutImg;
	QImage _painterPathImage;

private:
	Surface *_surfaceSegmentation;
	Surface *_surfaceOriginal;
	Surface *_surfaceOutPut;
	SmartScrollArea* _SA1;//ScrollArea1	
	SmartScrollArea* _SA2;//ScrollArea2	
	SmartScrollArea* _SA3;//ScrollArea3	
	QImage _InputImg;
	ClassSelection *_selection;
	SegmentationControl* _segmentation_control;
	QRect _boundingRect;
	bool _modified;
	QString _outPutDir;
	int _frameIdx;
	VideoControl* _pVidCtrl;
	bool _autoLoadResult;
};


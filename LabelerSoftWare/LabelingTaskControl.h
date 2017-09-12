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
using cv::Mat;
using std::shared_ptr;
class ProcessControl;
class LabelingTaskControl:public QObject
{
	Q_OBJECT
public:
	LabelingTaskControl(ProcessControl* pProcCtrl,VideoControl* vidCtrl, ClassSelection* selection, QString outPutDir = QString(), QObject* parent = NULL);
	virtual ~LabelingTaskControl();
private:
	void doSegmentation();//TODO

	void setupColorSelectionConnections();//set quick access to color selection panel
	void setupScrollAreaConnections();
	void setupSurfacePainterPathConnections();
	void setupConnections();
	void unsetConnections();

	void updateSurface(Surface *sf);
	void updateImgByTouchedSegments(QImage& Img);
	void updateOutPutImg(QRect boundingRect, QImage& mask);
	
	bool saveResult(QString filePath);
	bool maySaveResult(QString filePath);
	bool checkModified();
	QString getResultSavingPath();

protected:
	

public slots:
void retrievePainterPath(int PenWidth, QPainterPath& paintPath);
void saveLabelResult();//save directory is set by constructor
void openSaveDir();//open the directory that will be used to hold saving files
void clearResult();
void loadResultFromDir();
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
	void releaseAll();

	void setupOtherImg();
private:
	Surface *_surfaceSegmentation;
	Surface *_surfaceOriginal;
	Surface *_surfaceOutPut;
	SmartScrollArea* _SA1;//ScrollArea1	
	SmartScrollArea* _SA2;//ScrollArea2	
	SmartScrollArea* _SA3;//ScrollArea3	
	QImage _InputImg;
	ClassSelection *_selection;
	QRect _boundingRect;
	bool _modified;
	QString _outPutDir;
	int _frameIdx;
	VideoControl* _pVidCtrl;
};


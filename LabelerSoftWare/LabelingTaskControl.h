/*This is a singleton class, designed to control image labeling process*/
#pragma once
#include <qobject.h>
#include "Surface.h"
#include "ClassSelection.h"
#include <opencv.hpp>
#include <QPainterPath>
#include <memory>
using cv::Mat;
using std::shared_ptr;
class LabelingTaskControl:public QObject
{
	Q_OBJECT
public:
	LabelingTaskControl(QImage& Img, ClassSelection* selection,QObject* parent=NULL);
	virtual ~LabelingTaskControl();
private:
	void doSegmentation();//TODO

	void updateSurface(Surface *sf);
	void updateImgByTouchedSegments(QImage& Img);
	void updateOutPutImg(QRect boundingRect, QImage& mask);
private slots:
void retrievePainterPath(int PenWidth, QPainterPath& paintPath);

private:
	/*Internal Images*/
	QImage& _segImg();
	Mat& _labelImg();
	QImage& _outPutImg();
	QImage& _painterPathImage();
	void releaseAll();

	void setupOtherImg();
private:
	Surface *_surfaceSegmentation;
	Surface *_surfaceOriginal;
	Surface *_surfaceOutPut;
	QImage _InputImg;
	ClassSelection *_selection;
	QRect _boundingRect;
};


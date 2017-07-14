#pragma once
#include <qobject.h>
#include "Surface.h"
#include "ClassSelection.h"
#include <opencv.hpp>
#include <QPainterPath>
using cv::Mat;
class DrawTaskControl:public QObject
{
	Q_OBJECT
private:
	DrawTaskControl(QImage& Img, ClassSelection* selection);
public:
	static DrawTaskControl* getDrawControl(QImage& Img, ClassSelection* selection);
	~DrawTaskControl();
private:
	void doSegmentation();//TODO

private slots:
void retrieveSegments(int PenWidth, QPainterPath& paintPath);

private:
	/*Internal Images*/
	static QImage& _segImg();
	static Mat& _labelImg();
	static QImage& _outPutImg();
	static QImage& _painterPathImage();
	void releaseAll();

	void setupOtherImg();
private:
	Surface *_surfaceSegmentation;
	Surface *_surfaceOriginal;
	Surface *_surfaceOutPut;
	QImage _InputImg;
	
};


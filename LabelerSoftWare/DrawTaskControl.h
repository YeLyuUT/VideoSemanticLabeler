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
	virtual ~DrawTaskControl();
private:
	void doSegmentation();//TODO

	void updateSurface(Surface *sf);
	void updateImgByTouchedSegments(QImage& Img);
	void updateOutPutImg(QRect boundingRect, QImage& mask);
private slots:
void retrievePainterPath(int PenWidth, QPainterPath& paintPath);

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
	ClassSelection *_selection;
	QRect _boundingRect;
};


#include "clickableprogressbar.h"
#include <QDebug>
ClickableProgressBar::ClickableProgressBar(QWidget* parent):QProgressBar(parent)
{
    bLMouseDown=false;
    this->setRange(0,10000);
    this->setFormat(QString("Current Frame Position:%1%").arg(QString::number(0, 'f', 2)));
    this->setValue(0);
}

ClickableProgressBar::~ClickableProgressBar()
{

}

void ClickableProgressBar::mousePressEvent(QMouseEvent*event)
{
	qDebug() << "mousePressEvent:clicked";
	if(clickable)
	{
		qDebug() << "in mousePressEvent:clicked";
		if (event->button() == Qt::LeftButton&&event->type() == QMouseEvent::MouseButtonPress)
		{
			pos = event->pos();
			double ratio = getPosRatio(pos);
			setProgressBarValue(ratio);
			bLMouseDown = true;
		}
	}
}

void ClickableProgressBar::mouseMoveEvent(QMouseEvent *event)
{
	if (clickable)
	{
		if(bLMouseDown)
		{
			pos = event->pos();
			double ratio = getPosRatio(pos);
			setProgressBarValue(ratio);

		}
	}
}
void ClickableProgressBar::mouseReleaseEvent(QMouseEvent *event)
{
	if (clickable)
	{
		if (bLMouseDown)
		{
			pos = event->pos();
			double ratio = getPosRatio(pos);
			setProgressBarValue(ratio);
			emit sendPosRatio(ratio);
			bLMouseDown = false;
		}
	}
}

void ClickableProgressBar::setProgressBarValue(double ratio)
{
    int barPos = getPosFromRatio(ratio);
    QString str;
    str.sprintf("Frame Change To:%d,%3.2f%",barPos,ratio*100);
    this->setFormat(str);
    this->setValue(barPos);
}

int ClickableProgressBar::getWidth()
{
    return this->width();
}
int ClickableProgressBar::getHeight()
{
    return this->height();
}
double ClickableProgressBar::getPosRatio(QPoint pos,double vmin,double vmax)
{
    double ratio =(double)pos.x()/getWidth();
    ratio = fmax(vmin,ratio);
    ratio = fmin(vmax,ratio);
    return ratio;
}

int ClickableProgressBar::getPosFromRatio(double ratio)
{
    double span = this->maximum()-this->minimum();
    return int(ratio*span+this->minimum());
}

void ClickableProgressBar::setClickable(bool b)
{
	clickable = b;
}
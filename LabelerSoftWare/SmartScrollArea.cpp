#include "SmartScrollArea.h"
#include <QScrollBar>
#include <QDebug>
#include <Surface.h>
#include <QApplication>

SmartScrollArea::SmartScrollArea(QWidget*parent) :QScrollArea(parent)
{
	this->setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint);
	keyPressed = Qt::Key::Key_No;
	QScrollBar* barY = this->verticalScrollBar();
	QScrollBar* barX = this->horizontalScrollBar();
	QObject::connect(barX, SIGNAL(valueChanged(int)), this, SLOT(saveXBar(int)));
	QObject::connect(barY, SIGNAL(valueChanged(int)), this, SLOT(saveYBar(int)));
}

void SmartScrollArea::saveXBar(int val)
{
	qDebug() << "SmartScrollArea::saveXBar";
	oldBarValue.setX(val);
}
void SmartScrollArea::saveYBar(int val)
{
	qDebug() << "SmartScrollArea::saveYBar";
	oldBarValue.setY(val);
}

SmartScrollArea::~SmartScrollArea()
{
}

void SmartScrollArea::gentleShiftScrollAreaWhenScaled(QPoint mousePt,double oldRatio, double newRatio)
{
	qDebug() << "gentleShiftScrollAreaWhenScaled.";
	QScrollBar* barY = this->verticalScrollBar();
	QScrollBar* barX = this->horizontalScrollBar();
	double ptX = double(mousePt.x());
	double ptY = double(mousePt.y());
	double X = ptX - /*oldBarValue.x();*/barX->value();
	double Y = ptY - /*oldBarValue.y();*/barY->value();
	double newPtX = ptX / oldRatio*newRatio;
	double newPtY = ptY / oldRatio*newRatio;
	int newBarX = newPtX - X;
	int newBarY = newPtY - Y;
	newBarY = qMax(barY->minimum(), newBarY);
	newBarY = qMin(barY->maximum(), newBarY);
	newBarX = qMax(barX->minimum(), newBarX);
	newBarX = qMin(barX->maximum(), newBarX);
	barY->setValue(newBarY);
	barX->setValue(newBarX);
	/*double ptX = double(mousePt.x()) / oldRatio;
	double ptY = double(mousePt.y()) / oldRatio;
	double ptXNew = double(mousePt.x()) / newRatio;
	double ptYNew = double(mousePt.y()) / newRatio;
	double diffX = 0;
	double diffY = 0;
	diffX = double(ptXNew - ptX)*newRatio;
	diffY = double(ptYNew - ptY)*newRatio;

	int scrollPos_X_New = barX->value() - diffX;
	int scrollPos_Y_New = barY->value() - diffY;
	scrollPos_Y_New = qMax(barY->minimum(), scrollPos_Y_New);
	scrollPos_Y_New = qMin(barY->maximum(), scrollPos_Y_New);
	scrollPos_X_New = qMax(barX->minimum(), scrollPos_X_New);
	scrollPos_X_New = qMin(barX->maximum(), scrollPos_X_New);

	
	if (newRatio > oldRatio)
	{
		this->contentsRect();
	}
	qDebug() << this->contentsRect().x();
	qDebug() << this->contentsRect().y();
	qDebug() << this->contentsRect().width();
	qDebug() << this->contentsRect().height();
	barY->setValue(scrollPos_Y_New);
	barX->setValue(scrollPos_X_New);*/
}

void SmartScrollArea::wheelEvent(QWheelEvent* ev)
{
	qDebug() << "SmartScrollArea::wheelEvent";
	QScrollBar* barY = this->verticalScrollBar();
	QScrollBar* barX = this->horizontalScrollBar();
	oldBarValue.setX(barX->value());
	oldBarValue.setY(barY->value());
	return QScrollArea::wheelEvent(ev);
}

void SmartScrollArea::keyPressEvent(QKeyEvent*ev)
{
	//qDebug() << "SmartScrollArea::keyPressEvent";
	switch (ev->key())
	{
	case Qt::Key::Key_W://move up	
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() - 100);	
		break;
	case Qt::Key::Key_S://move down
		this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + 100);
		break;
	case Qt::Key::Key_A://move left
		this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() - 100);
		break;
	case Qt::Key::Key_D://move right
		this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + 100);
		break;
	case Qt::Key::Key_E://zoom in

		break;
	case Qt::Key::Key_Q://zoom out
		break;
	case Qt::Key::Key_Shift://color selection
		break;
	default:
		break;
	}
	//if (qobject_cast<QKeyEvent*>())
	//{

	//}
	//ev->ignore();
	//return QScrollArea::keyPressEvent(ev);
}

void SmartScrollArea::keyReleaseEvent(QKeyEvent*ev)
{
	return QScrollArea::keyReleaseEvent(ev);
}
#include "SmartScrollArea.h"
#include <QScrollBar>
#include <QDebug>
#include <Surface.h>
#include <QApplication>

SmartScrollArea::SmartScrollArea(QWidget*parent) :QScrollArea(parent)
{
	keyPressed = Qt::Key::Key_No;
}


SmartScrollArea::~SmartScrollArea()
{
}

void SmartScrollArea::gentleShiftScrollAreaWhenScaled(QPoint mousePt, double oldRatio, double newRatio)
{
	qDebug() << "gentleShiftScrollAreaWhenScaled.";
	QScrollBar* barY = this->verticalScrollBar();
	QScrollBar* barX = this->horizontalScrollBar();
	double ptX = (mousePt.x()) / oldRatio;
	double ptY = (mousePt.y()) / oldRatio;
	double ptXNew = (mousePt.x()) / newRatio;
	double ptYNew = (mousePt.y()) / newRatio;
	double diffX = (ptXNew - ptX)*newRatio;
	double diffY = (ptYNew - ptY)*newRatio;

	int scrollPos_X_New = barX->value() - diffX;
	int scrollPos_Y_New = barY->value() - diffY;

	scrollPos_Y_New = qMax(barY->minimum(), scrollPos_Y_New);
	scrollPos_Y_New = qMin(barY->maximum(), scrollPos_Y_New);
	scrollPos_X_New = qMax(barX->minimum(), scrollPos_X_New);
	scrollPos_X_New = qMin(barX->maximum(), scrollPos_X_New);

	barY->setValue(scrollPos_Y_New);
	barX->setValue(scrollPos_X_New);
}

void SmartScrollArea::wheelEvent(QWheelEvent* ev)
{
	return QScrollArea::wheelEvent(ev);
}

void SmartScrollArea::keyPressEvent(QKeyEvent*ev)
{
	qDebug() << "SmartScrollArea::keyPressEvent";
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
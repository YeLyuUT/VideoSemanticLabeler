#pragma once
#include "qlabel.h"
#include "qimage.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <opencv.hpp>
#include <ClassSelection.h>
#include <QPainterPath>
using cv::Mat;
using cv::Vec3b;
class Surface :	public QLabel
{
	Q_OBJECT
public:
	Surface(QImage &Img,QWidget*parent=NULL);
	~Surface();
public:
	void expandSize();
	void shrinkSize();
	void drawImage();
	void fitSizeToImage();
public:
	Vec3b getLabelColor();
	int getSegmentIdx();

signals:
	void openClassSelection();
	void closeClassSelection();
	void painterPathCreated(int PenWidth,QPainterPath& painterPath);
public slots:
void changeClass(QString txt, QColor clr);
private:
	void setCursorInvisible(bool);
	void paintCursor();
	void drawLineTo(const QPoint &endPoint);
	void updateCursorArea(bool drawCursor);
	void updateRectArea(QRect rect, int rad, bool drawCursor);
protected:
	void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent*ev) Q_DECL_OVERRIDE;
	void leaveEvent(QEvent*ev) Q_DECL_OVERRIDE;
	void enterEvent(QEvent*ev) Q_DECL_OVERRIDE;
private:
	QImage _pImage;
	bool LButtonDown;
	bool bSelectClass;
	bool bDrawCursor;

	QPoint lastPoint;
	QPainterPath paintPath;
	const int _cursorEdgeWidth = 2;

	QPoint _mousePos;
	int myPenRadius;
	QColor myPenColor;
};


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
	Surface(QImage Img, QWidget*parent = NULL);
	virtual ~Surface();
public:
	void setOriginalImage(QImage pOriginal);
	void setReferenceImage(QImage* pReference = NULL);
public:
	void expandSize();
	void shrinkSize();
	void setEditable(bool b = true);
	void startLabel();
	void endLabel();
	/*when image is editable, _ImageDraw is a copy of _oriImage;
	otherwise,  _ImageDraw and _oriImage are the same.
	*/
	bool isEditable();
	void fitSizeToImage();
	/*when original image is modified, this should
	be called to update qimage for drawing*/
	void updateImage();
public:
	Vec3b getLabelColor();
	int getSegmentIdx();

signals:
	void openClassSelection();
	void closeClassSelection();
	void painterPathCreated(int PenWidth,QPainterPath& painterPath);
	void mousePositionShiftedByScale(QPoint mousePt, double oldScaleRatio, double newScaleRatio);
public slots:
void changeClass(QString txt, QColor clr);
void applyScaleRatio();

private:
	/*These functions set showing image scale*/
	void setScaleRatio(double ratio, double maximum = 8.0, double minimum = 0.125);
	double getScaleRatio();
	void setScaleRatioRank(int rank, int maximum = 10.0, int minimum = -10);
	int getScaleRatioRank();
	void updateScaleRatioByRank();

	void showNormal();
	void showScaled();
	void showReferenceImg();
	void showInternalImg();
	void showScaledRefImg(QImage* Img);

	void setCursorInvisible(bool);
	void paintCursor();
	void drawLineTo(const QPoint &endPoint);
	void updateCursorArea(bool drawCursor);//updateCursorArea(false) can clean cursor;updateCursorArea(true) can redraw cursor
	void updateRectArea(QRect rect, int rad, bool drawCursor);

	QPoint getPointAfterNewScale(QPoint pt,double scaleOld,double scaleNew);

	QPoint FilterScalePoint(QPoint pt);
protected:
	void keyPressEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent *ev) Q_DECL_OVERRIDE;
	void mousePressEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void mouseMoveEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void mouseReleaseEvent(QMouseEvent *ev) Q_DECL_OVERRIDE;
	void mouseDoubleClickEvent(QMouseEvent* ev)Q_DECL_OVERRIDE;
	void paintEvent(QPaintEvent *ev) Q_DECL_OVERRIDE;
	void wheelEvent(QWheelEvent*ev) Q_DECL_OVERRIDE;
	void leaveEvent(QEvent*ev) Q_DECL_OVERRIDE;
	void enterEvent(QEvent*ev) Q_DECL_OVERRIDE;

private:
	QImage _oriImage;
	QImage _ImageDraw;
	QImage* _referenceImage;
	bool _bLButtonDown;
	bool _bSelectClass;
	bool _bDrawCursor;
	bool _bEdit;

	QPoint _lastPoint;
	QPainterPath _tempDrawPath;//for temp draw stroke display
	QPainterPath _paintPath;//the real path matched with the original scale image
	const int _cursorEdgeWidth = 2;

	QPoint _mousePos;
	int _myPenRadius;
	QColor _myPenColor;
private:
	double _scaleRatio;
	int _scaleRatioRank;
};


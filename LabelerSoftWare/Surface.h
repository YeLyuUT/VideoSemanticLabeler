#pragma once
#include "qlabel.h"
#include "qimage.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <opencv.hpp>
#include <ClassSelection.h>
#include <QPainterPath>
#include <QScrollArea>
#include <tuple>
#include <SegmentationControl.h>
#include <chrono>
using cv::Mat;
using cv::Vec3b;
using cv::Point;
using std::tuple;
using std::chrono::steady_clock;
typedef tuple<vector<QColor>, vector<Point> >  SavedPixels;

class Surface :	public QLabel
{
public:
	enum DRAW_TYPE { PIXEL_WISE, SUPER_PIXEL_WISE }_drawType;
	Q_OBJECT
public:
	Surface(const QImage& Img, QWidget*parent = NULL);
	virtual ~Surface();
public:
	QImage getImage();
	const QImage& getOriImage();
	QImage getImageCpy();
	QImage getOriImageCpy();
public:
	void setOriginalImage(const QImage& pOriginal);
	void setReferenceImage(const QImage* pReference = NULL);
	void setReferenceOriginalImage(const QImage* pReference = NULL);
	void setScrollArea(QScrollArea* pScrollArea=NULL);
public:
	void setBlendAlpha(double source = 0.5, double reference = 0.5);
	void zoom(int step, QPoint pt = QPoint(0, 0));//step is the relative scale ratio rank, pt is the focus point.
	void setEditable(bool b = true);
	void startLabel();
	void endLabel();
	void setDrawType(DRAW_TYPE type);
	double getZoomRatio();
	/*when image is editable, _ImageDraw is a copy of _oriImage;
	otherwise,  _ImageDraw and _oriImage are the same.
	*/
	bool isEditable();
	void fitSizeToImage();
	/*when original image is modified, this should
	be called to update qimage for drawing*/
	void updateImage(cv::Rect rect=cv::Rect());
public:
	Vec3b getLabelColor();
	int getSegmentIdx();

	void setAllowPolyMode(bool bAllow);
	bool getAllowPolyMode();

signals:
	void openClassSelection();
	void closeClassSelection();
	void painterPathCreated(int PenWidth,QPainterPath& painterPath);
	void mousePositionShiftedByScale(QPoint mousePt, double oldScaleRatio, double newScaleRatio);
	void clearResult();
	void signalPixelCovered(vector<Point>* vecPts);//send out pixel covereded by cursor
	void signalDrawPixelsToResult(vector<PtrSegmentPoints>*vecPts, QColor color);
	void signalSendPolygonDraw(vector<Point> vecPts, QColor clr);
public slots:
void changeClass(QString txt, QColor clr);
void applyScaleRatio();
void slotPixelCovered(vector<PtrSegmentPoints>* vecPts);//retrieve pixel need to draw

private:
	/*These functions set showing image scale*/
	void setScaleRatio(double ratio, double maximum = 2.0, double minimum = 0.25);
	double getScaleRatio();
	void setScaleRatioRank(int rank, int maximum = 2, int minimum = -4);
	int getScaleRatioRank();
	void updateScaleRatioByRank();
	void showNormal();
	void showScaled();
	void showReferenceImg();
	void showReferenceOriginalImg();
	void updateShowReferenceImg(cv::Rect rect);//rect none scaled original
	void updateRectOfImg(cv::Rect rect);//rect of true size,correspond to none scaled image
	void showInternalImg();
	void showScaledRefImg(const QImage* Img,cv::Rect rect=cv::Rect());

	
	vector<QPolygon> getMouseCursorTriangles(int penWidth,double angle=5.0);//angle in degree
	int getMyPenRadius();
	void setMyPenRadius(int val);
	void setCursorInvisible(bool);
	void paintCursor();
	void drawClipedMatToRect(QPainter& painter, Mat&clipMat, cv::Rect rect);
	void drawLineTo(const QPoint &endPoint);
	void drawCircle(const QPoint &Point);
	void drawPolygonToQImage(vector<cv::Point>&vecPts, QImage& image);
	void drawPolytonToMat(vector<cv::Point>&vecPts, Mat& image);
	void updateCursorArea(bool drawCursor);//updateCursorArea(false) can clean cursor;updateCursorArea(true) can redraw cursor
	void updateRectArea(QRect rect, int rad, bool drawCursor);

	void setVecPointsWithinRadiusOfPoint(vector<Point>&vecPts, vector<Point>&circleInnerPoint, Point center, int width, int height);
	void getCircleInnerPoints(vector<Point>&circleInnerPoint, int radius);

	void flipColor(QImage& IMG, vector<Point>& vecPts);
	void pushToSavedPixels(QColor& color, Point& Pt);
	void restoreSavedPixels(QImage&IMG, SavedPixels& savedPixels);
	void restoreSavedBoundingBoxImage(QImage&IMG, Mat&savedMat,cv::Rect&savedRect);
	QColor getSavedPixelColor(int idx);
	Point getSavedPixelPos(int idx);
	void clearSavedPixels();
	QPoint getPointAfterNewScale(QPoint pt,double scaleOld,double scaleNew);
	QPoint FilterScalePoint(QPoint pt);

	QImage blendImage(const QImage& img1, double ratio1, const QImage& img2, double ratio2, cv::Rect rect = cv::Rect());

	vector<Point> transformVecPtsByScaleAndPos(vector<Point>& vecPts, double scale,Point offset);//scale first then offset
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
	bool eventFilter(QObject* obj,QEvent* evt) Q_DECL_OVERRIDE;

private:
	std::chrono::time_point<steady_clock> _timePoint;
	const QImage* _oriImage;//This image will not be changed
	QImage _scaledOriImage;
	QImage _ImageDraw;
	const QImage* _referenceImage;
	const QImage* _referenceOriginalImage;
	Mat _blendImage;
	bool _bLButtonDown;
	bool _bShowRef;
	bool _bSelectClass;
	bool _bDrawCursor;
	bool _bEdit;//false if the image is in play mode, false if the image can be editted.
	bool _bColorFlipped;
	bool _bUpdateClipMat;
	bool _startPolygonMode;
	bool _allowPolygonMode;
	SavedPixels _savedPixels;
	Mat _drawClipMat;//temp clip mat for drawImage
	cv::Rect _savedBoundingRect;
	QScrollArea* _scrollArea;
	QPoint _lastPoint;
	QPainterPath _tempDrawPath;//for temp draw stroke display
	QPainterPath _paintPath;//the real path matched with the original scale image
	const int _cursorEdgeWidth = 2;
	double blendAlphaSource;
	double blendAlphaReference;
	QPoint _mousePos;
	vector<Point> _circleInnerPoint;
	vector<Point> _tempVecPoint;//scaled version
	vector<Point> _vecPtsToEmit;
	vector<PtrSegmentPoints> _tempVecSegs;
	vector<Point> _rightClickCache;
	vector<QPolygon> _mouseCursorTriangles;
	static int _myPenRadius;
	static QColor _myPenColor; 
private:
	double _scaleRatio;
	int _scaleRatioRank;//level of scale
	int _seg_drawn_num;

private:
	const double _alpha_src = 0.6;
	const double _alpha_dst = 0.4;
};

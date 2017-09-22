#include "Surface.h"
#include <qdebug.h>
#include "ImageConversion.h"
#include <QPainter>
#include <QRect>
#include <qmessagebox.h>

//#define CHECK_QIMAGE

#ifdef CHECK_QIMAGE
#include <QtDebug.h>
#endif // CHECK_QIMAGE


QColor Surface::_myPenColor(0, 0, 0);
int Surface::_myPenRadius = 10;

Surface::Surface(const QImage& Img, QWidget*parent) :QLabel(parent), _blendImage(Img.height(), Img.width(), CV_8UC3), _oriImage(&Img)
{
	//labelImg() = Mat(400, 400, CV_8UC3);//TODO labelImg should be replace
	//_ImageDraw = ImageConversion::cvMat_to_QImage(labelImg());
	_drawType = PIXEL_WISE;
	
	setOriginalImage(Img);
	
	this->setMouseTracking(true);
	_bLButtonDown = false;
	_bColorFlipped = false;
	_bSelectClass = false;
	setCursorInvisible(false);
	_bDrawCursor = true;
	_bShowRef = false;
	_scaleRatioRank = 0;
	_scaleRatio = 1.0;
	_bEdit = false;
	_scrollArea = nullptr;
	_referenceOriginalImage = nullptr;
	blendAlphaSource = 0.5;
	blendAlphaReference = 0.5;
	fitSizeToImage();
}

Surface::~Surface()
{
#ifdef CHECK_QIMAGE
	cv::destroyAllWindows();
#endif
}

QImage Surface::getImage()
{
	return this->_ImageDraw;
}

const QImage& Surface::getOriImage()
{
	return *(this->_oriImage);
}

QImage Surface::getImageCpy()
{
	return this->_ImageDraw.copy();
}

QImage Surface::getOriImageCpy()
{
	return this->_oriImage->copy();
}

void Surface::fitSizeToImage()
{
	int width = _oriImage->width();
	int height = _oriImage->height();
	this->resize(width, height);
}

void Surface::setEditable(bool b)
{
	_bEdit = b;
	setCursorInvisible(b);
	_ImageDraw = _oriImage->copy();
}
void Surface::startLabel()
{
	setEditable(true);
}
void Surface::endLabel()
{
	setEditable(false);
}

void Surface::setDrawType(DRAW_TYPE type)
{
	_drawType = type;
	getCircleInnerPoints(_circleInnerPoint, _myPenRadius);
}

bool Surface::isEditable()
{
	return _bEdit;
}

void Surface::setOriginalImage(const QImage& pOriginal)
{
	_oriImage = &pOriginal;
	if (isEditable())
	{
		_ImageDraw = _oriImage->copy();
	}
	else
	{
		_ImageDraw = *_oriImage;
	}
	//this->setPixmap(QPixmap::fromImage(_ImageDraw));
}

void Surface::setScrollArea(QScrollArea* pScrollArea)
{
	_scrollArea = pScrollArea;
}

void Surface::setReferenceImage(const QImage* pReference)
{
	_referenceImage = pReference;
}

void Surface::setReferenceOriginalImage(const QImage* pReference)
{
	_referenceOriginalImage = pReference;
}

void Surface::changeClass(QString txt, QColor clr)
{
	if (isEditable())
	{
		qDebug() << "Surface::changeClass to: \n" << txt << clr;
		_myPenColor = clr;
	}
}

void Surface::showNormal()
{
	this->setScaleRatio(1.0);
	this->applyScaleRatio();
	update();
}

void Surface::showScaled()
{
	//this->updateScaleRatioByRank();
	this->applyScaleRatio();
	_ImageDraw = _scaledOriImage.copy();
	update();
}

void Surface::showScaledRefImg(const QImage* Img)
{
	if (!_oriImage->isNull() && _scaleRatio != 1.0)
	{
		int width = _oriImage->width()*_scaleRatio;
		int height = _oriImage->height()*_scaleRatio;
		width = width < 1 ? 1 : width;
		height = height < 1 ? 1 : height;
		_ImageDraw = Img->scaled(QSize(width, height), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
	}
	else
		_ImageDraw = Img->copy();
	update();
}

void Surface::showReferenceImg()
{
	if (_referenceImage)
	{
		QImage img;
		img = blendImage(*_oriImage, blendAlphaSource, *_referenceImage, blendAlphaReference);
		showScaledRefImg(&img);
		//showScaledRefImg(_referenceImage); ??please uncomment this to restore
	}
	else
	{
		_ImageDraw.fill(0);
		update();
	}
}

void Surface::updateShowReferenceImg(cv::Rect rect)
{
	if (_referenceImage)
	{
		QImage img;
		img = blendImage(*_oriImage, blendAlphaSource, *_referenceImage, blendAlphaReference, rect);
		showScaledRefImg(&img);
		//showScaledRefImg(_referenceImage); ??please uncomment this to restore
	}
	else
	{
		_ImageDraw.fill(0);
		update();
	}
}

void Surface::showReferenceOriginalImg()
{
	if (_referenceOriginalImage)
	{
		showScaledRefImg(_referenceOriginalImage);
		//showScaledRefImg(_referenceImage); ??please uncomment this to restore
	}
	else
	{
		_ImageDraw.fill(0);
		update();
	}
}

void Surface::showInternalImg()
{
	showScaled();
}

bool Surface::eventFilter(QObject* obj, QEvent* evt)
{
	if (obj != nullptr)
	{
		if ((QScrollArea*)obj == _scrollArea)
		{
			if (evt->type() == QEvent::KeyPress)	
			{
				keyPressEvent((QKeyEvent *)evt);	
			}
			else if (evt->type() == QEvent::KeyRelease)
			{
				keyReleaseEvent((QKeyEvent *)evt);	
			}
		}
	}
	return false;
}

void Surface::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		if (isEditable()) 
		{
			emit openClassSelection();
			qDebug() << "emit openClassSelection()" ;
		}
		break;
	case Qt::Key::Key_V:
		//qDebug() << "showNormal()";
		this->showNormal(); break;
	case Qt::Key::Key_Tab:
		//qDebug() << "showReferenceImg()";
		if (!_bShowRef)
		{
			this->showReferenceImg();
			_bShowRef = true;
			//qDebug() << "_bTabKeyDown";
		}
		else
		{
			this->showInternalImg();
			_bShowRef = false;
		}
		break;
	case Qt::Key::Key_1:
		this->showReferenceOriginalImg(); break;
	case Qt::Key::Key_R:
		emit clearResult(); break;
	//case Qt::Key::Key_Space:qDebug() << "Key_Space" ; break;

	default:break;
	}
	QLabel::keyPressEvent(ev);
}
void Surface::keyReleaseEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		if (isEditable())
		{
			emit closeClassSelection();
			qDebug() << "emit closeClassSelection()" ;
		}
		break;
	case Qt::Key::Key_V:
		this->showScaled(); break;
	case Qt::Key::Key_Tab:

		break;
	case Qt::Key::Key_1:
		this->showInternalImg(); break;
		//case Qt::Key::Key_Space:qDebug() << "Key_Space" ; break;
	default:break;
	}
	QLabel::keyReleaseEvent(ev);
}

void Surface::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);

	/* draw background image */
	QRect dirtyRect = ev->rect();
	painter.drawImage(dirtyRect, _ImageDraw, dirtyRect);

	if (isEditable())
	{
		if (_drawType == DRAW_TYPE::PIXEL_WISE)
		{
			/* draw stroke trace */
			if (_bLButtonDown)
			{
				painter.setPen(QPen(_myPenColor, (_myPenRadius * 2 + 1), Qt::SolidLine, Qt::RoundCap,
					Qt::RoundJoin));
				painter.drawPath(_tempDrawPath);
				//qDebug() << "painter.drawPath";
			}
		}
		else if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
		{
			//TODO
			drawClipedMatToRect(painter, _drawClipMat, _savedBoundingRect);
			/*_tempVecPoint.clear();
			if (_bLButtonDown)
			{
				for (size_t i = 0; i < _tempVecSegs.size(); i++)
				{
					PtrSegmentPoints pSegPts = _tempVecSegs[i];
					for (size_t j = 0; j < pSegPts->size(); j++)
					{
						Point p = (*pSegPts)[j];
						p *= _scaleRatio;
						if (p.x >= 0 && p.y >= 0 && p.x < _ImageDraw.width() && p.y < _ImageDraw.height())
						{
							_tempVecPoint.push_back(p);
						}
					}
				}
				Mat 
				for (size_t i = 0; i < _tempVecPoint.size(); i++)
				{
					Point pt = _tempVecPoint[i] - r.tl();

					_drawClipMat.at<cv::Vec3b>(pt.y, pt.x) = Mat(drawIMG, r).at<cv::Vec3b>(pt.y, pt.x)*0.6
						+ cv::Vec3b(_myPenColor.red(), _myPenColor.green(), _myPenColor.blue())*0.4;
				}
			}*/
		}
		/*draw cursor*/
		if (_bDrawCursor)
			paintCursor();
	}
	//qDebug() << "paint event";
	//qDebug() << "paintEvent";
	ev->accept();
	QLabel::paintEvent(ev);
}

void Surface::mousePressEvent(QMouseEvent *ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton:
		if (isEditable())
		{
			
			qDebug() << "Qt::LeftButton Press";
			if (_drawType == DRAW_TYPE::PIXEL_WISE)
			{
				_bLButtonDown = true; _lastPoint = ev->pos(); _paintPath = QPainterPath(); _tempDrawPath = QPainterPath();
				if (_bLButtonDown)
				{
					qDebug() << "Draw first Pt: " << '(' << ev->x() << ':' << ev->y() << ')';
					QPoint endPoint = ev->pos() + QPoint(0, 1);
					drawLineTo(endPoint);
					updateRectArea(QRect(_lastPoint, endPoint).normalized(), _myPenRadius, false);
					//_lastPoint = endPoint;
				}
			}
			else if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
			{
				//TODO
				_bLButtonDown = true;
				if (_bLButtonDown)
				{
					this->update(_savedBoundingRect.x, _savedBoundingRect.y, _savedBoundingRect.width, _savedBoundingRect.height);
					QPoint center = ev->pos();
					center /= _scaleRatio;
					//qDebug() << center;
					setVecPointsWithinRadiusOfPoint(_tempVecPoint, _circleInnerPoint, Point(center.x(), center.y()), _oriImage->width(), _oriImage->height());
					emit signalPixelCovered(&_tempVecPoint);
				}
			}
			updateCursorArea(false);//erase old cursor
			_mousePos = ev->pos();
			updateCursorArea(true);//draw new cursor
		}
		break;
	case Qt::RightButton: qDebug() << "Qt::RightButton Press";

		break;
	case Qt::MidButton://qDebug() << "Qt::MidButton Press" ; 
		break;
	default://qDebug() << "mousePress Default case." ; 
		break;
	}
	QLabel::mousePressEvent(ev);
}

void Surface::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug() << '(' << ev->x() << ':' << ev->y() << ')' ;
	if (isEditable())
	{
		
		if (_drawType == DRAW_TYPE::PIXEL_WISE)
		{
			if (_bLButtonDown)
			{
				qDebug() << "Draw: " << '(' << ev->x() << ':' << ev->y() << ')';
				QPoint endPoint = ev->pos();
				drawLineTo(endPoint);
				updateRectArea(QRect(_lastPoint, endPoint).normalized(), _myPenRadius, false);
				_lastPoint = endPoint;
			}
		}
		else if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
		{
			this->update(_savedBoundingRect.x, _savedBoundingRect.y, _savedBoundingRect.width, _savedBoundingRect.height);
			QPoint center = ev->pos();
			center /= _scaleRatio;
			//qDebug() << center;
			setVecPointsWithinRadiusOfPoint(_tempVecPoint, _circleInnerPoint, Point(center.x(), center.y()), _oriImage->width(), _oriImage->height());
			emit signalPixelCovered(&_tempVecPoint);
			
		}
		updateCursorArea(false);
		_mousePos = ev->pos();
		updateCursorArea(true);

	}
	QLabel::mouseMoveEvent(ev);
}

void Surface::mouseReleaseEvent(QMouseEvent *ev)
{
	if (isEditable())
	{
		if (_drawType == DRAW_TYPE::PIXEL_WISE)
		{
			if (_bLButtonDown)
			{
				_bLButtonDown = false;
				emit painterPathCreated((_myPenRadius * 2 + 1) / _scaleRatio, _paintPath);
				updateRectArea(_tempDrawPath.boundingRect().toRect(), _myPenRadius, false);
				_paintPath = QPainterPath();
				_tempDrawPath = QPainterPath();
			}
		}
		else if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
		{
			//TODO
			if (_bLButtonDown)
			{
				if (_bShowRef)
					updateShowReferenceImg(cv::Rect());
				_bLButtonDown = false;
			}
		}

//#ifdef CHECK_QIMAGE
//		Mat temp = ImageConversion::QImage_to_cvMat(_ImageDraw, false);
//		cv::imshow("QImage", temp);
//		cv::waitKey(1);
//#endif
	}
	QLabel::mouseReleaseEvent(ev);
}

void Surface::mouseDoubleClickEvent(QMouseEvent* ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton:
		qDebug() << "Left Double Clicked"; break;
	case Qt::RightButton:
		qDebug() << "Right Double Clicked"; break;
	default:
		break;
	}
	QLabel::mouseDoubleClickEvent(ev);
}

void Surface::wheelEvent(QWheelEvent*ev)
{
	QPoint numDegrees = ev->angleDelta() / 8;
	QPoint numSteps = numDegrees / 15;
	qDebug() << "Surface::wheelEvent";
	qDebug() << "numDegrees:" << numDegrees;
	switch (ev->modifiers())
	{
	case Qt::KeyboardModifier::ControlModifier:
		if (isEditable())
		{
			updateCursorArea(false);
			setMyPenRadius(getMyPenRadius() + numSteps.y() * 2);//change pen radius	
			updateCursorArea(true);
			qDebug() << "myPenRadius" << _myPenRadius;
		}
		break;
	case Qt::KeyboardModifier::AltModifier:
		{
			zoom(numSteps.x(), ev->pos());
			qDebug() << "AltModifier";
		}
		break;
	default:
		break;
	}
	
	ev->accept();
}

void Surface::leaveEvent(QEvent*ev)
{
	if (isEditable())
	{
		_bDrawCursor = false;
		updateCursorArea(false);
	}
	//qDebug() << "leaveEvent";
	QLabel::leaveEvent(ev);
}

void Surface::enterEvent(QEvent*ev)
{
	if (isEditable())
	{
		_bDrawCursor = true;
	}
	QLabel::enterEvent(ev);
}

void Surface::updateRectArea(QRect rect, int rad, bool drawCursor)
{
	bool store = _bDrawCursor;
	_bDrawCursor = drawCursor;
	update(rect.adjusted(-rad, -rad, +rad, +rad).adjusted(-3, -3, 3, 3));
	_bDrawCursor = store;
}

QPoint Surface::FilterScalePoint(QPoint pt)
{
	QPoint rtv;
	rtv.setX(pt.x() / _scaleRatio);
	rtv.setY(pt.y() / _scaleRatio);
	return rtv;
}

void Surface::drawCircle(const QPoint &Point)
{
	double radius = _myPenRadius / _scaleRatio;
	_paintPath.addEllipse(FilterScalePoint(Point), radius, radius);
	_tempDrawPath.addEllipse(Point, _myPenRadius, _myPenRadius);
}

void Surface::drawClipedMatToRect(QPainter& painter,Mat&clipMat, cv::Rect rect)
{
	QRect dirtyRect(rect.x, rect.y, rect.width, rect.height);
	QImage qImg = ImageConversion::cvMat_to_QImage(clipMat, true, false);
	painter.drawImage(dirtyRect, qImg);
}


void Surface::drawLineTo(const QPoint &endPoint)
{
	/*QPainter painter(&_ImageDraw);
	painter.setPen(QPen(myPenColor, myPenRadius, Qt::SolidLine, Qt::RoundCap,
	Qt::RoundJoin));*/
	_paintPath.moveTo(FilterScalePoint(_lastPoint));
	_paintPath.lineTo(FilterScalePoint(endPoint));
	_tempDrawPath.moveTo(_lastPoint);
	_tempDrawPath.lineTo(endPoint);
	//painter.drawLine(lastPoint, endPoint);

	//int rad = (myPenRadius / 2) + 2;
	//update(QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));

}

void Surface::paintCursor()
{
	QPainter painter(this);
	painter.setPen(QPen(_myPenColor, _cursorEdgeWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	painter.setBrush(QBrush(_myPenColor, Qt::DiagCrossPattern));
	painter.drawEllipse(_mousePos, _myPenRadius, _myPenRadius);
}

void Surface::updateImage()
{
	//_ImageDraw = _oriImage.copy();

#ifdef CHECK_QIMAGE
	qt_debug::showQImage(_oriImage);
#endif //CHECK_QIMAGE 

	applyScaleRatio();
}

void Surface::updateCursorArea(bool drawCursor)
{
	bool beforeCursor = _bDrawCursor;
	_bDrawCursor = drawCursor;
	this->update(QRect(_mousePos.x() - (_myPenRadius + _cursorEdgeWidth), _mousePos.y() - (_myPenRadius + _cursorEdgeWidth), \
		(_myPenRadius + _cursorEdgeWidth) * 2 + 3, (_myPenRadius + _cursorEdgeWidth) * 2 + 3));
	_bDrawCursor = beforeCursor;
}

void Surface::setCursorInvisible(bool b)
{
	if(b)
		this->setCursor(Qt::BlankCursor);
	else
		this->setCursor(Qt::ArrowCursor);
}

QPoint Surface::getPointAfterNewScale(QPoint pt, double scaleOld, double scaleNew)
{
	double x = pt.x();
	double y = pt.y();
	x = x / scaleOld*scaleNew;
	y = y / scaleOld*scaleNew;
	return QPoint(x, y);
}

void Surface::setScaleRatio(double ratio, double maximum, double minimum)
{
	if (ratio <= 0.0)
	{
		QMessageBox::warning(NULL,"ScaleRatio Error","The resize scale cannot be lower than 0",QMessageBox::StandardButton::Ok);
	}
	else
	{
		_scaleRatio = ratio;
		int temp_scaleRank = 0;
		if (_scaleRatio > maximum)
		{
			_scaleRatio = qMin(maximum, _scaleRatio);
			temp_scaleRank = int(log(_scaleRatio) / log(1.25)) + 1;
			setScaleRatioRank(temp_scaleRank);
			_scaleRatio = pow(1.25, temp_scaleRank);
		}
		else if (_scaleRatio < minimum)
		{
			_scaleRatio = qMax(minimum, _scaleRatio);
			temp_scaleRank = int(log(_scaleRatio) / log(1.25));
			setScaleRatioRank(temp_scaleRank);
			_scaleRatio = pow(1.25, temp_scaleRank);
		}
		
		
		qDebug() << "_scaleRatio:" << _scaleRatio;
	}
}

double Surface::getScaleRatio()
{
	return _scaleRatio;
}

void Surface::setScaleRatioRank(int rank, int maximum, int minimum)
{
	_scaleRatioRank = rank;
	_scaleRatioRank = qMin(maximum, _scaleRatioRank);
	_scaleRatioRank = qMax(minimum, _scaleRatioRank);
	qDebug() << "setScaleRatioRank:" << _scaleRatioRank;
}

int Surface::getScaleRatioRank()
{
	qDebug() << "getScaleRatioRank:" << _scaleRatioRank;
	return _scaleRatioRank;
}

void Surface::updateScaleRatioByRank()
{
	if (_scaleRatioRank > 0)
	{
		setScaleRatio(pow(1.25, _scaleRatioRank));
	}
	else if (_scaleRatioRank < 0)
	{
		setScaleRatio(pow(0.8, -_scaleRatioRank));
	}
	else
	{
		setScaleRatio(1);
	}
}

void Surface::applyScaleRatio()
{
	if (!_oriImage->isNull() && _scaleRatio != 1.0)
	{
		int width = _oriImage->width()*_scaleRatio;
		int height = _oriImage->height()*_scaleRatio;
		width = width < 1 ? 1 : width;
		height = height < 1 ? 1 : height;
		_scaledOriImage = _oriImage->scaled(QSize(width, height), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
	}
	else
		_scaledOriImage = _oriImage->copy();
	if (_bShowRef)
		showReferenceImg();
	else
		_ImageDraw = _scaledOriImage.copy();
	
}

void Surface::zoom(int step, QPoint pt)
{
	double oldScaleRatio = getScaleRatio();
	setScaleRatioRank(getScaleRatioRank() + step);
	updateScaleRatioByRank();
	double newScaleRatio = getScaleRatio();
	if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
	{
		getCircleInnerPoints(_circleInnerPoint, _myPenRadius);
	}
	QPoint newPos = getPointAfterNewScale(pt, oldScaleRatio, newScaleRatio);
	emit mousePositionShiftedByScale(pt, oldScaleRatio, newScaleRatio);
	applyScaleRatio();

	this->resize(_ImageDraw.width(), _ImageDraw.height());
	update();
}

double Surface::getZoomRatio()
{
	return this->_scaleRatio;
}

QImage Surface::blendImage(const QImage& img1, double ratio1, const QImage& img2, double ratio2, cv::Rect rect)
{
	assert(img1.height() == img2.height() && img1.width() == img2.width());
	Mat m1 = ImageConversion::QImage_to_cvMat(img1, false);
	Mat m2 = ImageConversion::QImage_to_cvMat(img2, false);
	assert(m1.type() == m2.type() && m1.type() == CV_8UC3);
	
	if (!rect.empty() && !_blendImage.empty())
	{
		qDebug() << "Partial Change blend";
		cv::addWeighted(Mat(m1, rect), ratio1, Mat(m2, rect), ratio2, 0, Mat(_blendImage, rect));
	}
	else
	{
		qDebug() << "Whole Change blend";
		cv::addWeighted(m1, ratio1, m2, ratio2, 0, _blendImage);
	}
	//cv::cvtColor(_blendImage, _blendImage, CV_BGR2RGB);
	QImage rvt = ImageConversion::cvMat_to_QImage(_blendImage, false, false);
	return rvt;
}

void Surface::setBlendAlpha(double source, double reference)
{
	blendAlphaSource = source;
	blendAlphaReference = reference;
}

void Surface::setMyPenRadius(int val)
{
	_myPenRadius = val;
	_myPenRadius = qMax(0, _myPenRadius);
	if (_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
	{
		getCircleInnerPoints(_circleInnerPoint, _myPenRadius);
	}
}

int Surface::getMyPenRadius()
{
	return _myPenRadius;
}

void Surface::pushToSavedPixels(QColor& color, Point& Pt)
{
	std::get<0>(_savedPixels).push_back(color);
	std::get<1>(_savedPixels).push_back(Pt);
}
QColor Surface::getSavedPixelColor(int idx)
{
	return std::get<0>(_savedPixels)[idx];
}
Point Surface::getSavedPixelPos(int idx)
{
	return std::get<1>(_savedPixels)[idx];
}

void Surface::flipColor(QImage& IMG, vector<Point>& vecPts)
{
	Mat drawIMG = ImageConversion::QImage_to_cvMat(IMG, false);
	for (size_t i = 0; i < vecPts.size(); i++)
	{
		Point pt = (vecPts)[i];
		Vec3b color = drawIMG.at<cv::Vec3b>(pt.y, pt.x);
		drawIMG.at<cv::Vec3b>(pt.y, pt.x) = Vec3b(255, 255, 255) - color;
	}
	qDebug() << "flipColor";
}

void Surface::clearSavedPixels()
{
	std::get<0>(_savedPixels).clear();
	std::get<1>(_savedPixels).clear();
}


void Surface::slotPixelCovered(vector<PtrSegmentPoints>* vecPts)
{
	//flip color
	/*_tempVecPoint.assign(vecPts.begin(), vecPts.end());
	flipColor(_ImageDraw, _tempVecPoint);
	_bColorFlipped = true;*/
	//clearSavedPixels();
//	if (_bColorFlipped == false)
	if (!_bLButtonDown)
	{
		_tempVecSegs.clear();
	}
	_tempVecPoint.clear();

	/*draw current points*/
	//int cur_size = _tempVecSegs.size();	
	int cur_size = 0;
	for (int i = 0; i < vecPts->size(); i++)
	{
		if (std::find(_tempVecSegs.begin(), _tempVecSegs.end(), (*vecPts)[i]) == _tempVecSegs.end())
			_tempVecSegs.push_back((*vecPts)[i]);
		else
		{
			qDebug() << "Have Same Segs";
		}
	}

	
		std::vector<Point>& refSavePixels = std::get<1>(_savedPixels);
		Mat& drawIMG = ImageConversion::QImage_to_cvMat(_ImageDraw, false);
		for (size_t i = cur_size; i < _tempVecSegs.size(); i++)
		{
			PtrSegmentPoints pSegs = (_tempVecSegs)[i];
			for (int j = 0; j < pSegs->size(); j++)
			{
				Point p = (*pSegs)[j];
				p *= _scaleRatio;
				if (p.x >= 0 && p.y >= 0 && p.x < drawIMG.cols&&p.y < drawIMG.rows)
				{
					_tempVecPoint.push_back(p);
				}
			}
		}
		cv::Rect& r = _savedBoundingRect;
		/*if (r.x == 0) qDebug() << "r to the left ";
		if (r.y == 0) qDebug() << "r to the top ";
		if (r.x +r.width== drawIMG.cols) qDebug() << "r to the right";
		if (r.y + r.height == drawIMG.rows) qDebug() << "r to the bottom";
		if (r.x < 0) qDebug() << "r out of left ";
		if (r.y < 0) qDebug() << "r out of top ";
		if (r.x + r.width > drawIMG.cols) qDebug() << "r out of right";
		if (r.y + r.height > drawIMG.rows) qDebug() << "r out of bottom";*/
		r = cv::boundingRect(_tempVecPoint);
		Mat(drawIMG, r).copyTo(_drawClipMat);
		//_drawClipMat.create(r.height, r.width, CV_8UC3); _drawClipMat.setTo(0);
		for (size_t i = 0; i < _tempVecPoint.size(); i++)
		{
			Point pt = _tempVecPoint[i] - r.tl();

			_drawClipMat.at<cv::Vec3b>(pt.y, pt.x) = Mat(drawIMG, r).at<cv::Vec3b>(pt.y, pt.x)*0.6
				+ cv::Vec3b(_myPenColor.red(), _myPenColor.green(), _myPenColor.blue())*0.4;
			/*cv::rectangle(temp, cv::Rect(pt - Point(_scaleRatio + 0.5, _scaleRatio + 0.5), pt + Point(_scaleRatio + 0.5, _scaleRatio + 0.5))
			, cv::Scalar(_myPenColor.red(), _myPenColor.green(), _myPenColor.blue()), -1);*/
		}
		this->update(r.x, r.y, r.width, r.height);
		qDebug() << "slotPixelCovered";
	
	
	if (_bLButtonDown&&_drawType == DRAW_TYPE::SUPER_PIXEL_WISE)
	{
		emit signalDrawPixelsToResult(&_tempVecSegs, _myPenColor);
	}
	
}

void Surface::restoreSavedPixels(QImage&IMG, SavedPixels& savedPixels)
{
	assert(std::get<0>(savedPixels).size() == std::get<1>(savedPixels).size());
	int size = std::get<0>(savedPixels).size();
	Mat drawIMG = ImageConversion::QImage_to_cvMat(IMG, false);
#pragma omp parallel for
	for (int i = 0; i < size; i++)
	{
		QColor clr = std::get<0>(savedPixels)[i];
		Point pt = std::get<1>(savedPixels)[i];
		drawIMG.at<cv::Vec3b>(pt.y, pt.x)[0] = clr.red();
		drawIMG.at<cv::Vec3b>(pt.y, pt.x)[1] = clr.green();
		drawIMG.at<cv::Vec3b>(pt.y, pt.x)[2] = clr.blue();
	}
}

void Surface::restoreSavedBoundingBoxImage(QImage&IMG, Mat&savedMat, cv::Rect&savedRect)
{
	Mat drawIMG = ImageConversion::QImage_to_cvMat(IMG, false);
	savedMat.copyTo(Mat(drawIMG, savedRect));
}

void Surface::setVecPointsWithinRadiusOfPoint(vector<Point>&vecPts, vector<Point>&circleInnerPoint, Point center, int width, int height)
{
	vecPts.clear();
	//center.x = center.x;
	//center.y = center.y;
	Point tempPt;
	cv::Rect rect(0, 0, width, height);
//#pragma omp parallel for
	for (int i = 0; i < circleInnerPoint.size(); i++)
	{
		tempPt = circleInnerPoint[i] + center;
		if (rect.contains(tempPt))
		{
			vecPts.push_back(tempPt);
		}
	}
}

void Surface::getCircleInnerPoints(vector<Point>&circleInnerPoint, int radius)
{
	circleInnerPoint.clear();
	radius /= _scaleRatio;
	int radius_square = radius*radius;
	int i_square = 0;
	for (int i = 0; i < radius + 1; i++)//rows
	{
		i_square = i*i;
		for (int j = 0; j < radius + 1; j++)//cols
		{
			if (i*i + j*j <= radius_square)
			{
				circleInnerPoint.push_back(Point(j, i));
				if (i != 0)
				{
					circleInnerPoint.push_back(Point(j, -i));
				}
				if (j != 0)
				{
					circleInnerPoint.push_back(Point(-j, i));
				}
				if (i != 0 && j != 0)
				{
					circleInnerPoint.push_back(Point(-j, -i));
				}
			}
		}
	}
	qDebug() << "getCircleInnerPoints()";
}

#include "Surface.h"
#include <qdebug.h>
#include "ImageConversion.h"
#include <QPainter>
#include <QRect>
#include <qmessagebox.h>
//#define CHECK_QIMAGE

Surface::Surface(QImage &Img, QWidget*parent) :QLabel(parent)
{
	//labelImg() = Mat(400, 400, CV_8UC3);//TODO labelImg should be replace
	//_ImageDraw = ImageConversion::cvMat_to_QImage(labelImg());
	_oriImage = &Img;
	_ImageDraw = Img.copy();
	this->setPixmap(QPixmap::fromImage(_ImageDraw));
	this->setMouseTracking(true);
	LButtonDown = false;
	myPenColor.setRgb(0, 0, 0);
	myPenRadius = 1;
	bSelectClass = false;
	setCursorInvisible(true);
	bDrawCursor = true;
	_scaleRatioRank = 0;
	_scaleRatio = 1.0;
}


Surface::~Surface()
{
#ifdef CHECK_QIMAGE
	cv::destroyAllWindows();
#endif
}

void Surface::setReferenceImage(QImage* pReference)
{
	_referenceImage = pReference;
}

void Surface::changeClass(QString txt, QColor clr)
{
	qDebug() << "Surface::changeClass to: \n" << txt << clr << endl;
	myPenColor = clr;
}

void Surface::showNormal()
{
	this->setScaleRatio(1.0);
	this->applyScaleRatio();
	update();
}

void Surface::showScaled()
{
	this->updateScaleRatioByRank();
	this->applyScaleRatio();
	update();
}

void Surface::showScaledRefImg(QImage* Img)
{
	if (_scaleRatio != 1.0)
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
		showScaledRefImg(_referenceImage);
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

void Surface::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		emit openClassSelection();
		qDebug() << "emit openClassSelection()" << endl;
		break;
	case Qt::Key::Key_V:
		//qDebug() << "showNormal()";
		this->showNormal(); break;
	case Qt::Key::Key_Tab:
		//qDebug() << "showReferenceImg()";
		this->showReferenceImg(); break;
	//case Qt::Key::Key_Space:qDebug() << "Key_Space" << endl; break;
	default:break;
	}
}
void Surface::keyReleaseEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		emit closeClassSelection();
		qDebug() << "emit closeClassSelection()" << endl;
		break;
	case Qt::Key::Key_V:
		this->showScaled(); break;
	case Qt::Key::Key_Tab:
		this->showInternalImg(); break;
		//case Qt::Key::Key_Space:qDebug() << "Key_Space" << endl; break;
	default:break;
	}
}


void Surface::mousePressEvent(QMouseEvent *ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton: qDebug() << "Qt::LeftButton Press" << endl; 
		LButtonDown = true; lastPoint = ev->pos(); paintPath = QPainterPath(); tempDrawPath = QPainterPath(); break;
	case Qt::RightButton: qDebug() << "Qt::RightButton Press" << endl; 

		break;
	case Qt::MidButton://qDebug() << "Qt::MidButton Press" << endl; 
		break;
	default://qDebug() << "mousePress Default case." << endl; 
		break;
	}
}

void Surface::paintEvent(QPaintEvent *ev)
{
	QPainter painter(this);
	/* draw background image */
	QRect dirtyRect = ev->rect();
	painter.drawImage(dirtyRect, _ImageDraw, dirtyRect);
	/* draw stroke trace */
	if (LButtonDown)
	{
		painter.setPen(QPen(myPenColor, (myPenRadius * 2 + 1), Qt::SolidLine, Qt::RoundCap,
			Qt::RoundJoin));
		painter.drawPath(tempDrawPath);
		qDebug() << "painter.drawPath";
	}
	/*draw cursor*/
	if(bDrawCursor)
		paintCursor();
	//qDebug() << "paintEvent";
}

void Surface::updateRectArea(QRect rect, int rad, bool drawCursor)
{
	bool store = bDrawCursor;
	bDrawCursor = drawCursor;
	update(rect.adjusted(-rad, -rad, +rad, +rad).adjusted(-3, -3, 3, 3));
	bDrawCursor = store;
}

QPoint Surface::FilterScalePoint(QPoint pt)
{
	QPoint rtv;
	rtv.setX(pt.x() / _scaleRatio);
	rtv.setY(pt.y() / _scaleRatio);
	return rtv;
}

void Surface::drawLineTo(const QPoint &endPoint)
{	
	/*QPainter painter(&_ImageDraw);
	painter.setPen(QPen(myPenColor, myPenRadius, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));*/
	paintPath.moveTo(FilterScalePoint(lastPoint));
	paintPath.lineTo(FilterScalePoint(endPoint));
	tempDrawPath.moveTo(lastPoint);
	tempDrawPath.lineTo(endPoint);
	//painter.drawLine(lastPoint, endPoint);

	//int rad = (myPenRadius / 2) + 2;
	//update(QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));
	
}

void Surface::paintCursor()
{
	QPainter painter(this);
	painter.setPen(QPen(myPenColor, _cursorEdgeWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));
	painter.setBrush(QBrush(myPenColor, Qt::DiagCrossPattern));
	painter.drawEllipse(_mousePos, myPenRadius, myPenRadius);
}

void Surface::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug() << '(' << ev->x() << ':' << ev->y() << ')' << endl;
	if (LButtonDown)
	{
		qDebug() <<"Draw: " <<'(' << ev->x() << ':' << ev->y() << ')' << endl;
		QPoint endPoint = ev->pos();
		drawLineTo(endPoint);
		updateRectArea(QRect(lastPoint, endPoint).normalized(), myPenRadius, false);
		lastPoint = endPoint;
	}
	updateCursorArea(false);
	_mousePos = ev->pos();
	updateCursorArea(true);
}

void Surface::updateImage()
{
	_ImageDraw = _oriImage->copy();
	applyScaleRatio();
}

void Surface::updateCursorArea(bool drawCursor)
{
	bool beforeCursor = bDrawCursor;
	bDrawCursor = drawCursor;
	this->update(QRect(_mousePos.x() - (myPenRadius + _cursorEdgeWidth), _mousePos.y() - (myPenRadius + _cursorEdgeWidth), \
		(myPenRadius + _cursorEdgeWidth) * 2 + 3, (myPenRadius + _cursorEdgeWidth) * 2 + 3));
	bDrawCursor = beforeCursor;
}

void Surface::mouseReleaseEvent(QMouseEvent *ev)
{
	if (LButtonDown)
	{
		LButtonDown = false;
		emit painterPathCreated((myPenRadius * 2 + 1) / _scaleRatio, paintPath);
		updateRectArea(tempDrawPath.boundingRect().toRect(), myPenRadius, false);
		paintPath = QPainterPath();
		tempDrawPath = QPainterPath();
	}
#ifdef CHECK_QIMAGE
	Mat temp = ImageConversion::QImage_to_cvMat(_ImageDraw,false);
	cv::imshow("QImage", temp);
	cv::waitKey(1);
#endif
}

void Surface::mouseDoubleClickEvent(QMouseEvent* ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton:
		qDebug()<<"Left Double Clicked"; break;
	case Qt::RightButton:
		qDebug() << "Right Double Clicked"; break;
	default:
		break;
	}
}

void Surface::wheelEvent(QWheelEvent*ev)
{
	QPoint numDegrees = ev->angleDelta() / 8;
	QPoint numSteps = numDegrees / 15;
	if (ev->modifiers() == Qt::KeyboardModifiers::enum_type::ControlModifier)
	{
		updateCursorArea(false);
		myPenRadius += numSteps.y()*2;
		myPenRadius = qMax(0, myPenRadius);
		updateCursorArea(true);
		qDebug() << "myPenRadius" << myPenRadius << endl;
	}
	else
	{
		double oldScaleRatio = getScaleRatio();
		setScaleRatioRank(getScaleRatioRank() + numSteps.y());
		updateScaleRatioByRank();
		double newScaleRatio = getScaleRatio();
		QPoint newPos = getPointAfterNewScale(ev->pos(), oldScaleRatio, newScaleRatio);
		emit mousePositionShifted(newPos - ev->pos());
		applyScaleRatio();
		update();
		qDebug() << "numSteps:" << numSteps << endl;
	}
	ev->accept();
}


void Surface::leaveEvent(QEvent*ev)
{
	bDrawCursor = false;
	updateCursorArea(false);
	qDebug() << "leaveEvent";
	QLabel::leaveEvent(ev);
}

void Surface::enterEvent(QEvent*ev)
{
	bDrawCursor = true;
	QLabel::enterEvent(ev);
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
		_scaleRatio = qMin(maximum, _scaleRatio);
		_scaleRatio = qMax(minimum, _scaleRatio);
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
	qDebug() << "_scaleRatioRank:" << _scaleRatioRank;
}

int Surface::getScaleRatioRank()
{
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
	if (_scaleRatio != 1.0)
	{
		int width = _oriImage->width()*_scaleRatio;
		int height = _oriImage->height()*_scaleRatio;
		width = width < 1 ? 1 : width;
		height = height < 1 ? 1 : height;
		_ImageDraw = _oriImage->scaled(QSize(width, height), Qt::AspectRatioMode::KeepAspectRatioByExpanding);
	}
	else
		_ImageDraw = _oriImage->copy();
	
}
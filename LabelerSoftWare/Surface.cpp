#include "Surface.h"
#include <qdebug.h>
#include "ImageConversion.h"
#include <QPainter>

//#define CHECK_QIMAGE

Surface::Surface(QImage &Img, QWidget*parent) :QLabel(parent)
{
	//labelImg() = Mat(400, 400, CV_8UC3);//TODO labelImg should be replace
	//_pImage = ImageConversion::cvMat_to_QImage(labelImg());
	_pImage = Img;
	this->setPixmap(QPixmap::fromImage(_pImage));
	this->setMouseTracking(true);
	LButtonDown = false;
	myPenColor.setRgb(0, 0, 0);
	myPenWidth = 1;
	bSelectClass = false;
}


Surface::~Surface()
{
#ifdef CHECK_QIMAGE
	cv::destroyAllWindows();
#endif
}

void Surface::changeClass(QString txt, QColor clr)
{
	qDebug() << "Surface::changeClass to: \n" << txt << clr << endl;
	myPenColor = clr;
}

void Surface::keyPressEvent(QKeyEvent *ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		emit openClassSelection();
		qDebug() << "emit openClassSelection()" << endl;
		break;
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
		//case Qt::Key::Key_Space:qDebug() << "Key_Space" << endl; break;
	default:break;
	}
}


void Surface::mousePressEvent(QMouseEvent *ev)
{
	switch (ev->buttons())
	{
	case Qt::LeftButton: //qDebug() << "Qt::LeftButton Press" << endl; 
		LButtonDown = true; lastPoint = ev->pos(); paintPath = QPainterPath(); break;
	case Qt::RightButton: //qDebug() << "Qt::RightButton Press" << endl; 
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
	QRect dirtyRect = ev->rect();
	painter.drawImage(dirtyRect, _pImage, dirtyRect);
	if (LButtonDown)
	{
		painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
			Qt::RoundJoin));
		painter.drawPath(paintPath);
		qDebug() << "painter.drawPath";
	}
}

void Surface::drawLineTo(const QPoint &endPoint)
{	
	/*QPainter painter(&_pImage);
	painter.setPen(QPen(myPenColor, myPenWidth, Qt::SolidLine, Qt::RoundCap,
		Qt::RoundJoin));*/
	paintPath.moveTo(lastPoint);
	paintPath.lineTo(endPoint);
	//painter.drawLine(lastPoint, endPoint);

	//int rad = (myPenWidth / 2) + 2;
	//update(QRect(lastPoint, endPoint).normalized().adjusted(-rad, -rad, +rad, +rad));
	update(paintPath.boundingRect().toRect().adjusted(-1, -1, 1, 1));
	lastPoint = endPoint;
}

void Surface::mouseMoveEvent(QMouseEvent *ev)
{
	//qDebug() << '(' << ev->x() << ':' << ev->y() << ')' << endl;
	if (LButtonDown)
	{
		qDebug() <<"Draw: " <<'(' << ev->x() << ':' << ev->y() << ')' << endl;
		drawLineTo(ev->pos());
	}
}
void Surface::mouseReleaseEvent(QMouseEvent *ev)
{
	if (LButtonDown)
	{
		LButtonDown = false;
		emit painterPathCreated(myPenWidth, paintPath);
		update(paintPath.boundingRect().toRect().adjusted(-1, -1, 1, 1));
		paintPath = QPainterPath();
	}
#ifdef CHECK_QIMAGE
	Mat temp = ImageConversion::QImage_to_cvMat(_pImage,false);
	cv::imshow("QImage", temp);
	cv::waitKey(1);
#endif
}
void Surface::wheelEvent(QWheelEvent*ev)
{
	QPoint numDegrees = ev->angleDelta() / 8;
	QPoint numSteps = numDegrees / 15;
	if (ev->modifiers() == Qt::KeyboardModifiers::enum_type::ControlModifier)
	{
		qDebug() << "ControlModifier-" << "numSteps:" << numSteps << endl;
		
	}
	else
	{
		qDebug() << "numSteps:" << numSteps << endl;
	}
	ev->accept();
}

void Surface::setCursor()
{

}


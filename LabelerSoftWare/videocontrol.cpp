#include "videocontrol.h"
#include <QDebug>
#include <QReadWriteLock>
using namespace cv;
VideoControl::VideoControl():_lock(QReadWriteLock::Recursive)
{
	_skipFrameNum = 1;
	_savedSkipFrameNum = 1;
	_frameIdx = -1;
}

VideoControl::~VideoControl()
{

}

unsigned int VideoControl::getSkipFrameNum()
{
	return _skipFrameNum;
}

void VideoControl::setSkipFrameNum(unsigned int num)
{
	if (num >= 2)
		_skipFrameNum = num;
	else
		_skipFrameNum = 1;
}

void VideoControl::setPosFrames(int idx)
{
	_frameIdx = idx - 1;
	_videoCap.set(CAP_PROP_POS_FRAMES, idx);
}

bool VideoControl::open(QString filePath)
{
    QReadLocker locker(&_lock);
    if(_videoCap.isOpened()) _videoCap.release();
    _videoCap.open(filePath.toStdString());
    qDebug()<<"Video openned Successful."<<_videoCap.isOpened()<<endl;
    if(_videoCap.isOpened())
    {
        //locker.unlock();
        retrievePalyInfos();
		_frameIdx = -1;
        return true;
    }
    else return false;
}

void VideoControl::release()
{
    //if(_videoCap.isOpened()) _videoCap.release();
}

bool VideoControl::isOpenned()
{
    QReadLocker locker(&_lock);
    return _videoCap.isOpened();
}

QVector<QString> VideoControl::getAllInfos()
{
    QReadLocker locker(&_lock);
    QVector<QString> vecHints(10);
    if(_videoCap.isOpened())
    {
        vecHints[0]=(QString("Width: %0").arg(_videoCap.get(CAP_PROP_FRAME_WIDTH)));
        vecHints[1]=(QString("Height: %0").arg(_videoCap.get(CAP_PROP_FRAME_HEIGHT)));
        vecHints[2]=(QString("Total Frame Count: %0").arg(_videoCap.get(CAP_PROP_FRAME_COUNT)));
        vecHints[3]=(QString("FPS: %0").arg(_videoCap.get(CAP_PROP_FPS )));
		int ex = static_cast<int>(_videoCap.get(CV_CAP_PROP_FOURCC));
		// Transform from int to char via Bitwise operators
		char EXT[] = { (char)(ex & 0XFF),(char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24),0 };
        vecHints[4]=(QString("Video Code: %0").arg(QString(EXT)));
        vecHints[5]=(QString("Mat Format: %0").arg(_videoCap.get(CAP_PROP_FORMAT )));
        vecHints[6]=(QString("Current Time: %0").arg(_videoCap.get(CAP_PROP_POS_MSEC)));
        vecHints[7]=(QString("Current Frame(1 based): %0").arg(_videoCap.get(CAP_PROP_POS_FRAMES )));
        //vecHints[8]=(QString("Current Frame Ratio %0").arg(_videoCap.get(CAP_PROP_POS_AVI_RATIO )));
        vecHints[8]=(QString("Current Frame Ratio: %0").arg(_videoCap.get(CAP_PROP_POS_FRAMES)/(_videoCap.get(CAP_PROP_FRAME_COUNT)-1)));
        vecHints[9]=(QString("Current Mode: %0").arg(_videoCap.get(CAP_PROP_MODE )));
    }
    return vecHints;
}

void VideoControl::retrievePalyInfos()
{
    QReadLocker locker(&_lock);
    if(_videoCap.isOpened())
    {
        props._width=_videoCap.get(CAP_PROP_FRAME_WIDTH);
        props._height=_videoCap.get(CAP_PROP_FRAME_HEIGHT);
        props._fps=_videoCap.get(CAP_PROP_FPS );
    }
    else
    {
        props._width = props._height = props._fps=0;
    }

}

void VideoControl::reset(int frameNumber)
{
    QWriteLocker locker(&_lock);
	bool success = _videoCap.set(CAP_PROP_POS_FRAMES, frameNumber);
	_frameIdx = frameNumber - 1;
}

bool VideoControl::getFrame(Mat& img)
{
    QWriteLocker locker(&_lock);
	if (_videoCap.read(img))
	{
		_curMat = img;
		locker.unlock();
		increaseFrameIdxBy1();
		return true;
	}
	else
	{
		return false;
	}
}

bool VideoControl::getFrame(Mat& img,double frameNum)
{
    QWriteLocker locker(&_lock);
	_videoCap.set(CAP_PROP_POS_FRAMES, frameNum);
	_frameIdx = frameNum - 1;
	if (_videoCap.read(img))
	{
		_curMat = img;
		locker.unlock();
		increaseFrameIdxBy1();
		return true;
	}
	else
	{
		return false;
	}
}

Mat VideoControl::getCurMat()
{
	return _curMat;
}

double VideoControl::getWidth()
{
    QReadLocker locker(&_lock);
    props._width = _videoCap.get(CAP_PROP_FRAME_WIDTH);
    return props._width;
}
double VideoControl::getHeight()
{
    QReadLocker locker(&_lock);
    props._height = _videoCap.get(CAP_PROP_FRAME_HEIGHT);
    return props._height;
}

double VideoControl::getFps()
{
    QReadLocker locker(&_lock);
    props._fps = _videoCap.get(CAP_PROP_FPS);
    return props._fps;
}
double VideoControl::getFrameCount()
{
    QReadLocker locker(&_lock);
    props._frame_count = _videoCap.get(CAP_PROP_FRAME_COUNT);
    return props._frame_count;
}

double VideoControl::getFourcc()
{
    QReadLocker locker(&_lock);
    props._fourcc = _videoCap.get(CAP_PROP_FOURCC);
    return props._fourcc;
}
double VideoControl::getFormat()
{
    QReadLocker locker(&_lock);
    props._format = _videoCap.get(CAP_PROP_FORMAT);
    return props._format;
}
double VideoControl::getPosMsec()
{
    QReadLocker locker(&_lock);
    props._pos_msec = _videoCap.get(CAP_PROP_POS_MSEC);
    return props._pos_msec;
}
double VideoControl::getPosFrames()
{
    QReadLocker locker(&_lock);
	props._pos_frames = _videoCap.get(CAP_PROP_POS_FRAMES) - 1;
	if (_frameIdx>props._pos_frames)
	{
		props._pos_frames = _frameIdx;
	}
	return props._pos_frames;
}
double VideoControl::getPosAviRatio()
{
    QReadLocker locker(&_lock);
    props._pos_avi_ratio = _videoCap.get(CAP_PROP_POS_AVI_RATIO);
    return props._pos_avi_ratio;
}
double VideoControl::getMode()
{
    QReadLocker locker(&_lock);
    props._mode = _videoCap.get(CAP_PROP_MODE);
    return props._mode;
}

void VideoControl::forwardFrames(int n)
{
	for (size_t i = 0; i < n; i++)
	{
		_videoCap.grab();
	}
}

void VideoControl::setToFrameAndGrab(int idx)
{
	this->setPosFrames(idx);
	_videoCap.grab();
}

void VideoControl::setToNextFrameAndGrab()
{
	int idx = this->getPosFrames() + this->getSkipFrameNum();
	this->setPosFrames(idx);
	_videoCap.grab();
}

void VideoControl::setToPreviousFrameAndGrab()
{
	int idx = this->getPosFrames() - this->getSkipFrameNum();
	this->setPosFrames(idx);
	_videoCap.grab();
}

void VideoControl::saveSkipFrameNum()
{
	_savedSkipFrameNum = _skipFrameNum;
}

void VideoControl::setToMinSkipFrameNum()
{
	this->setSkipFrameNum(1);
}

void VideoControl::setToSavedSkipFrameNum()
{
	this->setSkipFrameNum(_savedSkipFrameNum);
}

void VideoControl::setSavedSkipFrameNum(unsigned int num)
{
	_savedSkipFrameNum = num;
}

void VideoControl::increaseFrameIdxBy1()
{
	QWriteLocker locker(&_lock);
	_frameIdx++;
	locker.unlock();
	int frameCount = (int)getFrameCount();
	locker.relock();
	if (_frameIdx >= frameCount)
	{
		_frameIdx = frameCount - 1;
	}
}
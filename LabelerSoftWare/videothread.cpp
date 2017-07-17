#include "videothread.h"
#include <QMessageBox>
#include <QDebug>
#include <ImageConversion.h>
#include <vector>
//#define IMPROVE_IMG

using namespace cv;
VideoThread::VideoThread(VideoControl* videoCtrl)
{
	_currentState = PLAY_STATE::STOP;
	_videoCtrl = videoCtrl;
	time.start();
	_skipFrameNum = 1;
}

VideoControl* VideoThread::getVideoControl()
{
    return _videoCtrl;
}

bool VideoThread::openVideo(QString filePath)
{
    if(getVideoControl()->open(filePath))
    {
        emitNextImage();
        int height = getVideoControl()->getHeight();
        int width = getVideoControl()->getWidth();
        emit changeFrameSize(width,height);
        _currentState = PLAY_STATE::PAUSE;
        return true;
    }
    else
    {
        if(getVideoControl()->isOpenned()==false)
        {
            QMessageBox::critical(NULL,"Video File Open Error","This file cannot be openned",\
                                  QMessageBox::StandardButton::Cancel);
        }
        return false;
    }
}
void VideoThread::closeVideo()
{
    _currentState=PLAY_STATE::STOP;
    getVideoControl()->release();
}

VideoThread::~VideoThread()
{
    closeVideo();
}

Mat VideoThread::getNextMat()
{
    if (!getVideoControl()->getFrame(_curFrame))
    {
        _currentState = PLAY_STATE::STOP;
        return Mat();
    }
    return _curFrame;
}
QImage VideoThread::convertToQImage(Mat&img)
{
	_img = ImageConversion::cvMat_to_QImage(img);
    return _img;
}

void VideoThread::run()
{
    double frameRate = getVideoControl()->props._fps;
    int delay = (1000.0/frameRate);
    while(_currentState==PLAY_STATE::PLAY){
        int dtime = qMax(0,time.elapsed());
        this->msleep(qMax(0,delay-dtime));
        time.start();

		if (_skipFrameNum >= 2)
		{
			_videoCtrl->forwardFrames(_skipFrameNum - 1);
		}
        emitNextImage();
		emitNextFrameInfo();
    }
}

void VideoThread::pause()
{
    if(_currentState==PLAY_STATE::PLAY)
    {
        _currentState=PLAY_STATE::PAUSE;
		emit changeState((int)PLAY_STATE::PAUSE);
    }
}
void VideoThread::play()
{
    if(_currentState==PLAY_STATE::STOP)
    {
        _currentState=PLAY_STATE::PLAY;
		emit changeState((int)PLAY_STATE::PLAY);
        this->start(LowPriority);
    }
    else if(_currentState==PLAY_STATE::PAUSE)
    {
        _currentState=PLAY_STATE::PLAY;
		emit changeState((int)PLAY_STATE::PLAY);
        this->start(LowPriority);
    }
}
void VideoThread::stop()
{
    if(_currentState!=PLAY_STATE::STOP)
    {
        _currentState=PLAY_STATE::STOP;
		this->wait();
        setNextFrame(0);
        emitNextImage();
		emitNextFrameInfo();
		emit changeState((int)PLAY_STATE::STOP);
    }
}

void VideoThread::setNextFrame(int frameNum)
{
    getVideoControl()->reset(frameNum);
}

void VideoThread::setShowNextFrame(int frameNum)
{
	setNextFrame(frameNum);
	emitNextImage();
}

void VideoThread::imgFilter(Mat& img)
{
#ifdef IMPROVE_IMG
	std::vector<Mat> vecMats;
	cv::split(img, vecMats);
	for (size_t i = 0; i < 3; i++)
	{
		//cv::GaussianBlur(vecMats[i], vecMats[i], cv::Size(3, 3), 1.0);
		cv::medianBlur(vecMats[i], vecMats[i], 7);
	}
	cv::merge(vecMats, img);
#endif
}

void VideoThread::emitNextImage()
{
    Mat img = getNextMat();
    if(!img.empty())
    {
		imgFilter(img);
        _img = convertToQImage(img);
        emit sendImage(_img);
    }
}

void VideoThread::emitNextFrameInfo()
{
	double Msec = getVideoControl()->getPosMsec();
	double posFrame = getVideoControl()->getPosFrames();
	double totalFrame = getVideoControl()->getFrameCount();
	double frameRatio = posFrame / (totalFrame-1);
	if (posFrame == (totalFrame - 1.0))
		pause();
	emit updateVideoInfo(Msec, posFrame, frameRatio);
}

void VideoThread::emitAll()
{
	emitNextImage();
	emitNextFrameInfo();
}

void VideoThread::setSkipFrameNum(unsigned int skipNum)
{
	if (skipNum >= 2)
		_skipFrameNum = skipNum;
	else
		_skipFrameNum = 1;
}

unsigned int VideoThread::getSkipFrameNum()
{
	return _skipFrameNum;
}

QImage& VideoThread::getCurrentImage()
{
	return _img;
}
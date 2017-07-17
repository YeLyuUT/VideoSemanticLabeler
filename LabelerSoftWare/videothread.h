#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H
#include <QThread>
#include <QString>
#include "videocontrol.h"
#include <QImage>
#include <QTime>

class VideoThread : public QThread
{
    Q_OBJECT
public:
    enum PLAY_STATE{PLAY=0,PAUSE,STOP};
public:
    explicit VideoThread(VideoControl* videoCtrl);
    ~VideoThread();
public:
    VideoControl* getVideoControl();
    cv::Mat getNextMat();
    QImage convertToQImage(cv::Mat&img);
	QImage& getCurrentImage();
	void setSkipFrameNum(unsigned int skipNum = 1);
	unsigned int getSkipFrameNum();
protected:
    void run();
public:
	void emitNextImage();
	void emitNextFrameInfo();
	void emitAll();//emitNextImageAndInfos
public slots:
    void pause();
    void play();
    void stop();
    bool openVideo(QString filePath);
    void closeVideo();
	void setNextFrame(int frameNum);
	void setShowNextFrame(int frameNum);
public:
    signals:
     void sendImage(const QImage &image);
     void changeFrameSize(int width,int height);
     void updateVideoInfo(double Msec,double posFrame,double frameRatio);
	 void changeState(int);
private:
	void imgFilter(cv::Mat& img);
private:
    QTime time;//Timing the play time interval
    PLAY_STATE _currentState;
    cv::Mat _curFrame;
    QImage _img;//Current QImage
    VideoControl* _videoCtrl;
	unsigned int _skipFrameNum;
};


#endif // VIDEOTHREAD_H

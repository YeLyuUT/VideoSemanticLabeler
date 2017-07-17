#ifndef LVIDEOWIDGET_H
#define LVIDEOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QScrollArea>
#include <QDockWidget>
#include <QPushButton>
#include "opencv.hpp"
#include "videothread.h"
#include <QVBoxLayout>
#include "clickableprogressbar.h"
#include <QLineEdit>

class LVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LVideoWidget(QWidget *parent = 0);
	virtual ~LVideoWidget();
public:
    bool openVideo(QString fileName);
private:
	bool isOpenned();
	void closeVideo();
	void constructInterface();
	void setupConnections();
	void setLineEditEnabled(bool e);
private:
    QLabel* wFrame;
    QLabel* wStatus;
	QLabel* wTotalFrameNumText;
	QLabel* wSkipFrameNumText;
	QLineEdit* wSkipFrameNumEdit;
	QLineEdit* wCurrentFrameNumEdit;
    ClickableProgressBar* wProgressBar;
    QScrollArea* wScrollArea;
    QDockWidget* wInfoPanel;
    QPushButton* wPlayButton;
    QPushButton* wPauseButton;
    QPushButton* wStopButton;
	QPushButton* wEditButton;
	QPushButton* wCommitButton;
	QPushButton* wSaveButton;
	QHBoxLayout *hBoxLayout0;//layout contain settings

    VideoThread* vthread;
    VideoControl* vcontrol;
    QVector<QLabel*> vecHints;
    QVBoxLayout* MainLayout;
	int _skipFrameNum;
	bool isEditting;
signals:
    void hasOpennedVideo();
    void hasClosedVideo();
	void edittingStarted(QImage&);
	void edittingStopped();
public slots:
    void play();
    void stop();
    void pause();
	void edit();
	void save();
	void commitSetting();
	void hasEditResult(QImage&);
    void showImage(const QImage&img);
    void changeFrameSize(int width,int height);
    void constructInfoPanel();
	void receiveVideoState(int state);

    void deleteInfoPanel();

	void skipFrameNumChanged(const QString& str);
	void currentFrameNumChanged(const QString& str);

	void updateInfos(double Msec, double posFrame, double frameRatio);
	void updateInfoPanel(double Msec, double posFrame, double frameRatio);
	void updateCurrentFrameNum(double num);
	void updateProgressBar(double frameRatio);

    void changeVideoPos(double framePosRatio);
};

#endif // LVIDEOWIDGET_H

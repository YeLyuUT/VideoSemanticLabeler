#ifndef LVIDEOWIDGET_H
#define LVIDEOWIDGET_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QSlider>
#include <QScrollArea>
#include <QDockWidget>
#include <QPushButton>
#include "opencv.hpp"
#include "videothread.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "clickableprogressbar.h"
#include <QLineEdit>
#include <Surface.h>
#include <QEvent>
#include <QObject>
#include <QCheckBox>
#include <QSpinBox>
#include <SmartScrollArea.h>

class LVideoWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LVideoWidget(QWidget *parent = 0);
	virtual ~LVideoWidget();
public:
    bool openVideo(QString fileName);
	VideoControl* getInternalVideoControl();
	void setSkipFrameNum(int num);
	void setSkipFrameNumToLabelingMode();
	void setSkipFrameNumToPlayingMode();
private:
	bool isOpenned();
	void closeVideo();
	void constructInterface();
	void addEventFilters();
	void setupConnections();
	void setLineEditEnabled(bool e);
	void ShrinkWindow();
	void NormalWindow();
	void showFullScreen();
	
protected:
	virtual bool eventFilter(QObject* obj, QEvent* ev);
	void hideEvent(QHideEvent* ev);
	void keyPressEvent(QKeyEvent *ev);
private:
    Surface* wFrame;
    QLabel* wStatus;
	QLabel* wTotalFrameNumText;
	QLabel* wSkipFrameNumText;
	QLineEdit* wSkipFrameNumEdit;
	QLineEdit* wCurrentFrameNumEdit;
    ClickableProgressBar* wProgressBar;
    SmartScrollArea* wScrollArea;
    QDockWidget* wInfoPanel;
    QPushButton* wPlayButton;
    QPushButton* wPauseButton;
    QPushButton* wStopButton;
	QPushButton* wEditButton;
	QPushButton* wCommitButton;
	QPushButton* wSaveButton;
	QPushButton* wOpenSaveDir;
	QSlider* wSliderTransparency;
	QLabel* wSliderText;
	QHBoxLayout *hBoxLayout0;//layout contain settings
	QCheckBox* wCheckBoxAutoLoadResult;
	QPushButton* wButtonUseSPSegs;
	QSpinBox* wSpinBoxSPSLevel;

    VideoThread* vthread;
    VideoControl* vcontrol;
    QVector<QLabel*> vecHints;
    QVBoxLayout* MainLayout;
	QPoint _oldScrollPos;
	int _skipFrameNum;
	bool isEditting;
signals:
    void hasOpennedVideo();
    void hasClosedVideo();
	void edittingStarted(VideoControl* vidCtrl);
	void edittingStopped();
	void signalClose();
	void signalSave();
	void signalAutoLoadResult(bool checked);
	void signalUseSPSegs(int level);
	void signalOpenSaveDir();
	void signalNewFrame();
	void signalTransparencyChanged(int);
public slots:
    void play();
    void stop();
    void pause();
	void edit();
	void save();
	void transparencyValueChanged(int value);
	void toggleAutoLoadResult(bool checked);
	void useSuperpixelEdit();
	void openSaveDir();
	void commitSetting();
	void hasEditResult(QImage&);
    void showImage(const QImage&img);
    void changeFrameSize(int width,int height);
    void constructInfoPanel();
	void receiveVideoState(int state);
	void slotSPSegsLevelChanged(int level);
	void slotSPSSegsLevelChangeOneStep(int diff);

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

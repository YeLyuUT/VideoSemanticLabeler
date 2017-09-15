#include "lvideowidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>
#include "videocontrol.h"
#include <QDebug>
#include <QSizePolicy>
#include <QMessageBox>
#include <QScrollBar>
#include <QApplication>
#include <ImageConversion.h>
#include <labelersoftware.h>

using namespace cv;
LVideoWidget::LVideoWidget(QWidget *parent) : QWidget(parent)
{
    wFrame= nullptr;
    wProgressBar= nullptr;
    wScrollArea= nullptr;
    wInfoPanel= nullptr;
    wPlayButton= nullptr;
    wPauseButton= nullptr;
    wStopButton= nullptr;
    vthread= nullptr;
    vcontrol= nullptr;
    MainLayout = nullptr;
	wTotalFrameNumText = nullptr;
	wCurrentFrameNumEdit = nullptr;
	wEditButton = nullptr;
	wCommitButton = nullptr;
	wSaveButton = nullptr;
	wOpenSaveDir = nullptr;
	wSliderText = nullptr;
	wSliderTransparency = nullptr;
	wCheckBoxAutoLoadResult = nullptr;
    constructInterface();
	vcontrol = new VideoControl();
	vthread = new VideoThread(vcontrol);
	addEventFilters();
	setupConnections();
	_skipFrameNum = 1;
	isEditting = false;
	NormalWindow();
}

LVideoWidget::~LVideoWidget()
{
    closeVideo();
	if (vthread != nullptr)
	{
		delete vthread;
	}
	if (vcontrol != nullptr) delete vcontrol;
    qDebug()<<"Video Widget Deleted."<<endl;
}

void LVideoWidget::ShrinkWindow()
{
	window()->showNormal();
	window()->resize(1, 1);
}

void LVideoWidget::NormalWindow()
{
	window()->showNormal();
}

void LVideoWidget::showFullScreen()
{
	window()->showFullScreen();
}

void LVideoWidget::hideEvent(QHideEvent* ev)
{
	qDebug() << "hideEvent";
	emit signalClose();
	QWidget::hideEvent(ev);
}

void LVideoWidget::setupConnections()
{
	connect(vthread, SIGNAL(sendImage(QImage)), this, SLOT(showImage(QImage)));
	connect(vthread, SIGNAL(changeFrameSize(int, int)), this, SLOT(changeFrameSize(int, int)));
	connect(vthread, SIGNAL(changeState(int)), this, SLOT(receiveVideoState(int)));
	connect(this, SIGNAL(hasOpennedVideo()), this, SLOT(constructInfoPanel()));
	connect(this, SIGNAL(hasClosedVideo()), this, SLOT(deleteInfoPanel()));
	connect(wProgressBar, SIGNAL(sendPosRatio(double)), this, SLOT(changeVideoPos(double)));
	connect(wSliderTransparency, SIGNAL(valueChanged(int)), this, SLOT(transparencyValueChanged(int)));
	//connect(wSkipFrameNumEdit, SIGNAL(textEdited(const QString&)), this, SLOT(skipFrameNumChanged(const QString&)));
	//connect(wCurrentFrameNumEdit, SIGNAL(textEdited(const QString&)), this, SLOT(currentFrameNumChanged(const QString&)));

	connect(wPlayButton, SIGNAL(clicked()), this, SLOT(play()));
	connect(wPauseButton, SIGNAL(clicked()), this, SLOT(pause()));
	connect(wStopButton, SIGNAL(clicked()), this, SLOT(stop()));
	connect(wEditButton, SIGNAL(clicked()), this, SLOT(edit()));
	connect(wSaveButton, SIGNAL(clicked()), this, SLOT(save()));
	connect(wCheckBoxAutoLoadResult, SIGNAL(toggled(bool)), this, SLOT(toggleAutoLoadResult(bool)));
	connect(wOpenSaveDir, SIGNAL(clicked()), this, SLOT(openSaveDir()));
	connect(wCommitButton, SIGNAL(clicked()), this, SLOT(commitSetting()));
	connect(vthread, SIGNAL(updateVideoInfo(double, double, double)), this, SLOT(updateInfos(double, double, double)));
	connect(wFrame, SIGNAL(mousePositionShiftedByScale(QPoint, double, double)), wScrollArea, SLOT(gentleShiftScrollAreaWhenScaled(QPoint, double, double)));
	
}

void LVideoWidget::addEventFilters()
{

}

void LVideoWidget::constructInterface()
{
    MainLayout = new QVBoxLayout();
    this->setLayout(MainLayout);
	/*1st level create canvas*/
    wScrollArea = new SmartScrollArea();
    wScrollArea->setBackgroundRole(QPalette::Dark);

	wFrame = new Surface(QImage(), this);
    wScrollArea->setWidget(wFrame);
    MainLayout->addWidget(wScrollArea);

	/*2nd level create status*/
	QWidget* tW0 = new QWidget(this);
	hBoxLayout0 = new QHBoxLayout();
	tW0->setLayout(hBoxLayout0);
    wStatus = new QLabel(this);
    wStatus->setText(QString("Stopped"));
    wStatus->setStyleSheet("QLabel {  color : red; }");
	hBoxLayout0->addWidget(wStatus);
	wEditButton = new QPushButton("Start Labeling",this);
	hBoxLayout0->addWidget(wEditButton);
	wSaveButton = new QPushButton("Save Result",this);
	hBoxLayout0->addWidget(wSaveButton);
	wSaveButton->hide();
	wOpenSaveDir = new QPushButton("Open Saving Directory");
	hBoxLayout0->addWidget(wOpenSaveDir);
	wOpenSaveDir->hide();
	wCheckBoxAutoLoadResult = new QCheckBox("Auto Load Result");
	hBoxLayout0->addWidget(wCheckBoxAutoLoadResult);
	wCheckBoxAutoLoadResult->hide();
	wCheckBoxAutoLoadResult->setChecked(false);//keep same as process control
	wSkipFrameNumEdit = new QLineEdit(this);
	wSkipFrameNumEdit->setMaximumWidth(50);
	wSkipFrameNumEdit->setText("1");
	hBoxLayout0->addWidget(wSkipFrameNumEdit);
	wSkipFrameNumText = new QLabel(this);
	wSkipFrameNumText->setText("<--SkipFrameNumber");
	hBoxLayout0->addWidget(wSkipFrameNumText);

	wCurrentFrameNumEdit = new QLineEdit(this);
	wCurrentFrameNumEdit->setText("0");
	wCurrentFrameNumEdit->setMaximumWidth(50);
	hBoxLayout0->addWidget(wCurrentFrameNumEdit);
	wTotalFrameNumText = new QLabel(this);
	wTotalFrameNumText->setText("/0 Max 0 based Index");
	hBoxLayout0->addWidget(wTotalFrameNumText);
	wCommitButton = new QPushButton("Commit Settings", this);
	hBoxLayout0->addWidget(wCommitButton);

	setLineEditEnabled(true);
    MainLayout->addWidget(tW0);

	QHBoxLayout* hLayoutTransparencySetting = new QHBoxLayout();
	wSliderText = new QLabel("Transparency Setting");
	wSliderTransparency = new QSlider(Qt::Orientation::Horizontal,this);
	wSliderTransparency->setMinimum(0);
	wSliderTransparency->setMaximum(100);
	wSliderTransparency->setValue(100);
	hLayoutTransparencySetting->addWidget(wSliderText);
	hLayoutTransparencySetting->addWidget(wSliderTransparency);
	wSliderText->hide();
	wSliderTransparency->hide();
	MainLayout->addLayout(hLayoutTransparencySetting);
    /*3rd level create progress bar*/
    wProgressBar = new ClickableProgressBar(this);
    wProgressBar->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);  // 对齐方式

    MainLayout->addWidget(wProgressBar);

    /*4th level create button*/
    QWidget* tW = new QWidget(this);
    QHBoxLayout* hBoxLayout = new QHBoxLayout();
    wPlayButton = new QPushButton("Play",this);
    wPauseButton= new QPushButton("Pause",this);
    wStopButton = new QPushButton("Stop",this);
    tW->setLayout(hBoxLayout);
    hBoxLayout->addWidget(wPlayButton);
    hBoxLayout->addWidget(wPauseButton);
    hBoxLayout->addWidget(wStopButton);
    MainLayout->addWidget(tW);
   
}

void LVideoWidget::setLineEditEnabled(bool e)
{
	wCommitButton->setEnabled(e);
	wSkipFrameNumEdit->setEnabled(e);
	wCurrentFrameNumEdit->setEnabled(e);
}

void LVideoWidget::changeVideoPos(double framePosRatio)
{
    int total_count = vcontrol->getFrameCount();
	int framePos = qMin<int>(total_count, qMax<int>(0, (int)(total_count*framePosRatio - 1.0)));
    vthread->setNextFrame(framePos);
	vthread->emitNextImageAndInfos();
}

void LVideoWidget::changeFrameSize(int width,int height)
{
    if(wFrame)
        wFrame->resize(width,height);
}

void LVideoWidget::deleteInfoPanel()
{
    if(wInfoPanel)
    {wInfoPanel->deleteLater();wInfoPanel=nullptr;}

}

void LVideoWidget::updateInfos(double Msec, double posFrame, double frameRatio)
{
	updateProgressBar(frameRatio);
	updateCurrentFrameNum(posFrame);
}

void LVideoWidget::updateInfoPanel(double Msec,double posFrame,double frameRatio)
{
    if(wInfoPanel!=nullptr)
    {
        vecHints[6]->setText(QString("Current Time: %0").arg(Msec));
		vecHints[7]->setText(QString("Current Frame(1 based): %0").arg(posFrame + 1));
        vecHints[8]->setText(QString("Current Frame Ratio: %0").arg(frameRatio));
    }
}

void LVideoWidget::updateProgressBar(double frameRatio)
{
	if (wProgressBar)
	{
		wProgressBar->setValue(frameRatio * 10000);
		double dProgress = frameRatio * 100;
		wProgressBar->setFormat(QString("Current Frame Position:%1%").arg(QString::number(dProgress, 'f', 2)));
	}
}

void LVideoWidget::updateCurrentFrameNum(double num)
{
	if (wCurrentFrameNumEdit)
		wCurrentFrameNumEdit->setText(QString("%0").arg(num));
}

void LVideoWidget::constructInfoPanel()
{
    /*create docker widget*/
    wInfoPanel=new QDockWidget("Video Info",this);
	wInfoPanel->setFloating(true);
    QWidget *mainWidget = new QWidget(wInfoPanel);
    wInfoPanel->setWidget(mainWidget);
    QVBoxLayout* layout = new QVBoxLayout();
    mainWidget->setLayout(layout);
	connect(vthread, SIGNAL(updateVideoInfo(double, double, double)), this, SLOT(updateInfoPanel(double, double, double)));
    
    if(wInfoPanel!=nullptr&&isOpenned())
    {
        QVector<QString> strAllInfos = this->vthread->getVideoControl()->getAllInfos();
        for(int i=0;i<strAllInfos.size();i++)
        {
            vecHints.push_back(new QLabel(strAllInfos[i],wInfoPanel));
        }

        for(int i=0;i<strAllInfos.size();i++)
        {
            layout->addWidget(vecHints[i]);
        }
    }
    //wInfoPanel->show();
	wInfoPanel->hide();
}
bool LVideoWidget::openVideo(QString fileName)
{
    if(this->vthread->openVideo(fileName))
    {
		wTotalFrameNumText->setText(QString("/%0 Max 0 based Index").arg(vcontrol->getFrameCount()-1));
        emit hasOpennedVideo();
        return true;
    }
    else
        return false;
}

void LVideoWidget::receiveVideoState(int state)
{
	qDebug() << "receiveVideoState" << endl;
	switch (state)
	{
	case (int)VideoThread::PLAY:
		wStatus->setText(QString("Playing"));
		wStatus->setStyleSheet("QLabel {  color : green; }");
		setLineEditEnabled(false);
		break;
	case (int)VideoThread::PAUSE:
		wStatus->setText(QString("Paused"));
		wStatus->setStyleSheet("QLabel {  color : orange; }");
		setLineEditEnabled(true);
		break;
	case (int)VideoThread::STOP:
		wStatus->setText(QString("Stopped"));
		wStatus->setStyleSheet("QLabel {  color : red; }");
		setLineEditEnabled(true);
		break;
	default:
		break;
	}
}

bool LVideoWidget::isOpenned()
{
    return this->vthread->getVideoControl()->isOpenned();
}

void LVideoWidget::closeVideo()
{
	vthread->stop();
    this->vthread->closeVideo();
    qDebug()<<"Video Closed."<<endl;
    emit hasClosedVideo();
}

void LVideoWidget::play()
{
	commitSetting();
    vthread->play();
}
void LVideoWidget::stop()
{
    vthread->stop();
}
void LVideoWidget::pause()
{
    vthread->pause();
}

void LVideoWidget::save()
{
	qDebug() << "emit signal save";
	emit signalSave();
}

void LVideoWidget::toggleAutoLoadResult(bool checked)
{
	qDebug() << "emit toggleAutoLoadResult";
	emit signalAutoLoadResult(checked);
}

void LVideoWidget::openSaveDir()
{
	qDebug() << "emit signal OpenSaveDir";
	emit signalOpenSaveDir();

}

void LVideoWidget::edit()
{
	if (!isEditting)
	{
		//stop();

		pause();
		setLineEditEnabled(true);

		wScrollArea->hide();
		//wProgressBar->hide();
		wProgressBar->setClickable(false);
		wPlayButton->hide();
		wPauseButton->hide();
		wStopButton->hide();
		wSaveButton->show();
		wCheckBoxAutoLoadResult->show();
		wSliderText->show();
		wSliderTransparency->show();
		wOpenSaveDir->show();
		wEditButton->setText("Stop Labeling");
		wStatus->setText(QString("Editting..."));
		wStatus->setStyleSheet("QLabel {  color : blue; }");
		//window()->setWindowFlags(window()->windowFlags()& ~Qt::WindowTitleHint);
		/*window()->showNormal();
		window()->resize(1, 1);*/
		setSkipFrameNumToLabelingMode();
		this->ShrinkWindow();
		QWidget* pa = (QWidget*)this->parent();
		LabelerSoftWare* ppa = (LabelerSoftWare*)pa->parent();
		ppa->moveToTopCenter();

		//qDebug() << window()->windowFlags();
		//window()->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
		emit edittingStarted(this->vcontrol);
		isEditting = true;
	}
	else
	{ 
		QWidget* pa = (QWidget*)this->parent();
		LabelerSoftWare* ppa = (LabelerSoftWare*)pa->parent();
		ppa->moveToLastPos();

		wScrollArea->show();
		//wProgressBar->show();
		wProgressBar->setClickable(true);
		wPlayButton->show();
		wPauseButton->show();
		wStopButton->show();
		wSaveButton->hide();
		wCheckBoxAutoLoadResult->hide();
		wSliderText->hide();
		wSliderTransparency->hide();
		wOpenSaveDir->hide();
		wEditButton->setText("Start Labeling");
		wStatus->setText(QString("Stopped"));
		wStatus->setStyleSheet("QLabel {  color : red; }");
		setSkipFrameNumToPlayingMode();
		//window()->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
		this->showImage(ImageConversion::cvMat_to_QImage(vcontrol->getCurMat()));
		emit edittingStopped();
		isEditting = false;
		/*window()->showNormal();
		window()->resize(1, 1);*/
		this->NormalWindow();
	}
}

void LVideoWidget::hasEditResult(QImage& result)
{

}

void LVideoWidget::commitSetting()
{
	skipFrameNumChanged(wSkipFrameNumEdit->text());
	currentFrameNumChanged(wCurrentFrameNumEdit->text());
}

void LVideoWidget::showImage(const QImage&img)
{
    if(!img.isNull())
    {
        //wFrame->setPixmap(QPixmap::fromImage(img));
		wFrame->setOriginalImage(img);
		wFrame->applyScaleRatio();
		wFrame->update();
    }
}

void LVideoWidget::skipFrameNumChanged(const QString& str)
{
	bool ok;
	int num = str.toInt(&ok);
	if (ok&&num > 0)
	{
		_skipFrameNum = num;
	}
	else
	{
		QMessageBox::warning(this, "Invalid Input", "Invalid SkipFrameNum specified.", QMessageBox::StandardButton::Ok);
		wSkipFrameNumEdit->setText(QString("1"));
		_skipFrameNum = 1;
	}
	if (isEditting)
	{
		vcontrol->setSavedSkipFrameNum(_skipFrameNum);
		vcontrol->setSkipFrameNum(_skipFrameNum);
	}
	qDebug() << _skipFrameNum << endl;
}

void LVideoWidget::currentFrameNumChanged(const QString& str)
{
	bool ok;
	int num = str.toInt(&ok);
	if (ok&&num >= 0&&num<=vcontrol->getFrameCount())
	{
		if (!isEditting)
		{
			vthread->setNextFrame(num);
			vthread->emitNextImage();
			double ratio = vcontrol->getPosFrames() / (vcontrol->getFrameCount() - 1.0);
			this->updateProgressBar(ratio);
		}
		else
		{
			vcontrol->setToFrameAndGrab(num);
			emit signalNewFrame();
		}
	}
	else
	{
		QMessageBox::warning(this, "Invalid Input", "Invalid CurrentFrameNum specified.", QMessageBox::StandardButton::Ok);
		wCurrentFrameNumEdit->setText(QString("%0").arg((int)vcontrol->getPosFrames()));
	}
	qDebug() << num << endl;
}

bool LVideoWidget::eventFilter(QObject* obj, QEvent* ev)
{
	//if (qobject_cast<Surface*>(obj) == wFrame)
	//{
	//	//shiftScrollArea(QPoint());
	//	if (ev->type() == QEvent::Type::Wheel)
	//	{
	//		if (((QWheelEvent*)ev)->modifiers() == Qt::KeyboardModifier::AltModifier)
	//		{

	//		}
	//	}
	//}
	//if (isEditting)
	//{
	//	if (keypressevent);
	//}
	return QWidget::eventFilter(obj, ev);
}

void LVideoWidget::keyPressEvent(QKeyEvent *ev)
{
	//if (isEditting)
	//{
	//	switch (ev->type())
	//	{
	//	case Qt::Key::Key_Right:

	//		break;
	//	case Qt::Key::Key_Left:
	//		break;
	//	}
	//}
}

VideoControl* LVideoWidget::getInternalVideoControl()
{
	return this->vcontrol;
}

void LVideoWidget::setSkipFrameNum(int num)
{
	wSkipFrameNumEdit->setText(QString("%1").arg(num));
	_skipFrameNum = num;
	vcontrol->setSkipFrameNum(num);
}

void LVideoWidget::setSkipFrameNumToLabelingMode()
{
	vcontrol->setToSavedSkipFrameNum();
	setSkipFrameNum(vcontrol->getSkipFrameNum());
}

void LVideoWidget::setSkipFrameNumToPlayingMode()
{
	vcontrol->setToMinSkipFrameNum();
	setSkipFrameNum(vcontrol->getSkipFrameNum());
}

void LVideoWidget::transparencyValueChanged(int value)
{
	//qDebug() << "LVideoWidget::transparencyValueChanged" << endl;
	emit signalTransparencyChanged(value);
}
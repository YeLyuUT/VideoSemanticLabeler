#include "ProcessControl.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <exception>
#include <labelersoftware.h>
#include <LabelingTaskControl.h>
#include <ImageConversion.h>
#include <vector>
using std::vector;

ProcessControl::ProcessControl(MetaData& meta, QObject* parent) :QObject(parent)
{
	_type = PROCESS_TYPE_NONE;
	_filePath = meta.filePath;
	_outputDir = meta.outputDir;
	_labelList = meta.labelList;
  _imgExtension = meta.imgExtension;
  _extractBoundary = meta.extractBoundary;
	_selection = new ClassSelection(meta.labelList);
	_w = NULL;
	_labelingTask = NULL;	
	_isLabeling = false;
	_skipFrameNum = meta.skipFrameNum;
	_autoLoadResult = false;//Please keep this the same as in the video widget.
  _superpixel_scales = meta.superpixel_scales;
  //qDebug() << _superpixel_scales.size() << endl;
}


ProcessControl::~ProcessControl()
{
	if (_w) { _w->deleteLater(); _w = NULL; }
	if (_selection) { _selection->deleteLater(); _selection = NULL; }
	if (_labelingTask) 
	{ 

		_labelingTask->deleteLater(); _labelingTask = NULL; 
	}
}

void ProcessControl::process()
{
	if(!checkCreateOutputDir()) throw std::exception("Output Directory is not valid");
	_type = checkForProcessType();
	switch (_type)
	{
	case 0:
		QMessageBox::critical(NULL, "Information", "Processing type cannot be defined.\n", QMessageBox::StandardButton::Ok);
		break;
	case 1:
		processImages();
		break;
	case 2:
		processVideo();
		break;
	default:
		break;
	};
}

int ProcessControl::checkForProcessType()
{
	QFileInfo info(QString(_filePath.c_str()));
	if (!info.exists())
	{
		if (info.fileName().contains('.'))
		{
			throw std::exception("File specified by <InputFilePath> does not exist");
		}
		else
		{
			throw std::exception("Directory specified by <InputFilePath> does not exist");
		}
	}
	else
	{
		if (info.fileName().contains('.'))
		{
			return PROCESS_TYPE_VIDEO;
		}
		else
		{
			return PROCESS_TYPE_IMAGES;
		}

	}
	return PROCESS_TYPE_NONE;
}

bool ProcessControl::checkCreateOutputDir()
{
	QString dirPath = QString(_outputDir.c_str());
	QFileInfo info(dirPath);
	if (info.isDir())
	{
		return true;
	}
	else
	{
		QMessageBox::StandardButton btn = QMessageBox::question(NULL, "Create Directory", "Directory does not exist, create this directory?", \
			QMessageBox::StandardButton::Ok|QMessageBox::StandardButton::Cancel);
		if (btn == QMessageBox::StandardButton::Ok)
		{
			QDir().mkdir(dirPath);
			if (QDir(dirPath).exists())
			{
				return true;
			}
			else
			{
				QMessageBox::critical(NULL, "Error", "Cannot create this directory", \
					QMessageBox::StandardButton::Cancel);
				return false;
			}
		}
		else if (btn == QMessageBox::StandardButton::Cancel)
		{
			return false;
		}
		return false;
	}
}

void ProcessControl::processImages()
{
	qDebug()<<"processImages" << endl;
  _w = new LabelerSoftWare(1, QString(_filePath.c_str()), QString(_outputDir.c_str()), _labelList, _imgExtension, _extractBoundary);
	_w->show();
}

void ProcessControl::processVideo()
{
	qDebug() << "processVideo" << endl;
	_w = new LabelerSoftWare(2, QString(_filePath.c_str()), QString(_outputDir.c_str()), _labelList,_imgExtension,_extractBoundary);
  _w->getVideoWidget()->getInternalVideoControl()->setSavedSkipFrameNum(_skipFrameNum);
	_w->getVideoWidget()->setSkipFrameNum(1);
	QObject::connect(_w->getVideoWidget(), SIGNAL(edittingStarted(VideoControl*)), this, SLOT(hasNewLabelingProcess(VideoControl*)));
	QObject::connect(_w->getVideoWidget(), SIGNAL(edittingStopped()), this, SLOT(closeLabelingProcess()));
	QObject::connect(_w->getVideoWidget(), SIGNAL(signalClose()), this, SLOT(labelerSoftWareQuit()));
	QObject::connect(_w->getVideoWidget(), SIGNAL(signalAutoLoadResult(bool)), this, SLOT(toggleAutoLoadResult(bool)));
	QObject::connect(_w->getVideoWidget(), SIGNAL(signalUseSPSegs(int)), this, SLOT(useSPSegsLabeling(int)));
	_w->show();
	
}

void ProcessControl::hasNewLabelingProcess(VideoControl* pVidCtrl)
{
	if (_labelingTask == NULL)
	{
		double posFrame = pVidCtrl->getPosFrames();
		double ratio = pVidCtrl->getFrameCount() == 1 ? 1.0 : posFrame / (pVidCtrl->getFrameCount() - 1.0);
		_labelingTask = new LabelingTaskControl(this, pVidCtrl, _selection, QString::fromStdString(this->_outputDir), _autoLoadResult, this);
		qDebug() << "LabelingTaskControl Created";
		QObject::connect(_w->getVideoWidget(), SIGNAL(signalSave()), _labelingTask, SLOT(saveLabelResult()));
		QObject::connect(_w->getVideoWidget(), SIGNAL(signalOpenSaveDir()), _labelingTask, SLOT(openSaveDir()));
		QObject::connect(_w->getVideoWidget(), SIGNAL(signalNewFrame()), this, SLOT(updateFrameToBeLabeled()));
		QObject::connect(_w->getVideoWidget(), SIGNAL(signalTransparencyChanged(int)), _labelingTask, SLOT(changeTransparency(int)));
		QObject::connect(_labelingTask, SIGNAL(signalChangeLevelByDiff(int)), _w->getVideoWidget(), SLOT(slotSPSSegsLevelChangeOneStep(int)));
    QObject::connect(_w->getVideoWidget(), SIGNAL(signalCanvasIndexChanged(int)), _labelingTask, SLOT(slotSetCanvasIndex(int)));
		_w->getVideoWidget()->setSkipFrameNum(pVidCtrl->getSkipFrameNum());
		_w->getVideoWidget()->updateProgressBar(ratio);
		_w->getVideoWidget()->updateCurrentFrameNum(posFrame);
		_isLabeling = true;
	}
	
}

void ProcessControl::closeLabelingProcess()
{
	if (_labelingTask != NULL)
	{
		VideoControl* pVidCtrl = _w->getVideoWidget()->getInternalVideoControl();
		pVidCtrl->setToMinSkipFrameNum();
		QObject::disconnect(_w->getVideoWidget(), SIGNAL(signalSave()), _labelingTask, SLOT(saveLabelResult()));
		QObject::disconnect(_w->getVideoWidget(), SIGNAL(signalOpenSaveDir()), _labelingTask, SLOT(openSaveDir()));
		QObject::disconnect(_w->getVideoWidget(), SIGNAL(signalNewFrame()), this, SLOT(updateFrameToBeLabeled()));
		QObject::disconnect(_w->getVideoWidget(), SIGNAL(signalTransparencyChanged(int)), _labelingTask, SLOT(changeTransparency(int)));
    QObject::disconnect(_w->getVideoWidget(), SIGNAL(signalCanvasIndexChanged(int)), _labelingTask, SLOT(slotSetCanvasIndex(int)));
		_labelingTask->deleteLater();
		_labelingTask = NULL;
		qDebug() << "LabelingTaskControl Closed";
		_isLabeling = false;
	}
}

void ProcessControl::switchToNextLabelFrame()
{
	if (_isLabeling)
	{
		closeLabelingProcess();
		VideoControl* pVidCtrl = _w->getVideoWidget()->getInternalVideoControl();
		pVidCtrl->setToSavedSkipFrameNum();
		pVidCtrl->setToNextFrameAndGrab();
		hasNewLabelingProcess(pVidCtrl);
	}
}

void ProcessControl::switchToPreviousLabelFrame()
{
	if (_isLabeling)
	{
		closeLabelingProcess();
		VideoControl* pVidCtrl = _w->getVideoWidget()->getInternalVideoControl();
		pVidCtrl->setToSavedSkipFrameNum();
		pVidCtrl->setToPreviousFrameAndGrab();
		hasNewLabelingProcess(pVidCtrl);
	}
}

void ProcessControl::switchToLabelFrame(int idx)
{
	if (_isLabeling)
	{
		closeLabelingProcess();
		VideoControl* pVidCtrl = _w->getVideoWidget()->getInternalVideoControl();
		pVidCtrl->setPosFrames(idx);
		hasNewLabelingProcess(pVidCtrl);
	}
}

void ProcessControl::updateFrameToBeLabeled()
{
	if (_isLabeling)
	{
		closeLabelingProcess();
		VideoControl* pVidCtrl = _w->getVideoWidget()->getInternalVideoControl();
		pVidCtrl->setToSavedSkipFrameNum();
		hasNewLabelingProcess(pVidCtrl);
	}
}

void ProcessControl::labelerSoftWareQuit()
{
	qDebug() << "LabelerSoftWareQuit";
	this->deleteLater();
	//return true;
}

bool ProcessControl::eventFilter(QObject* obj, QEvent* ev)
{
	if (_isLabeling)
	{
		if (qobject_cast<SmartScrollArea*>(obj) != nullptr)
		{
			if (ev->type() == QEvent::KeyPress)
			{
				QKeyEvent* kevt = (QKeyEvent*)ev;
				switch (kevt->key())
				{
				case Qt::Key::Key_Left:
					switchToPreviousLabelFrame();
					break;
				case Qt::Key::Key_Right:
					switchToNextLabelFrame();
					break;
				default:
					break;
				}
			}
			if (ev->type() == QEvent::Close)
			{
				qDebug() << "SA close event";
				this->_w->getVideoWidget()->edit();
			}
		}
	}
	return QObject::eventFilter(obj,ev);
}

void ProcessControl::toggleAutoLoadResult(bool checked)
{
	_autoLoadResult = checked;
	if (_isLabeling&&_labelingTask)
	{
		_labelingTask->setAutoLoadResult(_autoLoadResult);
	}
}

void ProcessControl::useSPSegsLabeling(int level)
{
	if (_isLabeling&&_labelingTask)
	{
		_labelingTask->setupSegmentationSurface(level);
	}
}
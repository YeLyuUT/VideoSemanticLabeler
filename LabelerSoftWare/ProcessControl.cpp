#include "ProcessControl.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <exception>
#include <labelersoftware.h>

ProcessControl::ProcessControl(string filePath, string outputDir, LabelList& labelList)
{
	_type = PROCESS_TYPE_NONE;
	_filePath = filePath;
	_outputDir = outputDir;
	_labelList = labelList;
}


ProcessControl::~ProcessControl()
{
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
	}
}

void ProcessControl::processImages()
{
	qDebug()<<"processImages" << endl;
	LabelerSoftWare *w = new LabelerSoftWare(1, QString(_filePath.c_str()), QString(_outputDir.c_str()), _labelList);
	w->show();
}

void ProcessControl::processVideo()
{
	qDebug() << "processVideo" << endl;
	LabelerSoftWare *w = new LabelerSoftWare(2, QString(_filePath.c_str()), QString(_outputDir.c_str()), _labelList);
	w->show();
}
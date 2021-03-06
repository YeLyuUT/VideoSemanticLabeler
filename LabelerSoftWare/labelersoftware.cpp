#include "labelersoftware.h"
#include <QLabel>
#include <QPixmap>
#include <lvideowidget.h>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include "videothread.h"
#include <QApplication>
#include <QDesktopWidget>

#include <QString>
#include <QDebug>
#include <opencv.hpp>
LabelerSoftWare::LabelerSoftWare(int processingType, QString inputFilePath, QString outputDir,
  LabelList labelList, string imgExtension, int extractBoundary, QWidget *parent)
	: QMainWindow(parent)
{
  this->setWindowIcon(QIcon(":/icon/icon/L.jpg"));

  _lvw = NULL;
	_type = processingType;
	_filePath = inputFilePath;
	_outputDir = outputDir;
	_labelList = labelList;
  _imgExtension = QString::fromStdString(imgExtension);
  _extractBoundary = extractBoundary;
	//this->setAttribute(Qt::WA_DeleteOnClose);
	ui.setupUi(this);
	mlayout = new QVBoxLayout();
	ui.centralWidget->setLayout(mlayout);
  //About
  connect(ui.actionAboutMe,SIGNAL(triggered()),this,SLOT(About()));
	if (_type == 1)
	{
		createImageProcessWindow();
	}
	else if (_type == 2)
	{
		createVideoProcessWindow();
	}
  
  this->show();
}

LabelerSoftWare::~LabelerSoftWare()
{
}

void LabelerSoftWare::moveToTopCenter()
{
	_lastRectPos = this->pos();
	this->move(QApplication::desktop()->screen()->rect().center().x() - this->window()->width() / 2, 0);
}

void LabelerSoftWare::moveToLastPos()
{
	this->move(_lastRectPos);
}

void LabelerSoftWare::createVideoProcessWindow()
{
  _lvw = new LVideoWidget(_labelList, this);
	mlayout->addWidget(_lvw);
	_lvw->openVideo(_filePath);
}
void LabelerSoftWare::createImageProcessWindow()
{

}

LVideoWidget*& LabelerSoftWare::getVideoWidget()
{
	return _lvw;
}



void LabelerSoftWare::About()
{
  QMessageBox::about(this, tr("About Me"),
            tr("Developer:<b>Ye Lyu</b>.<br> Webpage:<a href = https://yelyuut.github.io/ >Here</a>"));
}
#include "labelersoftware.h"
#include <QLabel>
#include <QPixmap>
#include <lvideowidget.h>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QMessageBox>
#include "videothread.h"

#include <QString>
#include <QDebug>
#include <opencv.hpp>
LabelerSoftWare::LabelerSoftWare(int processingType, QString inputFilePath, QString outputDir, LabelList labelList, QWidget *parent)
	: QMainWindow(parent)
{
	_type = processingType;
	_filePath = inputFilePath;
	_outputDir = outputDir;
	_labelList = labelList;
	this->setAttribute(Qt::WA_DeleteOnClose);
	ui.setupUi(this);
	mlayout = new QVBoxLayout();
	ui.centralWidget->setLayout(mlayout);
	if (_type == 1)
	{
		createImageProcessWindow();
	}
	else if (_type == 2)
	{
		createVideoProcessWindow();
	}
}

LabelerSoftWare::~LabelerSoftWare()
{

}

void LabelerSoftWare::createVideoProcessWindow()
{
	LVideoWidget* lvw = new LVideoWidget(this);
	mlayout->addWidget(lvw);
	lvw->openVideo(_filePath);
}
void LabelerSoftWare::createImageProcessWindow()
{

}
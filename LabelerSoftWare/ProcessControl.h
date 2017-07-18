#pragma once
#include "DataType.h"
#include "ClassSelection.h"
#include <QObject>
#include <LabelingTaskControl.h>
#include <LabelerSoftWare.h>
class ProcessControl:public QObject
{
	Q_OBJECT
private:
	const int PROCESS_TYPE_NONE = 0;
	const int PROCESS_TYPE_IMAGES = 1;
	const int PROCESS_TYPE_VIDEO = 2;
public:
	ProcessControl(string filePath, string outputDir, LabelList& labelList, QObject* parent = NULL);
	virtual ~ProcessControl();
public:
	void process();
	public slots:
	void hasNewLabelingProcess(QImage&Img);
	void closeLabelingProcess();
	void labelerSoftWareQuit();
private:
	bool checkCreateOutputDir();
	int checkForProcessType();
	void processImages();
	void processVideo();
private:
	int _type;
	string _filePath;
	string _outputDir;
	LabelList _labelList;
	ClassSelection *_selection;
	LabelingTaskControl* _labelingTask;
	LabelerSoftWare *w;
};


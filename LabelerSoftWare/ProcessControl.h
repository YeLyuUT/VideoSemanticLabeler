#pragma once
#include "DataType.h"
#include "ClassSelection.h"
#include <QObject>
#include <LabelingTaskControl.h>
#include <LabelerSoftWare.h>
#include <videocontrol.h>
#include <LabelingTaskControl.h>
#include <DataType.h>

#define defGeter(name,type) \
type get##name(){return this->name;}

#define defSeter(name,type) \
void set##name(type& val){this->name = val;}

class LabelingTaskControl;
class ProcessControl:public QObject
{
	Q_OBJECT
private:
	const int PROCESS_TYPE_NONE = 0;
	const int PROCESS_TYPE_IMAGES = 1;
	const int PROCESS_TYPE_VIDEO = 2;
public:
	ProcessControl(MetaData& meta, QObject* parent = NULL);
	virtual ~ProcessControl();
public:
	void process();
public slots:
	void hasNewLabelingProcess(VideoControl* pVidCtrl);
	void closeLabelingProcess();
	void labelerSoftWareQuit();
	void toggleAutoLoadResult(bool checked);
	void useSPSegsLabeling(int level);
	void switchToNextLabelFrame();
	void switchToPreviousLabelFrame();
	void updateFrameToBeLabeled();
	
protected:
	bool eventFilter(QObject* obj, QEvent* ev);
private:
	bool checkCreateOutputDir();
	int checkForProcessType();
	void processImages();
	void processVideo();
	void switchToLabelFrame(int idx);
public:
  
  defGeter(_type, int)
    defGeter(_filePath, string)
    defGeter(_outputDir, string)
    defGeter(_imgExtension, string)
    defGeter(_extractBoundary, int)
    defGeter(_labelList, LabelList)
    defGeter(_selection, ClassSelection*)
    defGeter(_labelingTask, LabelingTaskControl*)
    defGeter(_isLabeling, bool)
    defGeter(_skipFrameNum, int)
    defGeter(_autoLoadResult, bool)

    defSeter(_type, int)
    defSeter(_filePath, string)
    defSeter(_outputDir, string)
    defSeter(_imgExtension, string)
    defSeter(_extractBoundary, int)
    defSeter(_labelList, LabelList)
    defSeter(_selection, ClassSelection*)
    defSeter(_labelingTask, LabelingTaskControl*)
    defSeter(_isLabeling, bool)
    defSeter(_skipFrameNum, int)
    defSeter(_autoLoadResult, bool)

private:
	int _type;
	string _filePath;
	string _outputDir;
  string _imgExtension;
  int _extractBoundary;
	LabelList _labelList;
	ClassSelection *_selection;
	LabelingTaskControl* _labelingTask;
	LabelerSoftWare *_w;
	bool _isLabeling;
	int _skipFrameNum;//used to store parameter from metaData(xml file)
	bool _autoLoadResult;
};


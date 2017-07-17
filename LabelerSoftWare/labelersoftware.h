#ifndef LABELERSOFTWARE_H
#define LABELERSOFTWARE_H

#include <QtWidgets/QMainWindow>
#include "ui_labelersoftware.h"
#include "DataType.h"
#include <QVBoxLayout>
#include <LVideoWidget.h>

class LabelerSoftWare : public QMainWindow
{
	Q_OBJECT
public:
	LabelerSoftWare(int processingType, QString inputFilePath, QString outputDir, LabelList labelList, QWidget *parent = 0);
	virtual ~LabelerSoftWare();

	LVideoWidget* getVideoWidget();
private:
	void createVideoProcessWindow();
	void createImageProcessWindow();
private:
	Ui::LabelerSoftWareClass ui;
private:
	QVBoxLayout* mlayout;
	int _type;
	QString _filePath;
	QString _outputDir;
	LabelList _labelList;
	LVideoWidget* _lvw;
};

#endif // LABELERSOFTWARE_H

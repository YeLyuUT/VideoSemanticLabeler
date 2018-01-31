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
  LabelerSoftWare(int processingType, QString inputFilePath, QString outputDir, LabelList labelList, string imgExtension, int extractBoundary, QWidget *parent = 0);
	virtual ~LabelerSoftWare();

	LVideoWidget*& getVideoWidget();

	void moveToTopCenter();
	void moveToLastPos();
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
  QString _imgExtension;
  int _extractBoundary;
	LabelList _labelList;
	LVideoWidget* _lvw;
	QPoint _lastRectPos;
};

#endif // LABELERSOFTWARE_H

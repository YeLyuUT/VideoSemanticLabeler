#pragma once
#include "qwidget.h"
#include <DataType.h>
#include <QColor>
#include <QVector>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QEvent>
#include <QKeyEvent>
class ClassSelection :
	public QWidget
{
	Q_OBJECT
public:
	explicit ClassSelection(LabelList& ls, QWidget* parent = NULL, int wItemNumber = 4);
	~ClassSelection();
public:
	bool setToItem(int num);
	QString getSelectedItemText();
	bool eventFilter(QObject * watched, QEvent * ev);
	QColor getCurrentColor();
	string getCurrentString();
signals:
	void classChanged(QString text, QColor color);
protected:
	void mousePressEvent(QMouseEvent* ev) Q_DECL_OVERRIDE;
	void keyReleaseEvent(QKeyEvent*ev) Q_DECL_OVERRIDE;
private:
	QRadioButton* createRadioButtonForGroupBox(string text, QColor color);
	void constructPaletteInterface();
private:
	QColor _curColor;
	string _curLabelText;
	LabelList _pLs;
	QGroupBox *_groupBox;
	QButtonGroup *_buttonGroup;
	QGridLayout *_mainLayout;
private:
	int _wItemNumber;//set number of items in each row
};


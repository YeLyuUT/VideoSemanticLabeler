#pragma once
#include <QWidget>
#include <QLabel>
#include <QScrollArea>
#include <QPushButton>
#include "opencv.hpp"
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QLineEdit>

class LImageWidget :public QWidget
{
public:
	explicit LImageWidget(QWidget *parent = 0);
	virtual ~LImageWidget();
public:
	bool openImage(QString filePath);
private:
	
private:
	QTreeWidget* _treeWidget;
	QLineEdit* _searchEdit;
	QPushButton* _searchButton;
	QScrollArea* _scrollArea;
	QLabel* _mainFrame;
	QLabel* _descriptionText;

	QPushButton* _nextButton;
	QPushButton* _prevButton;
	QPushButton* _editButton;
};


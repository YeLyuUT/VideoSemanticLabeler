#include "ClassSelection.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <exception>
#include <QDebug>

ClassSelection::ClassSelection(LabelList& ls, QWidget* parent, int wItemNumber) :QWidget(parent)
{
	_curLabelText = "None";
	_pLs = ls;
	setToItem(0);
	QVBoxLayout *MainLayout = new QVBoxLayout();
	this->setLayout(MainLayout);
	_buttonGroup = new QButtonGroup(this);
	_buttonGroup->setExclusive(true);
	_groupBox = new QGroupBox("Class Selection", this);
	_wItemNumber = wItemNumber;
	_mainLayout = new QGridLayout();
	_mainLayout->setSpacing(0);
	_groupBox->setLayout(_mainLayout);
	MainLayout->addWidget(_groupBox);
	constructPaletteInterface();
	QSizePolicy sp;
	sp.setHorizontalPolicy(QSizePolicy::Policy::Maximum);
	this->setSizePolicy(sp);
}


ClassSelection::~ClassSelection()
{
}

void ClassSelection::constructPaletteInterface()
{
	for (size_t i = 0; i < _pLs.size(); i++)
	{
		if (setToItem(i))
		{
			QRadioButton* pRB = createRadioButtonForGroupBox(_curLabelText, _curColor);
			_mainLayout->addWidget(pRB, i / _wItemNumber, i%_wItemNumber);
			_buttonGroup->addButton(pRB);
			if (i == 0)
			{
				pRB->setChecked(true);
			}
		}	
		else
		{
			throw std::exception("Construct PaletteInterface Failed");
		}
	}
	setToItem(0);
}

QRadioButton* ClassSelection::createRadioButtonForGroupBox(string text, QColor color)
{
	QRadioButton* rvt = new QRadioButton(QString(text.c_str()), this);
	rvt->setMouseTracking(true);
	rvt->installEventFilter(this);
	QString Style =
		QString("color: rgb(255,255,255);"
				"padding: 15px;"
				"background-color: rgb(%0,%1,%2)")\
		.arg(color.red()).arg(color.green()).arg(color.blue());
	rvt->setStyleSheet(Style);
	QSizePolicy sp;
	sp.setHorizontalStretch(1);
	sp.setHorizontalPolicy(QSizePolicy::Policy::Expanding);
	rvt->setSizePolicy(sp);
	return rvt;
}

bool ClassSelection::setToItem(int num)
{
	if (_pLs.size() > num)
	{
		Label& lbl = (_pLs)[num];
		string Text = std::get<0>(lbl);
		int R, G, B;
		R = std::get<1>(lbl);
		G = std::get<2>(lbl);
		B = std::get<3>(lbl);
		_curColor.setRgb(R, G, B);
		_curLabelText = Text;
		return true;
	}
	else
	{
		_curColor.setRgb(0, 0, 0);
		_curLabelText = "None";
		return false;
	}
}

void ClassSelection::mousePressEvent(QMouseEvent* ev)
{
	qDebug() << "mousePressEvent" << endl;
}

void ClassSelection::keyReleaseEvent(QKeyEvent*ev)
{
	switch (ev->key())
	{
	case Qt::Key::Key_Shift:
		this->hide();
		break;
	default:
		break;
	}
}

QString ClassSelection::getSelectedItemText()
{
	QList<QAbstractButton*> list = _buttonGroup->buttons();
	foreach(QAbstractButton *pButton, list)
	{
		if (pButton->isChecked())
			return pButton->text();
	}
}

bool ClassSelection::eventFilter(QObject * watched, QEvent * ev)
{
	QRadioButton* pRB = qobject_cast<QRadioButton*>(watched);
	if (pRB && ev->type() == QEvent::Type::MouseMove)
	{
		if (pRB->isChecked() == false)
		{
			pRB->setChecked(true);
			for (size_t i = 0; i < _pLs.size(); i++)
			{
				if (std::get<0>(_pLs[i]) == pRB->text().toStdString())
				{
					setToItem(i);
					emit classChanged(QString(_curLabelText.c_str()), _curColor);
					//qDebug() << _curLabelText.c_str() << ':' << _curColor;
					return true;
				}
			}
		}
	}
	return QWidget::eventFilter(watched, ev);;
}
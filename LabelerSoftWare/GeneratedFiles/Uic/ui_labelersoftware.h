/********************************************************************************
** Form generated from reading UI file 'labelersoftware.ui'
**
** Created by: Qt User Interface Compiler version 5.6.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LABELERSOFTWARE_H
#define UI_LABELERSOFTWARE_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_LabelerSoftWareClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *LabelerSoftWareClass)
    {
        if (LabelerSoftWareClass->objectName().isEmpty())
            LabelerSoftWareClass->setObjectName(QStringLiteral("LabelerSoftWareClass"));
        LabelerSoftWareClass->resize(600, 400);
        menuBar = new QMenuBar(LabelerSoftWareClass);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        LabelerSoftWareClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(LabelerSoftWareClass);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        LabelerSoftWareClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(LabelerSoftWareClass);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        LabelerSoftWareClass->setCentralWidget(centralWidget);
        /*statusBar = new QStatusBar(LabelerSoftWareClass);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        LabelerSoftWareClass->setStatusBar(statusBar);*/

        retranslateUi(LabelerSoftWareClass);

        QMetaObject::connectSlotsByName(LabelerSoftWareClass);
    } // setupUi

    void retranslateUi(QMainWindow *LabelerSoftWareClass)
    {
        LabelerSoftWareClass->setWindowTitle(QApplication::translate("LabelerSoftWareClass", "LabelerSoftWare", 0));
    } // retranslateUi

};

namespace Ui {
    class LabelerSoftWareClass: public Ui_LabelerSoftWareClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LABELERSOFTWARE_H

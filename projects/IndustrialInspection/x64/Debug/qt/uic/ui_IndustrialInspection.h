/********************************************************************************
** Form generated from reading UI file 'IndustrialInspection.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INDUSTRIALINSPECTION_H
#define UI_INDUSTRIALINSPECTION_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_IndustrialInspectionClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *IndustrialInspectionClass)
    {
        if (IndustrialInspectionClass->objectName().isEmpty())
            IndustrialInspectionClass->setObjectName("IndustrialInspectionClass");
        IndustrialInspectionClass->resize(600, 400);
        menuBar = new QMenuBar(IndustrialInspectionClass);
        menuBar->setObjectName("menuBar");
        IndustrialInspectionClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(IndustrialInspectionClass);
        mainToolBar->setObjectName("mainToolBar");
        IndustrialInspectionClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(IndustrialInspectionClass);
        centralWidget->setObjectName("centralWidget");
        IndustrialInspectionClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(IndustrialInspectionClass);
        statusBar->setObjectName("statusBar");
        IndustrialInspectionClass->setStatusBar(statusBar);

        retranslateUi(IndustrialInspectionClass);

        QMetaObject::connectSlotsByName(IndustrialInspectionClass);
    } // setupUi

    void retranslateUi(QMainWindow *IndustrialInspectionClass)
    {
        IndustrialInspectionClass->setWindowTitle(QCoreApplication::translate("IndustrialInspectionClass", "IndustrialInspection", nullptr));
    } // retranslateUi

};

namespace Ui {
    class IndustrialInspectionClass: public Ui_IndustrialInspectionClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INDUSTRIALINSPECTION_H

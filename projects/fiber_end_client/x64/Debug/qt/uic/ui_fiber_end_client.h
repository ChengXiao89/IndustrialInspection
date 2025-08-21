/********************************************************************************
** Form generated from reading UI file 'fiber_end_client.ui'
**
** Created by: Qt User Interface Compiler version 6.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FIBER_END_CLIENT_H
#define UI_FIBER_END_CLIENT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_fiber_end_clientClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *fiber_end_clientClass)
    {
        if (fiber_end_clientClass->objectName().isEmpty())
            fiber_end_clientClass->setObjectName("fiber_end_clientClass");
        fiber_end_clientClass->resize(600, 400);
        menuBar = new QMenuBar(fiber_end_clientClass);
        menuBar->setObjectName("menuBar");
        fiber_end_clientClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(fiber_end_clientClass);
        mainToolBar->setObjectName("mainToolBar");
        fiber_end_clientClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(fiber_end_clientClass);
        centralWidget->setObjectName("centralWidget");
        fiber_end_clientClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(fiber_end_clientClass);
        statusBar->setObjectName("statusBar");
        fiber_end_clientClass->setStatusBar(statusBar);

        retranslateUi(fiber_end_clientClass);

        QMetaObject::connectSlotsByName(fiber_end_clientClass);
    } // setupUi

    void retranslateUi(QMainWindow *fiber_end_clientClass)
    {
        fiber_end_clientClass->setWindowTitle(QCoreApplication::translate("fiber_end_clientClass", "fiber_end_client", nullptr));
    } // retranslateUi

};

namespace Ui {
    class fiber_end_clientClass: public Ui_fiber_end_clientClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FIBER_END_CLIENT_H

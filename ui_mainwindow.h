/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionUpload_flow_data;
    QAction *actionDownload_Weather_Data;
    QAction *actionDownload_GeoTiff;
    QAction *actionUniformize_Flow_and_Rain;
    QAction *actionRead_Weather_Data;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout;
    QMenuBar *menubar;
    QMenu *menuData;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(800, 600);
        actionUpload_flow_data = new QAction(MainWindow);
        actionUpload_flow_data->setObjectName(QString::fromUtf8("actionUpload_flow_data"));
        actionDownload_Weather_Data = new QAction(MainWindow);
        actionDownload_Weather_Data->setObjectName(QString::fromUtf8("actionDownload_Weather_Data"));
        actionDownload_GeoTiff = new QAction(MainWindow);
        actionDownload_GeoTiff->setObjectName(QString::fromUtf8("actionDownload_GeoTiff"));
        actionUniformize_Flow_and_Rain = new QAction(MainWindow);
        actionUniformize_Flow_and_Rain->setObjectName(QString::fromUtf8("actionUniformize_Flow_and_Rain"));
        actionRead_Weather_Data = new QAction(MainWindow);
        actionRead_Weather_Data->setObjectName(QString::fromUtf8("actionRead_Weather_Data"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

        verticalLayout->addLayout(gridLayout);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 22));
        menuData = new QMenu(menubar);
        menuData->setObjectName(QString::fromUtf8("menuData"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuData->menuAction());
        menuData->addAction(actionUpload_flow_data);
        menuData->addAction(actionDownload_Weather_Data);
        menuData->addAction(actionDownload_GeoTiff);
        menuData->addAction(actionUniformize_Flow_and_Rain);
        menuData->addAction(actionRead_Weather_Data);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        actionUpload_flow_data->setText(QCoreApplication::translate("MainWindow", "Download flow data", nullptr));
        actionDownload_Weather_Data->setText(QCoreApplication::translate("MainWindow", "Download Weather Data", nullptr));
        actionDownload_GeoTiff->setText(QCoreApplication::translate("MainWindow", "Download GeoTiff", nullptr));
        actionUniformize_Flow_and_Rain->setText(QCoreApplication::translate("MainWindow", "Uniformize Flow and Rain", nullptr));
        actionRead_Weather_Data->setText(QCoreApplication::translate("MainWindow", "Read Weather Data", nullptr));
        menuData->setTitle(QCoreApplication::translate("MainWindow", "Data", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

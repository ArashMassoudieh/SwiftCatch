/********************************************************************************
** Form generated from reading UI file 'weatherdownloaderdlg.ui'
**
** Created by: Qt User Interface Compiler version 6.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WEATHERDOWNLOADERDLG_H
#define UI_WEATHERDOWNLOADERDLG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_WeatherDownloaderDlg
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QComboBox *StatescomboBox;
    QLabel *label_2;
    QComboBox *SelectStationcomboBox;
    QLabel *label_3;
    QDateTimeEdit *dateEditStart;
    QLabel *label_4;
    QDateTimeEdit *dateEditEnd;
    QLabel *label_5;
    QHBoxLayout *horizontalLayout;
    QPushButton *FetchDataBtm;
    QPushButton *ExporttoCSV;
    QTextBrowser *MetaDatatextBrowser;
    QLabel *label_6;
    QTextBrowser *textBrowserPrecip;

    void setupUi(QDialog *WeatherDownloaderDlg)
    {
        if (WeatherDownloaderDlg->objectName().isEmpty())
            WeatherDownloaderDlg->setObjectName(QString::fromUtf8("WeatherDownloaderDlg"));
        WeatherDownloaderDlg->resize(685, 732);
        verticalLayout = new QVBoxLayout(WeatherDownloaderDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(WeatherDownloaderDlg);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label);

        StatescomboBox = new QComboBox(WeatherDownloaderDlg);
        StatescomboBox->setObjectName(QString::fromUtf8("StatescomboBox"));

        formLayout->setWidget(0, QFormLayout::FieldRole, StatescomboBox);

        label_2 = new QLabel(WeatherDownloaderDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label_2);

        SelectStationcomboBox = new QComboBox(WeatherDownloaderDlg);
        SelectStationcomboBox->setObjectName(QString::fromUtf8("SelectStationcomboBox"));

        formLayout->setWidget(1, QFormLayout::FieldRole, SelectStationcomboBox);

        label_3 = new QLabel(WeatherDownloaderDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(4, QFormLayout::LabelRole, label_3);

        dateEditStart = new QDateTimeEdit(WeatherDownloaderDlg);
        dateEditStart->setObjectName(QString::fromUtf8("dateEditStart"));

        formLayout->setWidget(4, QFormLayout::FieldRole, dateEditStart);

        label_4 = new QLabel(WeatherDownloaderDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(5, QFormLayout::LabelRole, label_4);

        dateEditEnd = new QDateTimeEdit(WeatherDownloaderDlg);
        dateEditEnd->setObjectName(QString::fromUtf8("dateEditEnd"));

        formLayout->setWidget(5, QFormLayout::FieldRole, dateEditEnd);

        label_5 = new QLabel(WeatherDownloaderDlg);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        formLayout->setWidget(6, QFormLayout::LabelRole, label_5);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        FetchDataBtm = new QPushButton(WeatherDownloaderDlg);
        FetchDataBtm->setObjectName(QString::fromUtf8("FetchDataBtm"));

        horizontalLayout->addWidget(FetchDataBtm);

        ExporttoCSV = new QPushButton(WeatherDownloaderDlg);
        ExporttoCSV->setObjectName(QString::fromUtf8("ExporttoCSV"));

        horizontalLayout->addWidget(ExporttoCSV);


        formLayout->setLayout(6, QFormLayout::FieldRole, horizontalLayout);

        MetaDatatextBrowser = new QTextBrowser(WeatherDownloaderDlg);
        MetaDatatextBrowser->setObjectName(QString::fromUtf8("MetaDatatextBrowser"));

        formLayout->setWidget(2, QFormLayout::FieldRole, MetaDatatextBrowser);

        label_6 = new QLabel(WeatherDownloaderDlg);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_6);

        textBrowserPrecip = new QTextBrowser(WeatherDownloaderDlg);
        textBrowserPrecip->setObjectName(QString::fromUtf8("textBrowserPrecip"));

        formLayout->setWidget(3, QFormLayout::FieldRole, textBrowserPrecip);


        verticalLayout->addLayout(formLayout);


        retranslateUi(WeatherDownloaderDlg);

        QMetaObject::connectSlotsByName(WeatherDownloaderDlg);
    } // setupUi

    void retranslateUi(QDialog *WeatherDownloaderDlg)
    {
        WeatherDownloaderDlg->setWindowTitle(QCoreApplication::translate("WeatherDownloaderDlg", "Weather Downloader", nullptr));
        label->setText(QCoreApplication::translate("WeatherDownloaderDlg", "State", nullptr));
        label_2->setText(QCoreApplication::translate("WeatherDownloaderDlg", "Station", nullptr));
        label_3->setText(QCoreApplication::translate("WeatherDownloaderDlg", "Start Date", nullptr));
        label_4->setText(QCoreApplication::translate("WeatherDownloaderDlg", "End Date", nullptr));
        label_5->setText(QString());
        FetchDataBtm->setText(QCoreApplication::translate("WeatherDownloaderDlg", "Retrive Data", nullptr));
        ExporttoCSV->setText(QCoreApplication::translate("WeatherDownloaderDlg", "Export to CSV", nullptr));
        label_6->setText(QCoreApplication::translate("WeatherDownloaderDlg", "Meta Data", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WeatherDownloaderDlg: public Ui_WeatherDownloaderDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WEATHERDOWNLOADERDLG_H

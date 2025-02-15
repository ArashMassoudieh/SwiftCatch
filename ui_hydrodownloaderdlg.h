/********************************************************************************
** Form generated from reading UI file 'hydrodownloaderdlg.ui'
**
** Created by: Qt User Interface Compiler version 6.3.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HYDRODOWNLOADERDLG_H
#define UI_HYDRODOWNLOADERDLG_H

#include <QtCore/QDate>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_HydroDownloaderDlg
{
public:
    QVBoxLayout *verticalLayout;
    QFormLayout *formLayout;
    QLabel *label;
    QComboBox *SelectStationcomboBox;
    QLabel *label_2;
    QComboBox *StatescomboBox;
    QDateEdit *dateEditStart;
    QLabel *label_3;
    QLabel *label_4;
    QDateEdit *dateEditEnd;
    QHBoxLayout *horizontalLayout;
    QPushButton *RetrieveDataBtn;
    QPushButton *ExporttoCSV;

    void setupUi(QDialog *HydroDownloaderDlg)
    {
        if (HydroDownloaderDlg->objectName().isEmpty())
            HydroDownloaderDlg->setObjectName(QString::fromUtf8("HydroDownloaderDlg"));
        HydroDownloaderDlg->resize(875, 300);
        verticalLayout = new QVBoxLayout(HydroDownloaderDlg);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        formLayout = new QFormLayout();
        formLayout->setObjectName(QString::fromUtf8("formLayout"));
        label = new QLabel(HydroDownloaderDlg);
        label->setObjectName(QString::fromUtf8("label"));

        formLayout->setWidget(1, QFormLayout::LabelRole, label);

        SelectStationcomboBox = new QComboBox(HydroDownloaderDlg);
        SelectStationcomboBox->setObjectName(QString::fromUtf8("SelectStationcomboBox"));

        formLayout->setWidget(1, QFormLayout::FieldRole, SelectStationcomboBox);

        label_2 = new QLabel(HydroDownloaderDlg);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        formLayout->setWidget(0, QFormLayout::LabelRole, label_2);

        StatescomboBox = new QComboBox(HydroDownloaderDlg);
        StatescomboBox->setObjectName(QString::fromUtf8("StatescomboBox"));

        formLayout->setWidget(0, QFormLayout::FieldRole, StatescomboBox);

        dateEditStart = new QDateEdit(HydroDownloaderDlg);
        dateEditStart->setObjectName(QString::fromUtf8("dateEditStart"));
        dateEditStart->setDate(QDate(2010, 1, 1));

        formLayout->setWidget(2, QFormLayout::FieldRole, dateEditStart);

        label_3 = new QLabel(HydroDownloaderDlg);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        formLayout->setWidget(2, QFormLayout::LabelRole, label_3);

        label_4 = new QLabel(HydroDownloaderDlg);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        formLayout->setWidget(3, QFormLayout::LabelRole, label_4);

        dateEditEnd = new QDateEdit(HydroDownloaderDlg);
        dateEditEnd->setObjectName(QString::fromUtf8("dateEditEnd"));
        dateEditEnd->setDate(QDate(2010, 1, 1));

        formLayout->setWidget(3, QFormLayout::FieldRole, dateEditEnd);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        RetrieveDataBtn = new QPushButton(HydroDownloaderDlg);
        RetrieveDataBtn->setObjectName(QString::fromUtf8("RetrieveDataBtn"));

        horizontalLayout->addWidget(RetrieveDataBtn);

        ExporttoCSV = new QPushButton(HydroDownloaderDlg);
        ExporttoCSV->setObjectName(QString::fromUtf8("ExporttoCSV"));

        horizontalLayout->addWidget(ExporttoCSV);


        formLayout->setLayout(4, QFormLayout::FieldRole, horizontalLayout);


        verticalLayout->addLayout(formLayout);


        retranslateUi(HydroDownloaderDlg);

        QMetaObject::connectSlotsByName(HydroDownloaderDlg);
    } // setupUi

    void retranslateUi(QDialog *HydroDownloaderDlg)
    {
        HydroDownloaderDlg->setWindowTitle(QCoreApplication::translate("HydroDownloaderDlg", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("HydroDownloaderDlg", "SelectStation", nullptr));
        label_2->setText(QCoreApplication::translate("HydroDownloaderDlg", "Select State", nullptr));
        dateEditStart->setDisplayFormat(QCoreApplication::translate("HydroDownloaderDlg", "MM/dd/yyyy", nullptr));
        label_3->setText(QCoreApplication::translate("HydroDownloaderDlg", "Start Date", nullptr));
        label_4->setText(QCoreApplication::translate("HydroDownloaderDlg", "End Date", nullptr));
        dateEditEnd->setDisplayFormat(QCoreApplication::translate("HydroDownloaderDlg", "MM/dd/yyyy", nullptr));
        RetrieveDataBtn->setText(QCoreApplication::translate("HydroDownloaderDlg", "Retrieve Data", nullptr));
        ExporttoCSV->setText(QCoreApplication::translate("HydroDownloaderDlg", "Export to CSV", nullptr));
    } // retranslateUi

};

namespace Ui {
    class HydroDownloaderDlg: public Ui_HydroDownloaderDlg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HYDRODOWNLOADERDLG_H

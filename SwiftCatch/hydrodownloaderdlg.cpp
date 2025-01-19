#include "hydrodownloaderdlg.h"
#include "ui_hydrodownloaderdlg.h"
#include "hydrodownloader.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QStringList>
#include <QDebug>

HydroDownloaderDlg::HydroDownloaderDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HydroDownloaderDlg)
{
    ui->setupUi(this);

    ui->SelectStationcomboBox->setEnabled(false);
    ui->StatescomboBox->setEnabled(false);
    QStringList StateCodes;
    fetchStateCodes("https://raw.githubusercontent.com/ArashMassoudieh/State_Codes/main/State_Codes");

    ui->StatescomboBox->addItems(StateCodes);
    ui->StatescomboBox->setEnabled(true);
    connect(ui->StatescomboBox,SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_State_Changed()));

}

HydroDownloaderDlg::~HydroDownloaderDlg()
{
    delete ui;
}

void HydroDownloaderDlg::fetchStateCodes(const QString& url) {
    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &HydroDownloaderDlg::on_States_Downloaded);
    // Send the GET request
    reply = manager->get(QNetworkRequest(QUrl(url)));
}


void HydroDownloaderDlg::on_States_Downloaded()
{
    if (reply->error() == QNetworkReply::NoError) {
        // Read the response
        QByteArray responseData = reply->readAll();
        QString responseString = QString::fromUtf8(responseData);

        // Split the response into words (assuming each word is on a new line)
        QStringList wordList = responseString.split("\n", Qt::SkipEmptyParts);

        // Populate the combo box
        ui->StatescomboBox->addItems(wordList);

        qDebug() << "Words added to combo box:" << wordList;
    } else {
        qDebug() << "Error fetching words:" << reply->errorString();
    }

    reply->deleteLater();
}

void HydroDownloaderDlg::on_State_Changed()
{
    ui->SelectStationcomboBox->clear();
    HydroDownloader hydrodowloader;
    QVector<station_info> stations = hydrodowloader.fetchAllStations(ui->StatescomboBox->currentText());
    for (unsigned int i = 0; i<stations.size(); i++)
        ui->SelectStationcomboBox->addItem(stations[i].station_nm);
    ui->SelectStationcomboBox->setEnabled(true);
}


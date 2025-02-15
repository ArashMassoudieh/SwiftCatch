#include "weatherdownloaderdlg.h"
#include "ui_weatherdownloaderdlg.h"
#include <QtNetwork/QNetworkAccessManager>
#include "hydrodownloader.h"
#include "Utilities/BTC.h"

WeatherDownloaderDlg::WeatherDownloaderDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WeatherDownloaderDlg)
{
    ui->setupUi(this);
    ui->StatescomboBox->setEnabled(false);
    ui->SelectStationcomboBox->setEnabled(false);
    ui->dateEditStart->setEnabled(false);
    ui->dateEditEnd->setEnabled(false);
    ui->FetchDataBtm->setEnabled(false);
    ui->ExporttoCSV->setEnabled(false);
    fetchStateCodes("https://raw.githubusercontent.com/ArashMassoudieh/State_Codes/main/State_Codes");
    connect(ui->FetchDataBtm, SIGNAL(clicked()),this, SLOT(on_Retreive_Data()));
    connect(ui->dateEditEnd,SIGNAL(editingFinished()), this, SLOT(on_Date_Changed()));


}

WeatherDownloaderDlg::~WeatherDownloaderDlg()
{
    delete ui;
}

void WeatherDownloaderDlg::fetchStateCodes(const QString& url) {
    manager = new QNetworkAccessManager();
    connect(manager, &QNetworkAccessManager::finished, this, &WeatherDownloaderDlg::on_States_Downloaded);
    // Send the GET request
    reply = manager->get(QNetworkRequest(QUrl(url)));
}


void WeatherDownloaderDlg::on_States_Downloaded()
{
    if (reply->error() == QNetworkReply::NoError) {
        // Read the response
        QByteArray responseData = reply->readAll();
        QString responseString = QString::fromUtf8(responseData);

        // Split the response into words (assuming each word is on a new line)
        QStringList wordList = responseString.split("\n", Qt::SkipEmptyParts);
        for (int i=0; i<wordList.size(); i++)
        {
            QStringList line = wordList[i].split(',');
            if (line.size() == 3)
            {
                State_Info stateinfo;
                stateinfo.Name = line[1];
                stateinfo.Code = line[0];
                stateinfo.FIPS = line[2];
                ui->StatescomboBox->addItem(stateinfo.Code);
                StatesInformation[stateinfo.Code] = stateinfo;
            }
        }
        // Populate the combo box


        qDebug() << "Words added to combo box:" << wordList;
    } else {
        qDebug() << "Error fetching words:" << reply->errorString();
    }

    reply->deleteLater();
    ui->StatescomboBox->setEnabled(true);
    connect(ui->StatescomboBox,SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_State_Changed()));
}

void WeatherDownloaderDlg::on_State_Changed()
{
    ui->SelectStationcomboBox->clear();
    HydroDownloader hydrodownloader;
    stations.clear();

    stations = hydrodownloader.fetchPrecipitationStations(StatesInformation[ui->StatescomboBox->currentText()].FIPS,"AuOQEjHeTwRMJeUjLpoXmneFKxUDdred",precip_time_interval::PRECIP_5);
    for (const QString& key : stations.keys())
        ui->SelectStationcomboBox->addItem(key);

    ui->SelectStationcomboBox->setEnabled(true);
    connect(ui->SelectStationcomboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(on_Station_Selected()));

}

void WeatherDownloaderDlg::on_Station_Selected()
{
    ui->dateEditEnd->setEnabled(true);
    ui->dateEditStart->setEnabled(true);
    HydroDownloader hydrodownloader;
    QSet<DataType> Station_info = hydrodownloader.fetchAllDataTypesForStation(stations[ui->SelectStationcomboBox->currentText()].id,"AuOQEjHeTwRMJeUjLpoXmneFKxUDdred");
    ui->MetaDatatextBrowser->clear();
    ui->textBrowserPrecip->clear();
    for (const DataType& item : Station_info) {

        if (item.id!="HPCP")
            ui->MetaDatatextBrowser->append("ID: " + item.id + ", Name: " + item.name + ", Date range:" + item.min_date + ":" +item.max_date);
        else
            ui->MetaDatatextBrowser->append("<span style='color: red;'>ID: " + item.id + "</span>, Name: " + item.name + ", Date range:" + item.min_date + ":" +item.max_date);

        if (item.name.toLower().contains("precipitation"))
        {
            ui->textBrowserPrecip->append("ID: " + item.id + ", Name: " + item.name + ", Date range:" + item.min_date + ":" +item.max_date);
        }

    }

}

void WeatherDownloaderDlg::on_Retreive_Data()
{
    HydroDownloader hydrodownloader;
    qDebug()<<stations[ui->SelectStationcomboBox->currentText()].id;
    QVector<PrecipitationData> precipitation = hydrodownloader.fetchPrecipitationData(stations[ui->SelectStationcomboBox->currentText()].id,ui->dateEditStart->dateTime().toString("yyyy-MM-dd"),ui->dateEditEnd->dateTime().toString("yyyy-MM-dd"),"AuOQEjHeTwRMJeUjLpoXmneFKxUDdred",precip_time_interval::PRECIP_5);
    qDebug()<<precipitation.size();
    CTimeSeries<double> precipitation_TS;
    for (int i=0; i<precipitation.size(); i++)
    {
        precipitation_TS.append(qDateTimeToExcel(precipitation[i].dateTime),precipitation[i].precipitation);
    }
    precipitation_TS.writefile("precipitation.csv");
}

void WeatherDownloaderDlg::on_Date_Changed()
{
    if (ui->dateEditStart->date() < ui->dateEditEnd->date())
        ui->FetchDataBtm->setEnabled(true);
}


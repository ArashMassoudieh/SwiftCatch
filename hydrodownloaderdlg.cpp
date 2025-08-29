#include "hydrodownloaderdlg.h"
#include "ui_hydrodownloaderdlg.h"
#include "hydrodownloader.h"
#include <QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QStringList>
#include <QDebug>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QFileDialog>
#include <QDir>
#include "TableViewer.h"
#include "GeoDataModel.h"
#include "MapDialog.h"

using namespace QtCharts;

//QT_CHARTS_USE_NAMESPACE

HydroDownloaderDlg::HydroDownloaderDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HydroDownloaderDlg)
{
    ui->setupUi(this);
    ui->RetrieveDataBtn->setEnabled(false);
    ui->SelectStationcomboBox->setEnabled(false);
    ui->StatescomboBox->setEnabled(false);
    ui->ExporttoCSV->setEnabled(false);
    ui->dateEditEnd->setEnabled(false);
    ui->dateEditStart->setEnabled(false);
    QStringList StateCodes;
    fetchStateCodes("https://raw.githubusercontent.com/ArashMassoudieh/State_Codes/main/State_Codes");
    ui->StatescomboBox->addItems(StateCodes);
    ui->StatescomboBox->setEnabled(true);
    connect(ui->StatescomboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(on_State_Changed()));
    connect(ui->SelectStationcomboBox,SIGNAL(currentIndexChanged(int)), this, SLOT(on_Station_Selected()));
    connect(ui->dateEditEnd,SIGNAL(editingFinished()), this, SLOT(on_Date_Changed()));
    connect(ui->RetrieveDataBtn,SIGNAL(clicked()), this, SLOT(on_DataRetrieveRequested()));
    connect(ui->ExporttoCSV,SIGNAL(clicked()), this, SLOT(on_ExporttoCSV()));
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
            }
        }
        // Populate the combo box


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
    stations.clear();
    stations = hydrodowloader.fetchAllHydroStations(ui->StatescomboBox->currentText());
    PointGeoDataSet *pointgeodata = new PointGeoDataSet(stations);
    GeoDataModel* pointgeodatamodel = new GeoDataModel(pointgeodata);
    for (const QString& key : stations.keys())
        ui->SelectStationcomboBox->addItem(key);
    ui->SelectStationcomboBox->setEnabled(true);
    TableViewer *tableviewer = new TableViewer();
    tableviewer->setModel(pointgeodatamodel);
    tableviewer->show(); 
    MapDialog* map = new MapDialog(this); 
    map->AddLayer("USGS Stations", pointgeodata, "Site Type");
    map->show(); 

}

void HydroDownloaderDlg::on_Station_Selected()
{
    ui->dateEditEnd->setEnabled(true);
    ui->dateEditStart->setEnabled(true);

}

void HydroDownloaderDlg::on_ExporttoCSV()
{
    QString filePath = QFileDialog::getSaveFileName(
        nullptr,                     // Parent widget
        "Save CSV File",             // Dialog title
        QDir::homePath(),            // Default directory
        "CSV Files (*.csv);;All Files (*)" // File type filters
    );

    // Check if the user selected a file
    if (filePath.isEmpty()) {
        qDebug() << "No file selected.";
        return;
    }
    if (!filePath.contains("."))
        filePath += ".csv";
    UniformizedTimeSeries.writefile(filePath.toStdString());
}

void HydroDownloaderDlg::on_DataRetrieveRequested()
{
    HydroDownloader hydrodowloader;
    QVector<FlowData> flowdata = hydrodowloader.fetchFlowData(stations[ui->SelectStationcomboBox->currentText()].site_no,ui->dateEditStart->dateTime().toString("yyyy-MM-ddThh:mm:ssZ"),ui->dateEditEnd->dateTime().toString("yyyy-MM-ddThh:mm:ssZ"));
    qDebug()<<flowdata.size();
    CTimeSeries<double> OpenHydroQualTimeSeries;
    for (int i=0; i<flowdata.size(); i++)
        OpenHydroQualTimeSeries.append(qDateTimeToExcel(flowdata[i].dateTime),flowdata[i].flowRate*pow(0.3048,3)*86400);

    UniformizedTimeSeries = OpenHydroQualTimeSeries.make_uniform(1.0/24.0,qDateTimeToExcel(ui->dateEditStart->dateTime()));

    showGraph(UniformizedTimeSeries);
}

void HydroDownloaderDlg::on_Date_Changed()
{
    if (ui->dateEditStart->date() < ui->dateEditEnd->date())
        ui->RetrieveDataBtn->setEnabled(true);
}

void HydroDownloaderDlg::showGraph(const CTimeSeries<double> &data) {
    // Create a line series
    QLineSeries* series = new QLineSeries();

    // Populate the series with data
    for (int i=0; i<data.n; i++) {
        series->append(excelToQDateTime(data.GetT(i)).toMSecsSinceEpoch(), data.GetC(i));
    }

    // Create the chart
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Time Series Data");
    chart->setAnimationOptions(QChart::SeriesAnimations);

    // Create and configure the X-axis (QDateTimeAxis)
    QDateTimeAxis* axisX = new QDateTimeAxis();
    axisX->setFormat("yyyy-MM-dd HH:mm");
    axisX->setTitleText("Time");
    axisX->setTickCount(30); // Adjust the number of ticks
    axisX->setRange(excelToQDateTime(data.mint()), excelToQDateTime(data.maxt()));
    axisX->setLabelsAngle(90);
    // Create and configure the Y-axis (QValueAxis)
    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Flow rate [m<sup>3</sup>/day]");
    axisY->applyNiceNumbers(); // Adjust the axis range to fit data

    // Add axes to the chart
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // Configure chart appearance
    chart->legend()->hide(); // Hide legend for simplicity
    chart->setMargins(QMargins(10, 10, 10, 10));
    chart->setBackgroundRoundness(10);

    // Create the chart view
    QChartView* chartView = new QChartView(chart);

    chartView->setRenderHint(QPainter::Antialiasing);

    // Show the chart in a window
    ui->verticalLayout->addWidget(chartView);

    ui->ExporttoCSV->setEnabled(true);
}


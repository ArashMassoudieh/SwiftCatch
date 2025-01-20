#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hydrodownloaderdlg.h"
#include "weatherdownloaderdlg.h"
#include "geodatadownloader.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionUpload_flow_data, SIGNAL(triggered()), this, SLOT(on_Download_flow_data()));
    connect(ui->actionDownload_Weather_Data, SIGNAL(triggered()), this, SLOT(on_Download_Weather_data()));
    connect(ui->actionDownload_GeoTiff, SIGNAL(triggered()), this, SLOT(on_Download_GeoTIFF()));
}

MainWindow::~MainWindow()
{
    delete hydrodownloaderdlg;
    delete ui;

}

void MainWindow::on_Download_flow_data()
{
    hydrodownloaderdlg = new HydroDownloaderDlg(this);
    hydrodownloaderdlg->show();
}

void MainWindow::on_Download_Weather_data()
{
    weatherdownloaderdlg = new WeatherDownloaderDlg(this);
    weatherdownloaderdlg->show();
}

void MainWindow::on_Download_GeoTIFF()
{
    GeoDataDownloader geodatadownloader;
    double minX = -76.9771, minY = 38.9052, maxX = -76.9657, maxY = 38.9172;
    geodatadownloader.fetchDEMData(minX,minY,maxX,maxY);
    geodatadownloader.clipGeoTiffToBoundingBox("downloaded_dem.tif","downloaded_dem_clipped.tif",minX,minY,maxX,maxY);
}

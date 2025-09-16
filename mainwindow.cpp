#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hydrodownloaderdlg.h"
#include "weatherdownloaderdlg.h"
#include "geodatadownloader.h"
#include "weatherdata.h"
#include "QFileDialog"
#include "MapDialog.h"
#include "geotiffhandler.h"
#include <QMessageBox>
#include "path.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionUpload_flow_data, SIGNAL(triggered()), this, SLOT(on_Download_flow_data()));
    connect(ui->actionDownload_Weather_Data, SIGNAL(triggered()), this, SLOT(on_Download_Weather_data()));
    connect(ui->actionDownload_GeoTiff, SIGNAL(triggered()), this, SLOT(on_Download_GeoTIFF()));
    connect(ui->actionUniformize_Flow_and_Rain, SIGNAL(triggered()),this,SLOT(on_Uniformized()));
    connect(ui->actionRead_Weather_Data, SIGNAL(triggered()),this,SLOT(on_ReadWeatherData()));
    connect(ui->actionSelect_Area, SIGNAL(triggered()), this, SLOT(on_Select_Area()));
    connect(ui->actionLoad_GeoTiff, SIGNAL(triggered()), this, SLOT(on_Load_GeoTIFF()));

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
    
    double minX = -77.009, minY = 38.86, maxX = -76.9657, maxY = 38.9172;
    geodatadownloader.fetchDEMData(minX,minY,maxX,maxY,this);
    geodatadownloader.clipGeoTiffToBoundingBox("downloaded_dem.tif","downloaded_dem_clipped.tif",minX,minY,maxX,maxY);
    GDALAllRegister();

    // Open input DEM
    GDALDataset* demDataset = static_cast<GDALDataset*>(GDALOpen("downloaded_dem_clipped.tif", GA_ReadOnly));
    if (!demDataset) {
        cerr << "Failed to open input DEM file." << endl;
        return;
    }
    geodatadownloader.computeFlowDirection(demDataset, "flow_direction.tiff");

    GDALClose(demDataset);

}

void MainWindow::on_Load_GeoTIFF()
{
    QString fileName = getFileNameWithExtension(this,"DEM","/home/arash/Projects/DCDEM/","",DialogMode::Save,"tif");
    GeoTiffHandler dem(fileName.toStdString());

    pair<int,int> highestpoint = dem.maxCellIndex();

    Path path = dem.downstreamPath(highestpoint.first, highestpoint.second,FlowDirType::D8);

    path.saveAsGeoJSON("/home/arash/Projects/DCDEM/dem.geojson");

    dem.saveAsAscii("/home/arash/Projects/DCDEM/dem.asc",-9999);
    // Select target cell with value = 9
    pair<int,int> ID = dem.indicesAt(-77.055615,38.917401);

    qDebug()<<"IDs: " << ID;
    // Compute watershed


    GeoTiffHandler ws = dem.watershedMFD(ID.first, ID.second, FlowDirType::D8);

    // Save masked watershed
    ws.saveAsAscii("/home/arash/Projects/DCDEM/watershed_masked_9.asc", -9999.0);
    ws.saveAs("/home/arash/Projects/DCDEM/watershed_masked_9.tif");

    // Crop watershed
    GeoTiffHandler wsCrop = ws.cropMasked(-9999);
    wsCrop.saveAsAscii("/home/arash/Projects/DCDEM/watershed_cropped_9.asc", -9999.0);
    wsCrop.saveAs("/home/arash/Projects/DCDEM/watershed_cropped_9.tif");

}

void MainWindow::on_Uniformized()
{
    CTimeSeries<double> Flow_Hickey("/home/arash/Dropbox/Watershed_Modeling/flow_HickeyRun.csv");
    CTimeSeries<double> Flow_Watts("/home/arash/Dropbox/Watershed_Modeling/flow_WATTSBRANCH.csv");
    CTimeSeries<double> RainReagan("/home/arash/Dropbox/Watershed_Modeling/Precipitation_ReganAirport.csv");
    CTimeSeries<double> RainReagan_Uniformized_Hickey = RainReagan.make_uniform(1.0/24.0,Flow_Hickey.GetT(0));
    CTimeSeries<double> RainReagan_Uniformized_Watts = RainReagan.make_uniform(1.0/24.0,Flow_Watts.GetT(0));
    CTimeSeries<double> Flow_Hickey_Uniformized = Flow_Hickey.make_uniform(1.0/24.0);
    CTimeSeries<double> Flow_Watts_Uniformized = Flow_Watts.make_uniform(1.0/24.0);
    RainReagan_Uniformized_Hickey.writefile("/home/arash/Dropbox/Watershed_Modeling/Rain_Hickey.csv");
    RainReagan_Uniformized_Watts.writefile("/home/arash/Dropbox/Watershed_Modeling/Rain_Watts.csv");
    Flow_Hickey_Uniformized.writefile("/home/arash/Dropbox/Watershed_Modeling/Flow_Hickey.csv");
    Flow_Watts_Uniformized.writefile("/home/arash/Dropbox/Watershed_Modeling/Flow_Watts.csv");
    cout<<"Files saved!"<<endl;
}

void MainWindow::on_ReadWeatherData()
{
    QString fileName = QFileDialog::getOpenFileName(
           nullptr,
           "Open CSV File",
           "",
           "CSV Files (*.csv);;All Files (*)"
       );

       WeatherData data;
       data.ReadFromFile(fileName);
       WeatherData Filtered_Data = data.filterByColumnValue("REPORT_TYPE","FM-15");

       QString SavefileName = QFileDialog::getSaveFileName(
              nullptr,
              "Open CSV File",
              "",
              "CSV Files (*.csv);;All Files (*)"
          );



       Filtered_Data.writeCSV(SavefileName,"HourlyPrecipitation");
}

void MainWindow::on_Select_Area() {
    MapDialog *mapdialog = new MapDialog();
    mapdialog->AddLayer("C:/Projects/DCGIS/HickyrunClipped.geojson","ftype");
    mapdialog->exec();
}

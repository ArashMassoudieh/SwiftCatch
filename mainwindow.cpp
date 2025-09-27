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
#include "streamnetwork.h"
#include "modelcreator.h"
#include "polylineset.h"
#include "geometrymapdialog.h"
#include <memory>

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
    connect(ui->actionLoad_Transportation_Layer, &QAction::triggered, this, &MainWindow::on_Load_Transportation_Layer);
    connect(ui->actionGet_Closest_Sewer_Pipe, &QAction::triggered, this, &MainWindow::on_Find_Closest_Sewers);

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

// Replace your debug info code in the Transportation Layer loading function:
void MainWindow::on_Load_Transportation_Layer() {
    QString fileName = getFileNameWithExtension(this,"Shape files","","Shapefiles (*.shp);;All Files (*)",DialogMode::Open,"shp");
    if (fileName.isEmpty()) return;

    try {
        auto roads = std::make_shared<PolylineSet>();
        roads->loadFromShapefile(fileName);

        // Show detailed debug info - FIXED VERSION
        QString debugInfo = QString("Loaded: %1 polylines\nTotal points: %2")
                                .arg(roads->size())
                                .arg(roads->getTotalPointCount());

        if (roads->empty()) {
            QMessageBox::information(this, "Debug Info",
                                     "No polylines loaded. The shapefile might contain:\n"
                                     "- Point geometry (not supported by PolylineSet)\n"
                                     "- Polygon geometry (not supported by PolylineSet)\n"
                                     "- Empty features\n\n" + debugInfo);
            return;
        }

        auto bbox = roads->getBoundingBox();

        // CORRECT way to format the bounding box with 4 arguments
        debugInfo += QString("\nBounding box: (%1, %2) to (%3, %4)")
                         .arg(bbox.first.x)
                         .arg(bbox.first.y)
                         .arg(bbox.second.x)
                         .arg(bbox.second.y);

        QMessageBox::information(this, "Debug Info", debugInfo);

        // Create viewer
        GeometryMapDialog* mapDialog = new GeometryMapDialog(this);
        QString layerName = QFileInfo(fileName).baseName();
        QFileInfo info(fileName);
        QString folderPath = info.absolutePath() + "/";
        roads->saveAsEnhancedGeoJSON(folderPath + "roads.geojson");
        mapDialog->addGeometryLayer(layerName, roads, Qt::red, 3, 0);
        mapDialog->show();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Failed to load: %1").arg(e.what()));
    }
}

void MainWindow::on_Find_Closest_Sewers()
{
    try {
        // Step 1: Get input DEM/raster file
        QString demFileName = getFileNameWithExtension(
            this,
            "Select Input Raster (DEM)",
            "",
            "GeoTIFF Files (*.tif *.tiff);;All Files (*)",
            DialogMode::Open,
            "tif"
            );

        if (demFileName.isEmpty()) {
            return; // User cancelled
        }

        // Step 2: Get polyline shapefile
        QString shapeFileName = getFileNameWithExtension(
            this,
            "Select Polyline Shapefile",
            "",
            "Shapefiles (*.shp);;All Files (*)",
            DialogMode::Open,
            "shp"
            );

        if (shapeFileName.isEmpty()) {
            return; // User cancelled
        }

        // Step 3: Get output file location
        QString outputFileName = getFileNameWithExtension(
            this,
            "Save Closest Polyline Raster As",
            "",
            "GeoTIFF Files (*.tif *.tiff);;All Files (*)",
            DialogMode::Save,
            "tif"
            );

        if (outputFileName.isEmpty()) {
            return; // User cancelled
        }

        // Step 4: Load the input raster
        GeoTiffHandler inputRaster(demFileName.toStdString());

        // Step 5: Load the polylines
        PolylineSet polylines;
        polylines.loadFromShapefile(shapeFileName);

        // Step 6: Create closest polyline raster
        GeoTiffHandler closestPolylineRaster = inputRaster.closestPolylineRaster(polylines, -1.0);

        // Step 7: Save the result
        closestPolylineRaster.saveAs(outputFileName.toStdString());

        qDebug() << closestPolylineRaster.info(outputFileName);

                // Optional: Show completion message
        QMessageBox::information(this, "Success",
                                 QString("Closest polyline raster saved to:\n%1\n\n"
                                         "Raster contains polyline indices (0-%2) where:\n"
                                         "- Each pixel value represents the index of the closest polyline\n"
                                         "- Value -1 indicates no data/invalid pixels")
                                     .arg(outputFileName)
                                     .arg(polylines.size() - 1));


        GeoTiffHandler::diagnoseGeoTiff(outputFileName.toStdString());

        QFileInfo info(outputFileName);
        QString folderPath = info.absolutePath() + "/";

        polylines.calculateProjectedSlopes(&inputRaster);

        polylines.saveAsShapefile(folderPath + "Roads_with_projected_slopes.shp");

        polylines.exportNumericAttributesToCSV(folderPath + "Roads_with_projected_slopes.csv");

        return ;

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error",
                              QString("Failed to create closest polyline raster:\n%1").arg(e.what()));
        return ;
    }
}


void MainWindow::on_Load_GeoTIFF()
{
    QString fileName = getFileNameWithExtension(this,"DEM","","",DialogMode::Open,"tif");

    QFileInfo info(fileName);
    QString folderPath = info.absolutePath() + "/";

    GeoTiffHandler dem(fileName.toStdString());

    //resampling
    GeoTiffHandler dem_resampled = dem.resampleAverage(30,30);

    dem_resampled.saveAs(folderPath.toStdString() + "dem_resampled.tif");

    dem_resampled.saveAsAscii(folderPath.toStdString() + "dem_resampled.asc",-9999);

    // Sinks
    GeoTiffHandler sinks = dem_resampled.detectSinks(FlowDirType::D8);

    sinks.saveAs(folderPath.toStdString() + "sinks.tiff");

    GeoTiffHandler sinks_filled = dem_resampled.fillSinksIterative(FlowDirType::D8);

    sinks_filled.saveAs(folderPath.toStdString() + "sinksfilled.tiff");

    //pair<int,int> highestpoint = dem_resampled.maxCellIndex();

    pair<int,int> highestpoint = sinks_filled.indicesAt(325684, 4320369);
    Path path = sinks_filled.downstreamPath(highestpoint.first, highestpoint.second,FlowDirType::D8);

    path.saveAsGeoJSON(folderPath +  "path.geojson");

    pair<int,int> ID = sinks_filled.minCellIndex();

    qDebug()<<"IDs: " << ID;
    // Compute watershed


    pair<int,int> pourpoint = sinks_filled.indicesAt(327666.6,4316298.0 );

    GeoTiffHandler ws = sinks_filled.watershedMFD(pourpoint.first, pourpoint.second, FlowDirType::D8);



    // Save masked watershed
    ws.saveAsAscii(folderPath.toStdString() + "watershed_masked.asc", -9999.0);
    ws.saveAs(folderPath.toStdString() + "watershed_masked.tif");

    // Crop watershed
    GeoTiffHandler wsCrop = ws.cropMasked(-9999);
    wsCrop.saveAsAscii(folderPath.toStdString() + "watershed_cropped.asc", -9999.0);
    wsCrop.saveAs(folderPath.toStdString() + "watershed_cropped.tif");

    QMessageBox::information(this, "GeoTIFF Information", wsCrop.info("watershed_cropped.tif"));

    // Flow Accumulation
    GeoTiffHandler flowaccumulation = wsCrop.flowAccumulationMFD(FlowDirType::D8);
    flowaccumulation.saveAs(folderPath.toStdString() + "flow_accumulation.tif");

    GeoTiffHandler flowline = flowaccumulation.filterByThreshold(flowaccumulation.area()*0.05, GeoTiffHandler::FilterMode::Greater);
    flowline.saveAs(folderPath.toStdString() + "flow_line.tif");

    std::vector<Node> nodes = flowline.nodes(&wsCrop);

    StreamNetwork network = StreamNetwork::buildDirected(nodes);

    network.saveEdgesAsGeoJSON(folderPath + "network.geojson");

    ModelCreator modelcreator(wsCrop,network);
    modelcreator.saveModel(folderPath + "Model/model.json");
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

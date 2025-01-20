#include "geodatadownloader.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QEventLoop>
#include <gdal_priv.h>
#include <vector>
#include <string>
#include <ogr_spatialref.h>
#include <iostream>



GeoDataDownloader::GeoDataDownloader()
{

}

// Function to read the GeoTIFF file into a 2D vector
std::vector<std::vector<double>> GeoDataDownloader::readGeoTiffToVector(const std::string &filePath) {
    std::vector<std::vector<double>> demData;

    GDALAllRegister();
    GDALDataset *dataset = (GDALDataset *)GDALOpen(filePath.c_str(), GA_ReadOnly);
    if (!dataset) {
        qCritical() << "Failed to open GeoTIFF file.";
        return demData;
    }

    GDALRasterBand *band = dataset->GetRasterBand(1);
    int xSize = band->GetXSize();
    int ySize = band->GetYSize();

    std::vector<double> row(xSize);
    double *scanline = new double[xSize];

    for (int y = 0; y < ySize; ++y) {
        band->RasterIO(GF_Read, 0, y, xSize, 1, scanline, xSize, 1, GDT_Float64, 0, 0);
        std::copy(scanline, scanline + xSize, row.begin());
        demData.push_back(row);
    }

    delete[] scanline;
    GDALClose(dataset);

    return demData;
}

// Function to download and process DEM data
std::vector<std::vector<double>> GeoDataDownloader::fetchDEMData(double minX, double minY, double maxX, double maxY) {
    // Create the USGS API URL using the bounding box
    QString bbox = QString("%1,%2,%3,%4").arg(minX).arg(minY).arg(maxX).arg(maxY);
    QUrl apiUrl(QString("https://tnmaccess.nationalmap.gov/api/v1/products?datasets=National%20Elevation%20Dataset%20(NED)&bbox=%1&outputFormat=JSON").arg(bbox));
    QString localFilePath = "downloaded_dem.tif";

    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply = manager.get(QNetworkRequest(apiUrl));

    // Wait for the reply
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        qCritical() << "Failed to fetch DEM metadata:" << reply->errorString();
        reply->deleteLater();
        return {};
    }

    // Parse the API response
    QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();

    if (!jsonDoc.isObject()) {
        qCritical() << "Invalid JSON response.";
        return {};
    }

    QJsonArray items = jsonDoc.object()["items"].toArray();
    if (items.isEmpty()) {
        qCritical() << "No DEM data found for the given area.";
        return {};
    }

    // Get the first download URL
    QJsonObject firstItem = items[0].toObject();
    QString downloadUrl = firstItem["downloadURL"].toString();
    if (downloadUrl.isEmpty()) {
        qCritical() << "No download URL found in the metadata.";
        return {};
    }

    // Download the DEM file
    QNetworkReply *downloadReply = manager.get(QNetworkRequest(QUrl(downloadUrl)));
    QObject::connect(downloadReply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    if (downloadReply->error() != QNetworkReply::NoError) {
        qCritical() << "Failed to download DEM file:" << downloadReply->errorString();
        downloadReply->deleteLater();
        return {};
    }

    // Save the DEM file locally
    QFile file(localFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qCritical() << "Failed to save the DEM file.";
        downloadReply->deleteLater();
        return {};
    }
    file.write(downloadReply->readAll());
    file.close();
    downloadReply->deleteLater();

    // Process the DEM file and return the data
    return readGeoTiffToVector(localFilePath.toStdString());
}




bool GeoDataDownloader::clipGeoTiffToBoundingBox(const std::string &inputFile,
                              const std::string &outputFile,
                              double minX, double minY,
                              double maxX, double maxY) {
    GDALAllRegister();

    // Open the input GeoTIFF
    GDALDataset *inputDataset = (GDALDataset *)GDALOpen(inputFile.c_str(), GA_ReadOnly);
    if (!inputDataset) {
        std::cerr << "Error: Unable to open input GeoTIFF: " << inputFile << std::endl;
        return false;
    }

    // Get the GeoTransform of the input dataset
    double geoTransform[6];
    if (inputDataset->GetGeoTransform(geoTransform) != CE_None) {
        std::cerr << "Error: Unable to retrieve GeoTransform from input file." << std::endl;
        GDALClose(inputDataset);
        return false;
    }

    // Get the spatial reference
    OGRSpatialReference inputSRS;
    inputSRS.importFromWkt(inputDataset->GetProjectionRef());

    // Calculate pixel/line offsets and sizes for the bounding box
    int xOffset = static_cast<int>((minX - geoTransform[0]) / geoTransform[1]);
    int yOffset = static_cast<int>((minY - geoTransform[3]) / geoTransform[5]);
    int xSize = static_cast<int>((maxX - minX) / geoTransform[1]);
    int ySize = static_cast<int>((maxY - minY) / std::abs(geoTransform[5]));

    // Ensure offsets and sizes are within bounds
    xOffset = std::max(0, xOffset);
    yOffset = std::max(0, yOffset);
    xSize = std::min(xSize, inputDataset->GetRasterXSize() - xOffset);
    ySize = std::min(ySize, inputDataset->GetRasterYSize() - yOffset);

    if (xSize <= 0 || ySize <= 0) {
        std::cerr << "Error: Clipping area is out of bounds or invalid." << std::endl;
        GDALClose(inputDataset);
        return false;
    }

    // Create the output dataset
    GDALDriver *driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) {
        std::cerr << "Error: Unable to get GeoTIFF driver." << std::endl;
        GDALClose(inputDataset);
        return false;
    }

    GDALDataset *outputDataset = driver->Create(outputFile.c_str(), xSize, ySize, inputDataset->GetRasterCount(), GDT_Float32, nullptr);
    if (!outputDataset) {
        std::cerr << "Error: Unable to create output GeoTIFF: " << outputFile << std::endl;
        GDALClose(inputDataset);
        return false;
    }

    // Set GeoTransform and projection on the output dataset
    double outputGeoTransform[6] = {
        geoTransform[0] + xOffset * geoTransform[1],
        geoTransform[1],
        geoTransform[2],
        geoTransform[3] + yOffset * geoTransform[5],
        geoTransform[4],
        geoTransform[5]
    };
    outputDataset->SetGeoTransform(outputGeoTransform);
    outputDataset->SetProjection(inputDataset->GetProjectionRef());

    // Copy raster data
    for (int bandIdx = 1; bandIdx <= inputDataset->GetRasterCount(); ++bandIdx) {
        GDALRasterBand *inputBand = inputDataset->GetRasterBand(bandIdx);
        GDALRasterBand *outputBand = outputDataset->GetRasterBand(bandIdx);

        std::vector<float> buffer(xSize);
        for (int y = 0; y < ySize; ++y) {
            inputBand->RasterIO(GF_Read, xOffset, yOffset + y, xSize, 1, buffer.data(), xSize, 1, GDT_Float32, 0, 0);
            outputBand->RasterIO(GF_Write, 0, y, xSize, 1, buffer.data(), xSize, 1, GDT_Float32, 0, 0);
        }
    }

    // Close datasets
    GDALClose(inputDataset);
    GDALClose(outputDataset);

    std::cout << "Clipping completed successfully. Output saved to: " << outputFile << std::endl;
    return true;
}





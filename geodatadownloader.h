#ifndef GEODATADOWNLOADER_H
#define GEODATADOWNLOADER_H

#include <vector>
#include <string>
#include <gdal_priv.h>

class GeoDataDownloader
{
public:
    GeoDataDownloader();
    std::vector<std::vector<double>> fetchDEMData(double minX, double minY, double maxX, double maxY);
    std::vector<std::vector<double>> readGeoTiffToVector(const std::string &filePath);
    bool clipGeoTiffToBoundingBox(const std::string &inputFile, const std::string &outputFile, double minX, double minY, double maxX, double maxY);
    void computeFlowDirection(GDALDataset* demDataset, const char* outputFilename);

private:
    std::vector<std::vector<double>> demData;
    double pixelWidth, pixelHeight;
    double minX, minY, maxX, maxY;
};

#endif // GEODATADOWNLOADER_H

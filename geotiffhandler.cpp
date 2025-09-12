#include "geotiffhandler.h"
#include <stdexcept>
#include <algorithm>

GeoTiffHandler::GeoTiffHandler(const std::string& filename)
    : filename_(filename), dataset_(nullptr), width_(0), height_(0), bands_(0)
{
    GDALAllRegister();
    dataset_ = (GDALDataset*) GDALOpen(filename.c_str(), GA_ReadOnly);
    if (!dataset_) {
        throw std::runtime_error("Failed to open GeoTIFF: " + filename);
    }

    width_  = dataset_->GetRasterXSize();
    height_ = dataset_->GetRasterYSize();
    bands_  = dataset_->GetRasterCount();

    // Read first band for simplicity
    GDALRasterBand* band = dataset_->GetRasterBand(1);
    data_.resize(width_ * height_);
    CPLErr err = band->RasterIO(GF_Read, 0, 0, width_, height_,
                                data_.data(), width_, height_,
                                GDT_Float32, 0, 0);
    if (err != CE_None) {
        GDALClose(dataset_);
        throw std::runtime_error("Error reading raster data");
    }
}

GeoTiffHandler::~GeoTiffHandler() {
    if (dataset_) {
        GDALClose(dataset_);
    }
}

int GeoTiffHandler::width() const { return width_; }
int GeoTiffHandler::height() const { return height_; }
int GeoTiffHandler::bands() const { return bands_; }
const std::vector<float>& GeoTiffHandler::data() const { return data_; }

float GeoTiffHandler::minValue() const {
    return *std::min_element(data_.begin(), data_.end());
}

float GeoTiffHandler::maxValue() const {
    return *std::max_element(data_.begin(), data_.end());
}

double GeoTiffHandler::getGeoTransform(int idx) const {
    double gt[6];
    if (dataset_->GetGeoTransform(gt) == CE_None) {
        return gt[idx];
    }
    throw std::runtime_error("Failed to get GeoTransform");
}

void GeoTiffHandler::normalize() {
    float minVal = minValue();
    float maxVal = maxValue();
    for (auto& v : data_) {
        v = (v - minVal) / (maxVal - minVal);
    }
}

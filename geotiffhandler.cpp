#include "geotiffhandler.h"
#include <stdexcept>
#include <algorithm>
#include <utility> // for std::pair
#include <fstream>
#include <sstream>
#include <iomanip>
#include <set>       // for visited tracking in drainsToD4
#include <queue>

static const int di[4] = {  1, -1,  0,  0 };
static const int dj[4] = {  0,  0,  1, -1 };

GeoTiffHandler::GeoTiffHandler(const std::string& filename)
    : filename_(filename), dataset_(nullptr), width_(0), height_(0), bands_(0),
    dx_(0.0), dy_(0.0)
{
    GDALAllRegister();
    dataset_ = (GDALDataset*) GDALOpen(filename.c_str(), GA_ReadOnly);
    if (!dataset_) {
        throw std::runtime_error("Failed to open GeoTIFF: " + filename);
    }

    width_  = dataset_->GetRasterXSize();
    height_ = dataset_->GetRasterYSize();
    bands_  = dataset_->GetRasterCount();

    // Read first band
    GDALRasterBand* band = dataset_->GetRasterBand(1);
    data_.resize(width_ * height_);
    CPLErr err = band->RasterIO(GF_Read, 0, 0, width_, height_,
                                data_.data(), width_, height_,
                                GDT_Float32, 0, 0);
    if (err != CE_None) {
        GDALClose(dataset_);
        throw std::runtime_error("Error reading raster data");
    }

    // Fill 2D data
    data_2d_.resize(width_, std::vector<double>(height_));
    for (int j = 0; j < height_; ++j) {
        for (int i = 0; i < width_; ++i) {
            data_2d_[i][j] = static_cast<double>(data_[j * width_ + i]);
        }
    }

    // Extract geotransform for dx, dy and coordinate arrays
    double gt[6];
    if (dataset_->GetGeoTransform(gt) == CE_None) {
        dx_ = gt[1];   // pixel size in x
        dy_ = gt[5];   // pixel size in y (often negative)

        // X coordinates
        x_.resize(width_);
        for (int i = 0; i < width_; ++i) {
            x_[i] = gt[0] + i * dx_;
        }

        // Y coordinates
        y_.resize(height_);
        for (int j = 0; j < height_; ++j) {
            y_[j] = gt[3] + j * dy_;
        }
    }
}

GeoTiffHandler::GeoTiffHandler(int width, int height)
    : filename_(""), dataset_(nullptr),
    width_(width), height_(height), bands_(1),
    dx_(0.0), dy_(0.0)
{
    data_.resize(width * height, 0.0f);
    data_2d_.assign(width, std::vector<double>(height, 0.0));
}

// Copy constructor
GeoTiffHandler::GeoTiffHandler(const GeoTiffHandler& other)
    : filename_(""), dataset_(nullptr),
    width_(other.width_), height_(other.height_), bands_(other.bands_),
    data_(other.data_), data_2d_(other.data_2d_),
    x_(other.x_), y_(other.y_),
    dx_(other.dx_), dy_(other.dy_)
{
    // dataset_ deliberately left null (we do not duplicate GDAL handles)
}

// Copy assignment
GeoTiffHandler& GeoTiffHandler::operator=(const GeoTiffHandler& other) {
    if (this != &other) {
        filename_.clear();
        dataset_ = nullptr; // do not copy GDAL handle
        width_   = other.width_;
        height_  = other.height_;
        bands_   = other.bands_;
        data_    = other.data_;
        data_2d_ = other.data_2d_;
        x_       = other.x_;
        y_       = other.y_;
        dx_      = other.dx_;
        dy_      = other.dy_;
    }
    return *this;
}


GeoTiffHandler::~GeoTiffHandler() {
    if (dataset_) {
        GDALClose(dataset_);
    }
}

int GeoTiffHandler::width() const { return width_; }
int GeoTiffHandler::height() const { return height_; }
int GeoTiffHandler::bands() const { return bands_; }
const std::vector<float>& GeoTiffHandler::data1D() const { return data_; }
const std::vector<std::vector<double>>& GeoTiffHandler::data2D() const { return data_2d_; }

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
    for (int j = 0; j < height_; ++j) {
        for (int i = 0; i < width_; ++i) {
            data_2d_[i][j] = (data_2d_[i][j] - minVal) / (maxVal - minVal);
        }
    }
}

// ---- Getters and setters ----
const std::vector<double>& GeoTiffHandler::x() const { return x_; }
void GeoTiffHandler::setX(const std::vector<double>& x) { x_ = x; }

const std::vector<double>& GeoTiffHandler::y() const { return y_; }
void GeoTiffHandler::setY(const std::vector<double>& y) { y_ = y; }

double GeoTiffHandler::dx() const { return dx_; }
void GeoTiffHandler::setDx(double dx) { dx_ = dx; }

double GeoTiffHandler::dy() const { return dy_; }
void GeoTiffHandler::setDy(double dy) { dy_ = dy; }


#include <cmath>
#include <stdexcept>

double GeoTiffHandler::valueAt(double xCoord, double yCoord) const {
    if (x_.empty() || y_.empty())
        throw std::runtime_error("Coordinate arrays not initialized.");

    // Convert world coords to fractional indices
    double col = (xCoord - (x_.front())) / dx_;
    double row = (yCoord - (y_.front())) / dy_;

    // Account for negative dy (north-up rasters)
    if (dy_ < 0) {
        row = (yCoord - y_.front()) / dy_; // dy negative, works out
    }

    if (col < 0 || col >= width_-1 || row < 0 || row >= height_-1) {
        throw std::out_of_range("Coordinate outside raster extent.");
    }

    int i = static_cast<int>(std::floor(col));
    int j = static_cast<int>(std::floor(row));

    double dxFrac = col - i;
    double dyFrac = row - j;

    double q11 = data_2d_[i][j];
    double q21 = data_2d_[i+1][j];
    double q12 = data_2d_[i][j+1];
    double q22 = data_2d_[i+1][j+1];

    // Bilinear interpolation
    double val =
        q11 * (1 - dxFrac) * (1 - dyFrac) +
        q21 * dxFrac       * (1 - dyFrac) +
        q12 * (1 - dxFrac) * dyFrac +
        q22 * dxFrac       * dyFrac;

    return val;
}

GeoTiffHandler GeoTiffHandler::resample(int newNx, int newNy) const {
    if (x_.empty() || y_.empty()) {
        throw std::runtime_error("Coordinate arrays not initialized.");
    }
    if (newNx <= 1 || newNy <= 1) {
        throw std::invalid_argument("Resampled grid must have at least 2x2 cells.");
    }

    // Create a "blank" handler without using GDAL file — private constructor trick
    GeoTiffHandler out(*this);  // copy metadata
    out.width_  = newNx;
    out.height_ = newNy;

    // Recompute dx, dy
    double xmin = x_.front();
    double xmax = x_.back();
    double ymin = (dy_ > 0) ? y_.front() : y_.back();
    double ymax = (dy_ > 0) ? y_.back() : y_.front();

    out.dx_ = (xmax - xmin) / (newNx - 1);
    out.dy_ = (ymax - ymin) / (newNy - 1);

    // New coordinate vectors
    out.x_.resize(newNx);
    out.y_.resize(newNy);
    for (int i = 0; i < newNx; ++i) {
        out.x_[i] = xmin + i * out.dx_;
    }
    for (int j = 0; j < newNy; ++j) {
        out.y_[j] = ymin + j * out.dy_;
    }

    // Allocate new 2D data
    out.data_2d_.assign(newNx, std::vector<double>(newNy, 0.0));

    // Fill with interpolated values
    for (int j = 0; j < newNy; ++j) {
        for (int i = 0; i < newNx; ++i) {
            out.data_2d_[i][j] = valueAt(out.x_[i], out.y_[j]);
        }
    }

    // Flatten into 1D for consistency
    out.data_.resize(newNx * newNy);
    for (int j = 0; j < newNy; ++j) {
        for (int i = 0; i < newNx; ++i) {
            out.data_[j * newNx + i] = static_cast<float>(out.data_2d_[i][j]);
        }
    }

    return out;
}

void GeoTiffHandler::saveAs(const std::string& filename) const {
    if (data_2d_.empty() || x_.empty() || y_.empty()) {
        throw std::runtime_error("No data or coordinate arrays available to save.");
    }

    // Get GDAL driver for GeoTIFF
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
    if (!driver) {
        throw std::runtime_error("GTiff driver not available.");
    }

    // Create output dataset
    GDALDataset* outDs = driver->Create(
        filename.c_str(),
        width_, height_, 1,  // cols, rows, bands
        GDT_Float32,
        nullptr
        );
    if (!outDs) {
        throw std::runtime_error("Failed to create output GeoTIFF: " + filename);
    }

    // Construct geotransform
    double gt[6] = {0};
    gt[0] = x_.front();   // top-left x
    gt[1] = dx_;          // pixel size in x
    gt[2] = 0.0;          // rotation
    gt[3] = y_.front();   // top-left y
    gt[4] = 0.0;          // rotation
    gt[5] = dy_;          // pixel size in y (usually negative for north-up)

    outDs->SetGeoTransform(gt);

    // Copy projection if available
    if (dataset_) {
        const char* proj = dataset_->GetProjectionRef();
        if (proj && strlen(proj) > 0) {
            outDs->SetProjection(proj);
        }
    }

    // Flatten data_2d into row-major order for writing
    std::vector<float> buffer(width_ * height_);
    for (int j = 0; j < height_; ++j) {
        for (int i = 0; i < width_; ++i) {
            buffer[j * width_ + i] = static_cast<float>(data_2d_[i][j]);
        }
    }

    // Write data
    GDALRasterBand* band = outDs->GetRasterBand(1);
    CPLErr err = band->RasterIO(GF_Write, 0, 0, width_, height_,
                                buffer.data(), width_, height_,
                                GDT_Float32, 0, 0);
    if (err != CE_None) {
        GDALClose(outDs);
        throw std::runtime_error("Error writing raster data to " + filename);
    }

    GDALClose(outDs);
}

std::pair<int,int> GeoTiffHandler::indicesAt(double xCoord, double yCoord) const {
    if (x_.empty() || y_.empty()) {
        throw std::runtime_error("Coordinate arrays not initialized.");
    }

    // Check bounds
    if (xCoord < x_.front() || xCoord > x_.back() ||
        (dy_ < 0 && (yCoord > y_.front() || yCoord < y_.back())) ||
        (dy_ > 0 && (yCoord < y_.front() || yCoord > y_.back()))) {
        throw std::out_of_range("Requested coordinate is outside raster extent.");
    }

    // Find nearest i
    auto itx = std::lower_bound(x_.begin(), x_.end(), xCoord);
    int i = static_cast<int>(itx - x_.begin());
    if (i > 0 && (i == width_ || (xCoord - x_[i-1] < x_[i] - xCoord))) {
        i--; // pick closer index
    }

    // Find nearest j (accounting for dy sign)
    int j;
    if (dy_ < 0) {
        // y decreases downward
        auto ity = std::lower_bound(y_.rbegin(), y_.rend(), yCoord,
                                    [](double a, double b){ return a > b; });
        j = static_cast<int>(height_ - 1 - (ity - y_.rbegin()));
    } else {
        auto ity = std::lower_bound(y_.begin(), y_.end(), yCoord);
        j = static_cast<int>(ity - y_.begin());
        if (j > 0 && (j == height_ || (yCoord - y_[j-1] < y_[j] - yCoord))) {
            j--;
        }
    }

    return {i, j};
}

#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

void GeoTiffHandler::saveAsAscii(const std::string& filename, double nodata) const {
    if (data_2d_.empty() || x_.empty() || y_.empty())
        throw std::runtime_error("No data to save to ASCII.");

    std::ofstream out(filename, std::ios::out);  // text mode, not binary
    if (!out)
        throw std::runtime_error("Failed to open ASCII file for writing: " + filename);

    // ESRI ASCII Grid header
    out << "ncols " << width_ << "\n";
    out << "nrows " << height_ << "\n";

    double xll = x_.front() - dx_ / 2.0;
    double yll = (dy_ > 0 ? y_.front() : y_.back()) - std::abs(dy_) / 2.0;

    out << "xllcorner " << xll << "\n";
    out << "yllcorner " << yll << "\n";
    out << "cellsize " << std::abs(dx_) << "\n";  // assume square pixels
    out << "NODATA_value " << nodata << "\n";

    // Write rows top → bottom (per ESRI spec)
    out << std::fixed << std::setprecision(6);
    for (int j = height_ - 1; j >= 0; --j) {
        for (int i = 0; i < width_; ++i) {
            double v = data_2d_[i][j];
            if (std::isnan(v))
                out << nodata;
            else
                out << v;
            if (i < width_ - 1) out << " ";
        }
        out << "\n";
    }
}

void GeoTiffHandler::loadFromAscii(const std::string& filename) {
    std::ifstream in(filename, std::ios::in);  // text mode
    if (!in)
        throw std::runtime_error("Failed to open ASCII file: " + filename);

    std::string key;
    double nodata = -9999.0;
    double xll, yll, cellsize;

    in >> key >> width_;
    in >> key >> height_;
    in >> key >> xll;
    in >> key >> yll;
    in >> key >> cellsize;
    in >> key >> nodata;

    dx_ = cellsize;
    dy_ = -cellsize;   // north-up assumption
    bands_ = 1;

    // Build coordinate arrays (cell centers)
    x_.resize(width_);
    y_.resize(height_);
    for (int i = 0; i < width_; ++i)
        x_[i] = xll + (i + 0.5) * dx_;
    for (int j = 0; j < height_; ++j)
        y_[j] = yll + (j + 0.5) * std::abs(dy_);

    // Allocate storage
    data_2d_.assign(width_, std::vector<double>(height_, nodata));
    data_.resize(width_ * height_);

    // Read values row by row, top → bottom
    for (int j = height_ - 1; j >= 0; --j) {
        for (int i = 0; i < width_; ++i) {
            double v;
            in >> v;
            if (!in) throw std::runtime_error("Error reading data from ASCII grid.");
            data_2d_[i][j] = (v == nodata ? std::nan("") : v);
            data_[j * width_ + i] = static_cast<float>(data_2d_[i][j]);
        }
    }
}


std::pair<int,int> GeoTiffHandler::downslopeD4(int i, int j) const {
    double z = data_2d_[i][j];
    int bestI = -1, bestJ = -1;
    double maxDrop = 0.0;

    for (int k = 0; k < 4; ++k) {
        int ni = i + di[k];
        int nj = j + dj[k];
        if (ni < 0 || ni >= width_ || nj < 0 || nj >= height_) continue;

        double dz = z - data_2d_[ni][nj];
        if (dz > maxDrop) {
            maxDrop = dz;
            bestI = ni;
            bestJ = nj;
        }
    }

    return {bestI, bestJ}; // (-1,-1) if no downslope neighbor (pit/flat)
}

bool GeoTiffHandler::drainsToD4(int i0, int j0, int itarget, int jtarget) const {
    int ci = i0, cj = j0;
    std::set<std::pair<int,int>> visited; // avoid infinite loops

    while (true) {
        if (ci == itarget && cj == jtarget) return true;
        if (visited.count({ci,cj})) return false; // loop
        visited.insert({ci,cj});

        auto [ni,nj] = downslopeD4(ci, cj);
        if (ni == -1) return false; // pit or flat → doesn’t reach target
        ci = ni;
        cj = nj;
    }
}

GeoTiffHandler GeoTiffHandler::watershedD4(int itarget, int jtarget) const {
    // Step 1. Compute flow directions
    auto idx = [&](int i, int j){ return j * width_ + i; };

    std::vector<std::pair<int,int>> downslope(width_ * height_, {-1,-1});
    std::vector<std::vector<std::vector<std::pair<int,int>>>> inflow(
        width_, std::vector<std::vector<std::pair<int,int>>>(height_));

    static const int di[4] = { 1, -1, 0, 0 };
    static const int dj[4] = { 0, 0, 1, -1 };

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            double z = data_2d_[i][j];
            int bestI = -1, bestJ = -1;
            double maxDrop = 0.0;

            for (int k = 0; k < 4; ++k) {
                int ni = i + di[k];
                int nj = j + dj[k];
                if (ni < 0 || ni >= width_ || nj < 0 || nj >= height_) continue;
                double dz = z - data_2d_[ni][nj];
                if (dz > maxDrop) {
                    maxDrop = dz;
                    bestI = ni; bestJ = nj;
                }
            }
            downslope[idx(i,j)] = {bestI, bestJ};
            if (bestI != -1) {
                inflow[bestI][bestJ].push_back({i,j});
            }
        }
    }

    // Step 2. BFS upstream from target
    std::vector<bool> visited(width_ * height_, false);
    std::queue<std::pair<int,int>> q;

    q.push({itarget, jtarget});
    visited[idx(itarget,jtarget)] = true;

    int minI = itarget, maxI = itarget;
    int minJ = jtarget, maxJ = jtarget;

    while (!q.empty()) {
        auto [ci, cj] = q.front(); q.pop();

        minI = std::min(minI, ci);
        maxI = std::max(maxI, ci);
        minJ = std::min(minJ, cj);
        maxJ = std::max(maxJ, cj);

        for (auto& nb : inflow[ci][cj]) {
            int ni = nb.first, nj = nb.second;
            if (!visited[idx(ni,nj)]) {
                visited[idx(ni,nj)] = true;
                q.push({ni,nj});
            }
        }
    }

    // Step 3. Crop to bounding box
    int newW = maxI - minI + 1;
    int newH = maxJ - minJ + 1;

    GeoTiffHandler out(newW, newH); // use in-memory constructor (not file-based)

    out.dx_ = dx_;
    out.dy_ = dy_;

    // coordinates (centers)
    out.x_.resize(newW);
    out.y_.resize(newH);
    for (int i = 0; i < newW; ++i) out.x_[i] = x_[minI] + i * dx_;
    for (int j = 0; j < newH; ++j) out.y_[j] = y_[minJ] + j * dy_;

    out.data_2d_.assign(newW, std::vector<double>(newH, std::nan("")));
    out.data_.resize(newW * newH);

    for (int i = minI; i <= maxI; ++i) {
        for (int j = minJ; j <= maxJ; ++j) {
            int ni = i - minI;
            int nj = j - minJ;
            if (visited[idx(i,j)]) {
                out.data_2d_[ni][nj] = data_2d_[i][j];
            } else {
                out.data_2d_[ni][nj] = std::nan("");
            }
            out.data_[nj * newW + ni] = static_cast<float>(out.data_2d_[ni][nj]);
        }
    }

    return out;
}

#include <tuple>
#include <limits>

std::tuple<int,int,double> GeoTiffHandler::maxCell() const {
    int bestI = -1, bestJ = -1;
    double maxVal = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            double v = data_2d_[i][j];
            if (std::isnan(v)) continue; // skip NODATA
            if (v > maxVal) {
                maxVal = v;
                bestI = i;
                bestJ = j;
            }
        }
    }
    return {bestI, bestJ, maxVal};
}

std::tuple<int,int,double> GeoTiffHandler::minCell() const {
    int bestI = -1, bestJ = -1;
    double minVal = std::numeric_limits<double>::infinity();

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            double v = data_2d_[i][j];
            if (std::isnan(v)) continue; // skip NODATA
            if (v < minVal) {
                minVal = v;
                bestI = i;
                bestJ = j;
            }
        }
    }
    return {bestI, bestJ, minVal};
}

std::pair<int,int> GeoTiffHandler::maxCellIndex() const {
    int bestI = -1, bestJ = -1;
    double maxVal = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            double v = data_2d_[i][j];
            if (std::isnan(v)) continue; // skip NODATA
            if (v > maxVal) {
                maxVal = v;
                bestI = i;
                bestJ = j;
            }
        }
    }
    return {bestI, bestJ};
}

std::pair<int,int> GeoTiffHandler::minCellIndex() const {
    int bestI = -1, bestJ = -1;
    double minVal = std::numeric_limits<double>::infinity();

    for (int i = 0; i < width_; ++i) {
        for (int j = 0; j < height_; ++j) {
            double v = data_2d_[i][j];
            if (std::isnan(v)) continue; // skip NODATA
            if (v < minVal) {
                minVal = v;
                bestI = i;
                bestJ = j;
            }
        }
    }
    return {bestI, bestJ};
}



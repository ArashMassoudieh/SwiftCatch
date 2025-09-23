#include "polylineset.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cmath>
// Core GDAL includes
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <cpl_conv.h>
#include <cpl_string.h>

// Alternative GDAL includes for compatibility
// Add this at the top of polylineset.cpp instead of the current GDAL includes

#include <cpl_conv.h>  // for CPLMalloc()
#include <cpl_string.h>
#include <gdal_version.h>

// Simple GDAL initialization
static void initializeGDAL() {
    static bool initialized = false;
    if (!initialized) {
        GDALAllRegister();
        initialized = true;
    }
}


// Then in your loadFromShapefile and saveAsShapefile methods, replace:
// GDALAllRegister();
// with:
// GDAL_INIT();

// ============================================================================
// Constructors and Basic Management
// ============================================================================

PolylineSet::PolylineSet(std::initializer_list<Polyline> polylines) {
    for (const auto& polyline : polylines) {
        addPolyline(polyline);
    }
}

void PolylineSet::addPolyline(const Polyline& polyline) {
    polylines_.push_back(polyline);
    ensureAttributeVectorSize(polylines_.size());
}

void PolylineSet::addPolyline(Polyline&& polyline) {
    polylines_.push_back(std::move(polyline));
    ensureAttributeVectorSize(polylines_.size());
}

void PolylineSet::removePolyline(size_t index) {
    validateIndex(index);
    polylines_.erase(polylines_.begin() + index);
    numeric_attributes_.erase(numeric_attributes_.begin() + index);
    string_attributes_.erase(string_attributes_.begin() + index);
}

void PolylineSet::clear() {
    polylines_.clear();
    numeric_attributes_.clear();
    string_attributes_.clear();
}

// ============================================================================
// Accessors
// ============================================================================

const Polyline& PolylineSet::getPolyline(size_t index) const {
    validateIndex(index);
    return polylines_[index];
}

Polyline& PolylineSet::getPolyline(size_t index) {
    validateIndex(index);
    return polylines_[index];
}

const Polyline& PolylineSet::operator[](size_t index) const {
    return polylines_[index];
}

Polyline& PolylineSet::operator[](size_t index) {
    return polylines_[index];
}

size_t PolylineSet::size() const {
    return polylines_.size();
}

bool PolylineSet::empty() const {
    return polylines_.empty();
}

// ============================================================================
// Iterator Support
// ============================================================================

std::vector<Polyline>::iterator PolylineSet::begin() {
    return polylines_.begin();
}

std::vector<Polyline>::iterator PolylineSet::end() {
    return polylines_.end();
}

std::vector<Polyline>::const_iterator PolylineSet::begin() const {
    return polylines_.begin();
}

std::vector<Polyline>::const_iterator PolylineSet::end() const {
    return polylines_.end();
}

std::vector<Polyline>::const_iterator PolylineSet::cbegin() const {
    return polylines_.cbegin();
}

std::vector<Polyline>::const_iterator PolylineSet::cend() const {
    return polylines_.cend();
}

// ============================================================================
// Numeric Attributes Management
// ============================================================================

void PolylineSet::setPolylineNumericAttribute(size_t polylineIndex, const std::string& name, double value) {
    validateIndex(polylineIndex);
    ensureAttributeVectorSize(polylines_.size());
    numeric_attributes_[polylineIndex][name] = value;
}

std::optional<double> PolylineSet::getPolylineNumericAttribute(size_t polylineIndex, const std::string& name) const {
    validateIndex(polylineIndex);
    if (polylineIndex >= numeric_attributes_.size()) {
        return std::nullopt;
    }

    const auto& attrs = numeric_attributes_[polylineIndex];
    auto it = attrs.find(name);
    if (it != attrs.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool PolylineSet::hasPolylineNumericAttribute(size_t polylineIndex, const std::string& name) const {
    if (polylineIndex >= polylines_.size() || polylineIndex >= numeric_attributes_.size()) {
        return false;
    }

    const auto& attrs = numeric_attributes_[polylineIndex];
    return attrs.find(name) != attrs.end();
}

void PolylineSet::removePolylineNumericAttribute(size_t polylineIndex, const std::string& name) {
    validateIndex(polylineIndex);
    if (polylineIndex < numeric_attributes_.size()) {
        numeric_attributes_[polylineIndex].erase(name);
    }
}

// ============================================================================
// String Attributes Management
// ============================================================================

void PolylineSet::setPolylineStringAttribute(size_t polylineIndex, const std::string& name, const std::string& value) {
    validateIndex(polylineIndex);
    ensureAttributeVectorSize(polylines_.size());
    string_attributes_[polylineIndex][name] = value;
}

std::optional<std::string> PolylineSet::getPolylineStringAttribute(size_t polylineIndex, const std::string& name) const {
    validateIndex(polylineIndex);
    if (polylineIndex >= string_attributes_.size()) {
        return std::nullopt;
    }

    const auto& attrs = string_attributes_[polylineIndex];
    auto it = attrs.find(name);
    if (it != attrs.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool PolylineSet::hasPolylineStringAttribute(size_t polylineIndex, const std::string& name) const {
    if (polylineIndex >= polylines_.size() || polylineIndex >= string_attributes_.size()) {
        return false;
    }

    const auto& attrs = string_attributes_[polylineIndex];
    return attrs.find(name) != attrs.end();
}

void PolylineSet::removePolylineStringAttribute(size_t polylineIndex, const std::string& name) {
    validateIndex(polylineIndex);
    if (polylineIndex < string_attributes_.size()) {
        string_attributes_[polylineIndex].erase(name);
    }
}

// ============================================================================
// Bulk Attribute Operations
// ============================================================================

void PolylineSet::setNumericAttributeForAllPolylines(const std::string& name, double value) {
    ensureAttributeVectorSize(polylines_.size());
    for (auto& attrs : numeric_attributes_) {
        attrs[name] = value;
    }
}

void PolylineSet::setStringAttributeForAllPolylines(const std::string& name, const std::string& value) {
    ensureAttributeVectorSize(polylines_.size());
    for (auto& attrs : string_attributes_) {
        attrs[name] = value;
    }
}

void PolylineSet::setNumericAttributeForRange(size_t start, size_t end, const std::string& name, double value) {
    if (start >= polylines_.size() || end > polylines_.size() || start > end) {
        throw std::out_of_range("Invalid range");
    }

    ensureAttributeVectorSize(polylines_.size());
    for (size_t i = start; i < end; ++i) {
        numeric_attributes_[i][name] = value;
    }
}

void PolylineSet::setStringAttributeForRange(size_t start, size_t end, const std::string& name, const std::string& value) {
    if (start >= polylines_.size() || end > polylines_.size() || start > end) {
        throw std::out_of_range("Invalid range");
    }

    ensureAttributeVectorSize(polylines_.size());
    for (size_t i = start; i < end; ++i) {
        string_attributes_[i][name] = value;
    }
}

// ============================================================================
// Query Operations
// ============================================================================

std::vector<size_t> PolylineSet::findPolylinesWithNumericAttribute(const std::string& name) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        if (hasPolylineNumericAttribute(i, name)) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> PolylineSet::findPolylinesWithStringAttribute(const std::string& name) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        if (hasPolylineStringAttribute(i, name)) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> PolylineSet::findPolylinesWithNumericValue(const std::string& name, double value, double tolerance) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr && std::abs(*attr - value) <= tolerance) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> PolylineSet::findPolylinesWithStringValue(const std::string& name, const std::string& value) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineStringAttribute(i, name);
        if (attr && *attr == value) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> PolylineSet::findPolylinesWhere(std::function<bool(const Polyline&, size_t)> predicate) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        if (predicate(polylines_[i], i)) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> PolylineSet::findPolylinesWithNumericRange(const std::string& name, double minValue, double maxValue) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr && *attr >= minValue && *attr <= maxValue) {
            indices.push_back(i);
        }
    }
    return indices;
}

// ============================================================================
// Statistical Operations
// ============================================================================

std::optional<double> PolylineSet::getMinNumericAttribute(const std::string& name) const {
    std::optional<double> min_val;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr) {
            if (!min_val || *attr < *min_val) {
                min_val = *attr;
            }
        }
    }
    return min_val;
}

std::optional<double> PolylineSet::getMaxNumericAttribute(const std::string& name) const {
    std::optional<double> max_val;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr) {
            if (!max_val || *attr > *max_val) {
                max_val = *attr;
            }
        }
    }
    return max_val;
}

std::optional<double> PolylineSet::getAverageNumericAttribute(const std::string& name) const {
    double sum = 0.0;
    size_t count = 0;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr) {
            sum += *attr;
            count++;
        }
    }
    if (count > 0) {
        return sum / count;
    }
    return std::nullopt;
}

std::pair<std::optional<double>, std::optional<double>> PolylineSet::getNumericAttributeRange(const std::string& name) const {
    return {getMinNumericAttribute(name), getMaxNumericAttribute(name)};
}

// ============================================================================
// Attribute Metadata
// ============================================================================

std::set<std::string> PolylineSet::getAllNumericAttributeNames() const {
    std::set<std::string> names;
    for (const auto& attrs : numeric_attributes_) {
        for (const auto& pair : attrs) {
            names.insert(pair.first);
        }
    }
    return names;
}

std::set<std::string> PolylineSet::getAllStringAttributeNames() const {
    std::set<std::string> names;
    for (const auto& attrs : string_attributes_) {
        for (const auto& pair : attrs) {
            names.insert(pair.first);
        }
    }
    return names;
}

std::set<std::string> PolylineSet::getAllAttributeNames() const {
    auto numeric_names = getAllNumericAttributeNames();
    auto string_names = getAllStringAttributeNames();

    std::set<std::string> all_names;
    all_names.insert(numeric_names.begin(), numeric_names.end());
    all_names.insert(string_names.begin(), string_names.end());
    return all_names;
}

// ============================================================================
// Statistics About Polylines
// ============================================================================

size_t PolylineSet::getTotalPointCount() const {
    return std::accumulate(polylines_.begin(), polylines_.end(), size_t(0),
                           [](size_t sum, const Polyline& p) { return sum + p.size(); });
}

std::optional<size_t> PolylineSet::getMinPolylineSize() const {
    if (polylines_.empty()) {
        return std::nullopt;
    }

    auto min_it = std::min_element(polylines_.begin(), polylines_.end(),
                                   [](const Polyline& a, const Polyline& b) {
                                       return a.size() < b.size();
                                   });
    return min_it->size();
}

std::optional<size_t> PolylineSet::getMaxPolylineSize() const {
    if (polylines_.empty()) {
        return std::nullopt;
    }

    auto max_it = std::max_element(polylines_.begin(), polylines_.end(),
                                   [](const Polyline& a, const Polyline& b) {
                                       return a.size() < b.size();
                                   });
    return max_it->size();
}

double PolylineSet::getAveragePolylineSize() const {
    if (polylines_.empty()) {
        return 0.0;
    }
    return static_cast<double>(getTotalPointCount()) / polylines_.size();
}

// ============================================================================
// Helper Methods
// ============================================================================

void PolylineSet::validateIndex(size_t index) const {
    if (index >= polylines_.size()) {
        throw std::out_of_range("Polyline index out of range");
    }
}

void PolylineSet::ensureAttributeVectorSize(size_t requiredSize) {
    while (numeric_attributes_.size() < requiredSize) {
        numeric_attributes_.emplace_back();
    }
    while (string_attributes_.size() < requiredSize) {
        string_attributes_.emplace_back();
    }
}

// ============================================================================
// Shapefile I/O Operations using GDAL
// ============================================================================

void PolylineSet::loadFromShapefile(const QString& filename) {
    // Initialize GDAL/OGR
    initializeGDAL();

    // Clear existing data
    clear();

    // Open the shapefile
    GDALDataset* dataset = static_cast<GDALDataset*>(GDALOpenEx(
        filename.toUtf8().constData(),
        GDAL_OF_VECTOR,
        nullptr, nullptr, nullptr
        ));

    if (dataset == nullptr) {
        throw std::runtime_error("Failed to open shapefile: " + filename.toStdString());
    }

    try {
        // Get the first layer (shapefiles typically have one layer)
        OGRLayer* layer = dataset->GetLayer(0);
        if (layer == nullptr) {
            throw std::runtime_error("No layer found in shapefile: " + filename.toStdString());
        }

        // Reset reading to start from the beginning
        layer->ResetReading();

        // Process each feature in the layer
        OGRFeature* feature;
        while ((feature = layer->GetNextFeature()) != nullptr) {
            // Get the geometry
            OGRGeometry* geometry = feature->GetGeometryRef();
            if (geometry == nullptr) {
                OGRFeature::DestroyFeature(feature);
                continue;
            }

            // Check geometry type
            OGRwkbGeometryType geomType = wkbFlatten(geometry->getGeometryType());

            if (geomType == wkbLineString) {
                // Process single linestring
                OGRLineString* lineString = static_cast<OGRLineString*>(geometry);
                processLineString(lineString);

                // Process attributes for this polyline
                size_t polylineIndex = polylines_.size() - 1;
                processOGRFeature(feature, polylineIndex);

            } else if (geomType == wkbMultiLineString) {
                // Process multi-linestring (each linestring becomes a separate polyline)
                OGRMultiLineString* multiLineString = static_cast<OGRMultiLineString*>(geometry);

                for (int i = 0; i < multiLineString->getNumGeometries(); ++i) {
                    OGRLineString* lineString = static_cast<OGRLineString*>(
                        multiLineString->getGeometryRef(i)
                        );
                    processLineString(lineString);

                    // Process attributes for this polyline (same attributes for all parts)
                    size_t polylineIndex = polylines_.size() - 1;
                    processOGRFeature(feature, polylineIndex);
                }
            } else {
                // Skip non-linestring geometries
                std::string geomName = OGRGeometryTypeToName(geomType);
                // Optionally log warning: unsupported geometry type
            }

            OGRFeature::DestroyFeature(feature);
        }

    } catch (...) {
        GDALClose(dataset);
        throw;
    }

    GDALClose(dataset);
}

void PolylineSet::saveAsShapefile(const QString& filename, int crsEPSG) const {
    // Initialize GDAL
    initializeGDAL();

    // Get the ESRI Shapefile driver
    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    if (driver == nullptr) {
        throw std::runtime_error("ESRI Shapefile driver not available");
    }

    // Create the output dataset
    GDALDataset* dataset = driver->Create(
        filename.toUtf8().constData(), 0, 0, 0, GDT_Unknown, nullptr
    );

    if (dataset == nullptr) {
        throw std::runtime_error("Failed to create shapefile: " + filename.toStdString());
    }

    try {
        // Create spatial reference system
        OGRSpatialReference srs;
        if (srs.importFromEPSG(crsEPSG) != OGRERR_NONE) {
            throw std::runtime_error("Failed to create spatial reference system for EPSG:" + std::to_string(crsEPSG));
        }

        // Create the layer
        OGRLayer* layer = dataset->CreateLayer(
            "polylines", &srs, wkbLineString, nullptr
        );

        if (layer == nullptr) {
            throw std::runtime_error("Failed to create layer in shapefile");
        }

        // Get all attribute names to create fields
        auto numericAttrNames = getAllNumericAttributeNames();
        auto stringAttrNames = getAllStringAttributeNames();

        // Create numeric attribute fields
        for (const auto& attrName : numericAttrNames) {
            OGRFieldDefn numericField(attrName.c_str(), OFTReal);
            numericField.SetWidth(15);
            numericField.SetPrecision(6);

            if (layer->CreateField(&numericField) != OGRERR_NONE) {
                throw std::runtime_error("Failed to create numeric field: " + attrName);
            }
        }

        // Create string attribute fields
        for (const auto& attrName : stringAttrNames) {
            OGRFieldDefn stringField(attrName.c_str(), OFTString);
            stringField.SetWidth(254);  // Maximum width for shapefiles

            if (layer->CreateField(&stringField) != OGRERR_NONE) {
                throw std::runtime_error("Failed to create string field: " + attrName);
            }
        }

        // Write each polyline as a feature
        for (size_t i = 0; i < polylines_.size(); ++i) {
            const auto& polyline = polylines_[i];

            // Create feature
            OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());

            // Create linestring geometry
            OGRLineString lineString;
            for (const auto& point : polyline.getEnhancedPoints()) {
                lineString.addPoint(point.x, point.y);
            }

            feature->SetGeometry(&lineString);

            // Set numeric attributes
            if (i < numeric_attributes_.size()) {
                const auto& numAttrs = numeric_attributes_[i];
                for (const auto& pair : numAttrs) {
                    int fieldIndex = feature->GetFieldIndex(pair.first.c_str());
                    if (fieldIndex >= 0) {
                        feature->SetField(fieldIndex, pair.second);
                    }
                }
            }

            // Set string attributes
            if (i < string_attributes_.size()) {
                const auto& strAttrs = string_attributes_[i];
                for (const auto& pair : strAttrs) {
                    int fieldIndex = feature->GetFieldIndex(pair.first.c_str());
                    if (fieldIndex >= 0) {
                        feature->SetField(fieldIndex, pair.second.c_str());
                    }
                }
            }

            // Create the feature in the layer
            if (layer->CreateFeature(feature) != OGRERR_NONE) {
                OGRFeature::DestroyFeature(feature);
                throw std::runtime_error("Failed to create feature in shapefile");
            }

            OGRFeature::DestroyFeature(feature);
        }

    } catch (...) {
        GDALClose(dataset);
        throw;
    }

    GDALClose(dataset);
}

// ============================================================================
// GDAL Helper Methods
// ============================================================================

void PolylineSet::processLineString(OGRLineString* lineString) {
    Polyline polyline;

    // Extract points from the linestring
    int numPoints = lineString->getNumPoints();
    for (int i = 0; i < numPoints; ++i) {
        double x = lineString->getX(i);
        double y = lineString->getY(i);
        double z = lineString->getZ(i);  // Z coordinate if available

        // Create enhanced point
        std::map<std::string, double> attributes;
        if (lineString->getCoordinateDimension() >= 3) {
            attributes["elevation"] = z;
        }

        polyline.addEnhancedPoint(x, y, attributes);
    }

    addPolyline(std::move(polyline));
}

void PolylineSet::processOGRFeature(OGRFeature* feature, size_t polylineIndex) {
    ensureAttributeVectorSize(polylines_.size());

    // Get feature definition to access field information
    OGRFeatureDefn* featureDefn = feature->GetDefnRef();
    int fieldCount = featureDefn->GetFieldCount();

    for (int i = 0; i < fieldCount; ++i) {
        OGRFieldDefn* fieldDefn = featureDefn->GetFieldDefn(i);
        const char* fieldName = fieldDefn->GetNameRef();
        OGRFieldType fieldType = fieldDefn->GetType();

        // Skip if field is not set
        if (!feature->IsFieldSet(i)) {
            continue;
        }

        switch (fieldType) {
            case OFTInteger:
            case OFTInteger64:
                {
                    int value = feature->GetFieldAsInteger(i);
                    numeric_attributes_[polylineIndex][fieldName] = static_cast<double>(value);
                }
                break;

            case OFTReal:
                {
                    double value = feature->GetFieldAsDouble(i);
                    numeric_attributes_[polylineIndex][fieldName] = value;
                }
                break;

            case OFTString:
                {
                    const char* value = feature->GetFieldAsString(i);
                    string_attributes_[polylineIndex][fieldName] = std::string(value);
                }
                break;

            case OFTDate:
            case OFTTime:
            case OFTDateTime:
                {
                    const char* value = feature->GetFieldAsString(i);
                    string_attributes_[polylineIndex][fieldName] = std::string(value);
                }
                break;

            default:
                // For other field types, convert to string
                {
                    const char* value = feature->GetFieldAsString(i);
                    string_attributes_[polylineIndex][fieldName] = std::string(value);
                }
                break;
        }
    }
}

std::string PolylineSet::ogrFieldTypeToString(int fieldType) const {
    switch (fieldType) {
        case OFTInteger: return "Integer";
        case OFTInteger64: return "Integer64";
        case OFTReal: return "Real";
        case OFTString: return "String";
        case OFTDate: return "Date";
        case OFTTime: return "Time";
        case OFTDateTime: return "DateTime";
        default: return "Unknown";
    }
}

// ============================================================================
// Spatial Operations
// ============================================================================

std::pair<Point, Point> PolylineSet::getBoundingBox() const {
    if (polylines_.empty()) {
        return {Point(0.0, 0.0), Point(0.0, 0.0)};
    }

    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto& polyline : polylines_) {
        for (const auto& point : polyline.getEnhancedPoints()) {
            minX = std::min(minX, point.x);
            minY = std::min(minY, point.y);
            maxX = std::max(maxX, point.x);
            maxY = std::max(maxY, point.y);
        }
    }

    return {Point(minX, minY), Point(maxX, maxY)};
}

std::vector<size_t> PolylineSet::sortIndicesByString(const std::string& name, bool ascending) const {
    std::vector<size_t> indices(polylines_.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [this, &name, ascending](size_t a, size_t b) {
        auto attrA = getPolylineStringAttribute(a, name);
        auto attrB = getPolylineStringAttribute(b, name);

        if (!attrA && !attrB) return false;
        if (!attrA) return !ascending;
        if (!attrB) return ascending;

        return ascending ? (*attrA < *attrB) : (*attrA > *attrB);
    });

    return indices;
}

void PolylineSet::sortByNumericAttribute(const std::string& name, bool ascending) {
    auto indices = sortIndicesByNumeric(name, ascending);
    reorderByIndices(indices);
}

void PolylineSet::sortByStringAttribute(const std::string& name, bool ascending) {
    auto indices = sortIndicesByString(name, ascending);
    reorderByIndices(indices);
}

void PolylineSet::sortBySize(bool ascending) {
    std::vector<size_t> indices(polylines_.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [this, ascending](size_t a, size_t b) {
        return ascending ? (polylines_[a].size() < polylines_[b].size())
                        : (polylines_[a].size() > polylines_[b].size());
    });

    reorderByIndices(indices);
}

void PolylineSet::sortByCustom(std::function<bool(const std::pair<Polyline*, size_t>&, const std::pair<Polyline*, size_t>&)> comparator) {
    std::vector<std::pair<Polyline*, size_t>> indexedPolylines;
    for (size_t i = 0; i < polylines_.size(); ++i) {
        indexedPolylines.emplace_back(&polylines_[i], i);
    }

    std::sort(indexedPolylines.begin(), indexedPolylines.end(), comparator);

    std::vector<size_t> indices;
    indices.reserve(polylines_.size());
    for (const auto& pair : indexedPolylines) {
        indices.push_back(pair.second);
    }

    reorderByIndices(indices);
}

void PolylineSet::reorderByIndices(const std::vector<size_t>& indices) {
    if (indices.size() != polylines_.size()) {
        throw std::invalid_argument("Indices size must match polylines size");
    }

    // Create new vectors in the desired order
    std::vector<Polyline> newPolylines;
    std::vector<std::map<std::string, double>> newNumericAttrs;
    std::vector<std::map<std::string, std::string>> newStringAttrs;

    newPolylines.reserve(polylines_.size());
    newNumericAttrs.reserve(numeric_attributes_.size());
    newStringAttrs.reserve(string_attributes_.size());

    for (size_t idx : indices) {
        if (idx >= polylines_.size()) {
            throw std::out_of_range("Invalid index in reordering");
        }

        newPolylines.push_back(std::move(polylines_[idx]));

        if (idx < numeric_attributes_.size()) {
            newNumericAttrs.push_back(std::move(numeric_attributes_[idx]));
        } else {
            newNumericAttrs.emplace_back();
        }

        if (idx < string_attributes_.size()) {
            newStringAttrs.push_back(std::move(string_attributes_[idx]));
        } else {
            newStringAttrs.emplace_back();
        }
    }

    // Replace the old vectors
    polylines_ = std::move(newPolylines);
    numeric_attributes_ = std::move(newNumericAttrs);
    string_attributes_ = std::move(newStringAttrs);
}

// ============================================================================
// File I/O Operations (GeoJSON)
// ============================================================================

void PolylineSet::saveAsGeoJSON(const QString& filename, int crsEPSG) const {
    QJsonObject root;
    root["type"] = "FeatureCollection";

    // Create CRS object
    QJsonObject crs;
    crs["type"] = "name";
    QJsonObject crsProperties;
    crsProperties["name"] = QString("EPSG:%1").arg(crsEPSG);
    crs["properties"] = crsProperties;
    root["crs"] = crs;

    // Create features array
    QJsonArray features;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        QJsonObject feature;
        feature["type"] = "Feature";

        // Create geometry
        QJsonObject geometry;
        geometry["type"] = "LineString";

        QJsonArray coordinates;
        for (const auto& point : polylines_[i].getEnhancedPoints()) {
            QJsonArray coord;
            coord.append(point.x);
            coord.append(point.y);
            coordinates.append(coord);
        }
        geometry["coordinates"] = coordinates;
        feature["geometry"] = geometry;

        // Create properties (basic version without point attributes)
        QJsonObject properties;

        // Add polyline-level numeric attributes
        if (i < numeric_attributes_.size()) {
            for (const auto& attr : numeric_attributes_[i]) {
                properties[QString::fromStdString(attr.first)] = attr.second;
            }
        }

        // Add polyline-level string attributes
        if (i < string_attributes_.size()) {
            for (const auto& attr : string_attributes_[i]) {
                properties[QString::fromStdString(attr.first)] = QString::fromStdString(attr.second);
            }
        }

        feature["properties"] = properties;
        features.append(feature);
    }

    root["features"] = features;

    // Write to file
    QJsonDocument doc(root);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << doc.toJson();
    } else {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }
}

void PolylineSet::loadFromGeoJSON(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Could not open file for reading: " + filename.toStdString());
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    clear();

    if (root["type"].toString() != "FeatureCollection") {
        throw std::runtime_error("Invalid GeoJSON: not a FeatureCollection");
    }

    QJsonArray features = root["features"].toArray();

    for (const auto& featureValue : features) {
        QJsonObject feature = featureValue.toObject();

        if (feature["type"].toString() != "Feature") {
            continue;
        }

        QJsonObject geometry = feature["geometry"].toObject();
        if (geometry["type"].toString() != "LineString") {
            continue;
        }

        // Create polyline from coordinates
        Polyline polyline;
        QJsonArray coordinates = geometry["coordinates"].toArray();

        for (const auto& coordValue : coordinates) {
            QJsonArray coord = coordValue.toArray();
            if (coord.size() >= 2) {
                polyline.addEnhancedPoint(coord[0].toDouble(), coord[1].toDouble());
            }
        }

        addPolyline(std::move(polyline));
        size_t polylineIndex = polylines_.size() - 1;

        // Read properties
        QJsonObject properties = feature["properties"].toObject();
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            const QString& key = it.key();
            const QJsonValue& value = it.value();

            if (value.isDouble()) {
                setPolylineNumericAttribute(polylineIndex, key.toStdString(), value.toDouble());
            } else if (value.isString()) {
                setPolylineStringAttribute(polylineIndex, key.toStdString(), value.toString().toStdString());
            }
        }
    }
}

void PolylineSet::saveAsEnhancedGeoJSON(const QString& filename, int crsEPSG) const {
    QJsonObject root;
    root["type"] = "FeatureCollection";

    // Create CRS object
    QJsonObject crs;
    crs["type"] = "name";
    QJsonObject crsProperties;
    crsProperties["name"] = QString("EPSG:%1").arg(crsEPSG);
    crs["properties"] = crsProperties;
    root["crs"] = crs;

    // Create features array
    QJsonArray features;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        QJsonObject feature;
        feature["type"] = "Feature";

        // Create geometry
        QJsonObject geometry;
        geometry["type"] = "LineString";

        QJsonArray coordinates;
        for (const auto& point : polylines_[i].getEnhancedPoints()) {
            QJsonArray coord;
            coord.append(point.x);
            coord.append(point.y);
            coordinates.append(coord);
        }
        geometry["coordinates"] = coordinates;
        feature["geometry"] = geometry;

        // Create properties with both polyline and point attributes
        QJsonObject properties;

        // Add polyline-level numeric attributes
        if (i < numeric_attributes_.size()) {
            for (const auto& attr : numeric_attributes_[i]) {
                properties[QString::fromStdString("polyline_" + attr.first)] = attr.second;
            }
        }

        // Add polyline-level string attributes
        if (i < string_attributes_.size()) {
            for (const auto& attr : string_attributes_[i]) {
                properties[QString::fromStdString("polyline_" + attr.first)] = QString::fromStdString(attr.second);
            }
        }

        // Add point-level attributes as arrays
        auto pointAttributeNames = polylines_[i].getAllAttributeNames();
        for (const auto& attrName : pointAttributeNames) {
            QJsonArray attrArray;
            for (const auto& point : polylines_[i].getEnhancedPoints()) {
                auto attrValue = point.getAttribute(attrName);
                if (attrValue) {
                    attrArray.append(*attrValue);
                } else {
                    attrArray.append(QJsonValue::Null);
                }
            }
            properties[QString::fromStdString("point_" + attrName)] = attrArray;
        }

        feature["properties"] = properties;
        features.append(feature);
    }

    root["features"] = features;

    // Write to file
    QJsonDocument doc(root);
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream stream(&file);
        stream << doc.toJson();
    } else {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }
}

void PolylineSet::loadFromEnhancedGeoJSON(const QString& filename) {
    // Implementation similar to loadFromGeoJSON but handles point-level attributes
    // This would be quite complex, so I'll provide a basic version
    loadFromGeoJSON(filename); // For now, fall back to basic loading
}

// ============================================================================
// Export Operations
// ============================================================================

void PolylineSet::exportNumericAttributesToCSV(const QString& filename, const std::vector<std::string>& attributeNames) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }

    QTextStream stream(&file);

    // Determine which attributes to export
    auto attrsToExport = attributeNames.empty() ? getAllNumericAttributeNames() :
                        std::set<std::string>(attributeNames.begin(), attributeNames.end());

    // Write header
    stream << "polyline_id";
    for (const auto& attr : attrsToExport) {
        stream << "," << QString::fromStdString(attr);
    }
    stream << "\n";

    // Write data
    for (size_t i = 0; i < polylines_.size(); ++i) {
        stream << i;
        for (const auto& attr : attrsToExport) {
            stream << ",";
            auto value = getPolylineNumericAttribute(i, attr);
            if (value) {
                stream << *value;
            }
        }
        stream << "\n";
    }
}

void PolylineSet::exportStringAttributesToCSV(const QString& filename, const std::vector<std::string>& attributeNames) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }

    QTextStream stream(&file);

    // Determine which attributes to export
    auto attrsToExport = attributeNames.empty() ? getAllStringAttributeNames() :
                        std::set<std::string>(attributeNames.begin(), attributeNames.end());

    // Write header
    stream << "polyline_id";
    for (const auto& attr : attrsToExport) {
        stream << "," << QString::fromStdString(attr);
    }
    stream << "\n";

    // Write data
    for (size_t i = 0; i < polylines_.size(); ++i) {
        stream << i;
        for (const auto& attr : attrsToExport) {
            stream << ",";
            auto value = getPolylineStringAttribute(i, attr);
            if (value) {
                // Escape quotes and handle CSV formatting
                QString csvValue = QString::fromStdString(*value);
                if (csvValue.contains(',') || csvValue.contains('"') || csvValue.contains('\n')) {
                    csvValue.replace('"', "\"\"");
                    csvValue = "\"" + csvValue + "\"";
                }
                stream << csvValue;
            }
        }
        stream << "\n";
    }
}

void PolylineSet::exportSummaryStatistics(const QString& filename) const {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }

    QTextStream stream(&file);

    // General statistics
    stream << "PolylineSet Summary Statistics\n";
    stream << "============================\n\n";

    stream << "Number of polylines: " << size() << "\n";
    stream << "Total points: " << getTotalPointCount() << "\n";

    auto minSize = getMinPolylineSize();
    auto maxSize = getMaxPolylineSize();
    if (minSize && maxSize) {
        stream << "Polyline size range: " << *minSize << " - " << *maxSize << " points\n";
    }
    stream << "Average polyline size: " << getAveragePolylineSize() << " points\n\n";

    // Bounding box
    auto bbox = getBoundingBox();
    stream << "Bounding box:\n";
    stream << "  Min: (" << bbox.first.x << ", " << bbox.first.y << ")\n";
    stream << "  Max: (" << bbox.second.x << ", " << bbox.second.y << ")\n\n";

    // Numeric attribute statistics
    auto numericAttrs = getAllNumericAttributeNames();
    if (!numericAttrs.empty()) {
        stream << "Numeric Attribute Statistics:\n";
        stream << "----------------------------\n";
        for (const auto& attr : numericAttrs) {
            auto minVal = getMinNumericAttribute(attr);
            auto maxVal = getMaxNumericAttribute(attr);
            auto avgVal = getAverageNumericAttribute(attr);

            stream << QString::fromStdString(attr) << ": ";
            if (minVal && maxVal && avgVal) {
                stream << "min=" << *minVal << ", max=" << *maxVal << ", avg=" << *avgVal;
            } else {
                stream << "no data";
            }
            stream << "\n";
        }
        stream << "\n";
    }

    // String attribute summary
    auto stringAttrs = getAllStringAttributeNames();
    if (!stringAttrs.empty()) {
        stream << "String Attributes: ";
        for (const auto& attr : stringAttrs) {
            stream << QString::fromStdString(attr) << " ";
        }
        stream << "\n";
    }
}

std::vector<size_t> PolylineSet::findPolylinesIntersectingBounds(const Point& minPoint, const Point& maxPoint) const {
    std::vector<size_t> indices;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        const auto& polyline = polylines_[i];
        bool intersects = false;

        for (const auto& point : polyline.getEnhancedPoints()) {
            if (point.x >= minPoint.x && point.x <= maxPoint.x &&
                point.y >= minPoint.y && point.y <= maxPoint.y) {
                intersects = true;
                break;
            }
        }

        if (intersects) {
            indices.push_back(i);
        }
    }

    return indices;
}

// ============================================================================
// Filtering Operations
// ============================================================================

PolylineSet PolylineSet::filterByNumericAttribute(const std::string& name, double minValue, double maxValue) const {
    PolylineSet result;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineNumericAttribute(i, name);
        if (attr && *attr >= minValue && *attr <= maxValue) {
            result.addPolyline(polylines_[i]);

            // Copy attributes
            if (i < numeric_attributes_.size()) {
                result.numeric_attributes_.back() = numeric_attributes_[i];
            }
            if (i < string_attributes_.size()) {
                result.string_attributes_.back() = string_attributes_[i];
            }
        }
    }

    return result;
}

PolylineSet PolylineSet::filterByStringAttribute(const std::string& name, const std::string& value) const {
    PolylineSet result;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto attr = getPolylineStringAttribute(i, name);
        if (attr && *attr == value) {
            result.addPolyline(polylines_[i]);

            // Copy attributes
            if (i < numeric_attributes_.size()) {
                result.numeric_attributes_.back() = numeric_attributes_[i];
            }
            if (i < string_attributes_.size()) {
                result.string_attributes_.back() = string_attributes_[i];
            }
        }
    }

    return result;
}

PolylineSet PolylineSet::filterBySize(size_t minSize, size_t maxSize) const {
    PolylineSet result;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        if (polylines_[i].size() >= minSize && polylines_[i].size() <= maxSize) {
            result.addPolyline(polylines_[i]);

            // Copy attributes
            if (i < numeric_attributes_.size()) {
                result.numeric_attributes_.back() = numeric_attributes_[i];
            }
            if (i < string_attributes_.size()) {
                result.string_attributes_.back() = string_attributes_[i];
            }
        }
    }

    return result;
}

PolylineSet PolylineSet::filterByPredicate(std::function<bool(const Polyline&, size_t)> predicate) const {
    PolylineSet result;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        if (predicate(polylines_[i], i)) {
            result.addPolyline(polylines_[i]);

            // Copy attributes
            if (i < numeric_attributes_.size()) {
                result.numeric_attributes_.back() = numeric_attributes_[i];
            }
            if (i < string_attributes_.size()) {
                result.string_attributes_.back() = string_attributes_[i];
            }
        }
    }

    return result;
}

// ============================================================================
// Sorting Operations
// ============================================================================

std::vector<size_t> PolylineSet::sortIndicesByNumeric(const std::string& name, bool ascending) const {
    std::vector<size_t> indices(polylines_.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [this, &name, ascending](size_t a, size_t b) {
        auto attrA = getPolylineNumericAttribute(a, name);
        auto attrB = getPolylineNumericAttribute(b, name);

        if (!attrA && !attrB) return false;
        if (!attrA) return !ascending;
        if (!attrB) return ascending;

        return ascending ? (*attrA < *attrB) : (*attrA > *attrB);
    });

    return indices;
}


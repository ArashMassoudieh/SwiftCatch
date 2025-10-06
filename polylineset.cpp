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
#include <cpl_conv.h>  // for CPLMalloc()
#include <cpl_string.h>
#include <gdal_version.h>
#include "geotiffhandler.h"

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
    junctions_.clear();
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

        std::map<std::string, std::string> fieldNameMapping; // original -> actual

        for (const auto& attrName : numericAttrNames) {
            // Truncate field name for shapefile (10 char limit)
            std::string truncatedName = attrName.substr(0, 10);

            std::cout << "Creating field: " << attrName << " -> " << truncatedName << std::endl;

            OGRFieldDefn numericField(truncatedName.c_str(), OFTReal);
            numericField.SetWidth(15);
            numericField.SetPrecision(6);

            if (layer->CreateField(&numericField) != OGRERR_NONE) {
                throw std::runtime_error("Failed to create numeric field: " + truncatedName);
            }

            // Store the mapping
            fieldNameMapping[attrName] = truncatedName;
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
                    // Truncate attribute name to match what was created
                    std::string fieldName = pair.first.substr(0, 10);

                    int fieldIndex = feature->GetFieldIndex(fieldName.c_str());
                    if (fieldIndex >= 0) {
                        if (std::isnan(pair.second)) {
                            feature->SetFieldNull(fieldIndex);
                        } else {
                            feature->SetField(fieldIndex, pair.second);
                        }
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

double PolylineSet::minDistanceToPoint(const Point& point) const {
    if (polylines_.empty()) {
        return std::numeric_limits<double>::infinity();
    }

    double minDistance = std::numeric_limits<double>::infinity();

    for (const auto& polyline : polylines_) {
        double distance = polyline.distanceToPoint(point);
        minDistance = std::min(minDistance, distance);
    }

    return minDistance;
}

std::string PolylineSet::getGeometryType() const {
    return "MultiLineString";
}


size_t PolylineSet::findNearestPolyline(const Point& point) const {
    if (polylines_.empty()) {
        throw std::runtime_error("No polylines in set");
    }

    double minDistance = std::numeric_limits<double>::infinity();
    size_t nearestIndex = 0;

    for (size_t i = 0; i < polylines_.size(); ++i) {
        double distance = polylines_[i].distanceToPoint(point);
        if (distance < minDistance) {
            minDistance = distance;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

void PolylineSet::calculateProjectedSlopes(const GeoTiffHandler* demPtr, const std::string& attributeName) {
    if (!demPtr) {
        throw std::runtime_error("GeoTiffHandler pointer is null");
    }

    if (polylines_.empty()) {
        return;
    }

    ensureAttributeVectorSize(polylines_.size());

    for (size_t i = 0; i < polylines_.size(); ++i) {
        const auto& polyline = polylines_[i];

        // Skip polylines with insufficient points
        if (polyline.size() < 2) {
            numeric_attributes_[i][attributeName] = std::nan("");
            continue;
        }

        // Get centroid of the polyline
        Point centroid;
        try {
            centroid = polyline.getCentroid();
        } catch (const std::exception& e) {
            numeric_attributes_[i][attributeName] = std::nan("");
            continue;
        }

        // Get first and last points to define the projection direction
        const auto& firstPoint = polyline.getEnhancedPoints().front();
        const auto& lastPoint = polyline.getEnhancedPoints().back();

        // Calculate direction vector from first to last point
        double dx_line = lastPoint.x - firstPoint.x;
        double dy_line = lastPoint.y - firstPoint.y;
        double line_length = std::sqrt(dx_line * dx_line + dy_line * dy_line);

        if (line_length == 0.0) {
            // First and last points are the same
            numeric_attributes_[i][attributeName] = 0.0;
            continue;
        }

        // Normalize direction vector
        double unit_x = dx_line / line_length;
        double unit_y = dy_line / line_length;

        // Get slope components at the centroid from DEM
        // This now returns NaN gracefully instead of throwing
        auto [slope_x, slope_y] = demPtr->slopeAtBilinear(centroid.x, centroid.y);

        // Check if slope calculation returned valid values
        if (std::isnan(slope_x) || std::isnan(slope_y)) {
            numeric_attributes_[i][attributeName] = std::nan("");
            continue;
        }

        // Project slope onto the line direction using dot product
        double projected_slope = slope_x * unit_x + slope_y * unit_y;

        // Store the result
        numeric_attributes_[i][attributeName] = projected_slope;
    }
}

// Junction management methods
const JunctionSet& PolylineSet::getJunctions() const {
    return junctions_;
}

JunctionSet& PolylineSet::getJunctions() {
    return junctions_;
}

void PolylineSet::clearJunctions() {
    junctions_.clear();
}

int PolylineSet::getJunctionCount() const {
    return junctions_.size();
}

// Main junction detection method
void PolylineSet::findJunctions(double tolerance) {
    QMap<QString, QVariant> emptyAttributes;
    findJunctionsWithAttributes(tolerance, emptyAttributes);
}

void PolylineSet::findJunctionsWithAttributes(double tolerance, const QMap<QString, QVariant>& defaultAttributes) {
    junctions_.clear();

    if (polylines_.empty()) {
        return;
    }

    // Collect all endpoints
    QVector<QPair<QPointF, QPair<size_t, bool>>> endpoints; // point, (polyline_index, is_end)

    for (size_t i = 0; i < polylines_.size(); ++i) {
        const auto& polyline = polylines_[i];
        if (polyline.size() >= 2) {
            const auto& points = polyline.getEnhancedPoints();

            // First point (beginning)
            QPointF firstPoint(points.front().x, points.front().y);
            endpoints.append({firstPoint, {i, false}});

            // Last point (end)
            QPointF lastPoint(points.back().x, points.back().y);
            endpoints.append({lastPoint, {i, true}});
        }
    }

    // Group endpoints that are within tolerance
    QVector<bool> processed(endpoints.size(), false);

    for (int i = 0; i < endpoints.size(); ++i) {
        if (processed[i]) continue;

        QVector<QPair<size_t, bool>> connectedPolylines; // (polyline_index, is_end)
        QPointF junctionLocation = endpoints[i].first;
        connectedPolylines.append(endpoints[i].second);
        processed[i] = true;

        // Find all other endpoints within tolerance
        for (int j = i + 1; j < endpoints.size(); ++j) {
            if (processed[j]) continue;

            QPointF otherPoint = endpoints[j].first;
            double distance = std::sqrt(std::pow(junctionLocation.x() - otherPoint.x(), 2) +
                                        std::pow(junctionLocation.y() - otherPoint.y(), 2));

            if (distance <= tolerance) {
                connectedPolylines.append(endpoints[j].second);
                processed[j] = true;

                // Update junction location to centroid of all coincident points
                junctionLocation = QPointF(
                    (junctionLocation.x() * connectedPolylines.size() + otherPoint.x()) / (connectedPolylines.size() + 1),
                    (junctionLocation.y() * connectedPolylines.size() + otherPoint.y()) / (connectedPolylines.size() + 1)
                    );
            }
        }

        if (connectedPolylines.size() > 0) {
            Junction junction(junctionLocation, defaultAttributes);

            // Add connected polylines (you'll need shared_ptr access to polylines)
            for (const auto& connection : connectedPolylines) {
                // Note: This assumes you have a way to get shared_ptr to polylines
                // You might need to modify this based on your polyline storage
                junction.addConnectedPolyline(std::make_shared<Polyline>(polylines_[connection.first]));
            }

            // Set additional attributes
            junction.setIntAttribute("polyline_count", connectedPolylines.size());
            junction.setStringAttribute("junction_type",
                                        connectedPolylines.size()==1 ? "headwater" : (connectedPolylines.size() == 2 ? "connection" : "branch"));

            junctions_.addJunction(std::move(junction));
        }
    }
}

// Analysis methods
QVector<int> PolylineSet::getPolylinesConnectedToJunction(int junctionIndex) const {
    if (junctionIndex < 0 || junctionIndex >= junctions_.size()) {
        return QVector<int>();
    }

    QVector<int> connectedIndices;
    const auto& connectedPolylines = junctions_[junctionIndex].getConnectedPolylines();

    // This is a simplified version - you'd need to maintain proper mapping
    // between shared_ptr<Polyline> and indices in your actual implementation
    for (size_t i = 0; i < polylines_.size(); ++i) {
        for (const auto& connectedPtr : connectedPolylines) {
            // Compare polylines (you might need a better comparison method)
            if (connectedPtr) {
                connectedIndices.append(static_cast<int>(i));
                break;
            }
        }
    }

    return connectedIndices;
}

QVector<int> PolylineSet::findJunctionsNearPolyline(int polylineIndex, double searchRadius) const {
    validateIndex(polylineIndex);

    QVector<int> nearbyJunctions;
    const auto& polyline = polylines_[polylineIndex];

    for (int i = 0; i < junctions_.size(); ++i) {
        double distance = polyline.distanceToPoint(Point(junctions_[i].x(), junctions_[i].y()));
        if (distance <= searchRadius) {
            nearbyJunctions.append(i);
        }
    }

    return nearbyJunctions;
}

// Export/Import wrapper methods
void PolylineSet::saveJunctionsAsGeoJSON(const QString& filename, int crsEPSG) const {
    junctions_.saveAsGeoJSON(filename, crsEPSG);
}

void PolylineSet::saveJunctionsAsShapefile(const QString& filename, int crsEPSG) const {
    junctions_.saveAsShapefile(filename, crsEPSG);
}

void PolylineSet::loadJunctionsFromGeoJSON(const QString& filename) {
    junctions_.loadFromGeoJSON(filename);
}

void PolylineSet::loadJunctionsFromShapefile(const QString& filename) {
    junctions_.loadFromShapefile(filename);
}

void PolylineSet::assignElevationToJunctions(const GeoTiffHandler* demPtr, const QString& attributeName) {
    junctions_.assignElevationToJunctions(demPtr, attributeName);
}

void PolylineSet::findJunctionsWithElevation(double tolerance,
                                             const GeoTiffHandler* demPtr,
                                             const QMap<QString, QVariant>& defaultAttributes) {
    if (!demPtr) {
        throw std::runtime_error("GeoTiffHandler pointer is null");
    }

    junctions_.clear();

    if (polylines_.empty()) {
        return;
    }

    // Collect all endpoints with polyline index and endpoint type
    QVector<QPair<QPointF, QPair<size_t, bool>>> endpoints; // point, (polyline_index, is_end)

    for (size_t i = 0; i < polylines_.size(); ++i) {
        const auto& polyline = polylines_[i];
        if (polyline.size() >= 2) {
            const auto& points = polyline.getEnhancedPoints();

            // First point (beginning)
            QPointF firstPoint(points.front().x, points.front().y);
            endpoints.append({firstPoint, {i, false}});

            // Last point (end)
            QPointF lastPoint(points.back().x, points.back().y);
            endpoints.append({lastPoint, {i, true}});
        }
    }

    // Group endpoints that are within tolerance
    QVector<bool> processed(endpoints.size(), false);
    int junctionId = 0;

    for (int i = 0; i < endpoints.size(); ++i) {
        if (processed[i]) continue;

        QVector<QPair<size_t, bool>> connectedPolylines; // (polyline_index, is_end)
        QPointF junctionLocation = endpoints[i].first;
        connectedPolylines.append(endpoints[i].second);
        processed[i] = true;

        // Find all other endpoints within tolerance
        for (int j = i + 1; j < endpoints.size(); ++j) {
            if (processed[j]) continue;

            QPointF otherPoint = endpoints[j].first;
            double distance = std::sqrt(std::pow(junctionLocation.x() - otherPoint.x(), 2) +
                                        std::pow(junctionLocation.y() - otherPoint.y(), 2));

            if (distance <= tolerance) {
                connectedPolylines.append(endpoints[j].second);
                processed[j] = true;

                // Update junction location to centroid of all coincident points
                junctionLocation = QPointF(
                    (junctionLocation.x() * connectedPolylines.size() + otherPoint.x()) / (connectedPolylines.size() + 1),
                    (junctionLocation.y() * connectedPolylines.size() + otherPoint.y()) / (connectedPolylines.size() + 1)
                    );
            }
        }

        if (connectedPolylines.size() > 0) {
            Junction junction(junctionLocation, defaultAttributes);

            // Get elevation from DEM
            double elevation = demPtr->valueAt(junctionLocation.x(), junctionLocation.y());
            if (!std::isnan(elevation)) {
                junction.setNumericAttribute("elevation", elevation);
            }

            // Set junction ID
            junction.setIntAttribute("id", junctionId);

            // Add connected polylines
            for (const auto& connection : connectedPolylines) {
                junction.addConnectedPolyline(std::make_shared<Polyline>(polylines_[connection.first]));
            }

            // Set additional attributes
            junction.setIntAttribute("polyline_count", connectedPolylines.size());
            junction.setStringAttribute("type",
                                        connectedPolylines.size() == 1 ? "headwater" :
                                            (connectedPolylines.size() == 2 ? "connection" : "branch"));

            junctions_.addJunction(std::move(junction));

            // Now assign upstream/downstream node IDs to connected polylines
            for (const auto& connection : connectedPolylines) {
                size_t polylineIndex = connection.first;
                bool isEnd = connection.second;

                if (isEnd) {
                    // This junction is at the end of the polyline
                    setPolylineStringAttribute(polylineIndex, "d_node",
                                               QString::number(junctionId).toStdString());
                } else {
                    // This junction is at the beginning of the polyline
                    setPolylineStringAttribute(polylineIndex, "u_node",
                                               QString::number(junctionId).toStdString());
                }
            }

            junctionId++;
        }
    }

    // Now determine upstream/downstream based on elevation for polylines that have both nodes assigned
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto upstreamNodeStr = getPolylineStringAttribute(i, "u_node");
        auto downstreamNodeStr = getPolylineStringAttribute(i, "d_node");

        if (upstreamNodeStr && downstreamNodeStr) {
            int node1Id = std::stoi(*upstreamNodeStr);
            int node2Id = std::stoi(*downstreamNodeStr);

            // Get elevations of both junctions
            double elev1 = junctions_[node1Id].getNumericAttribute("elevation", std::nan(""));
            double elev2 = junctions_[node2Id].getNumericAttribute("elevation", std::nan(""));

            // If both elevations are valid, assign based on which is higher
            if (!std::isnan(elev1) && !std::isnan(elev2)) {
                if (elev1 < elev2) {
                    // Node 1 is lower, so swap them
                    setPolylineStringAttribute(i, "u_node", QString::number(node2Id).toStdString());
                    setPolylineStringAttribute(i, "d_node", QString::number(node1Id).toStdString());
                }
                // Otherwise they're already correct (elev1 >= elev2)
            }
        }
    }
}

// Add to polylineset.cpp:
PolylineSet PolylineSet::filterByValidDEMCells(const GeoTiffHandler* demPtr, double junctionTolerance) const {
    if (!demPtr) {
        throw std::runtime_error("GeoTiffHandler pointer is null");
    }

    PolylineSet result;

    // Filter polylines based on centroid validity
    for (size_t i = 0; i < polylines_.size(); ++i) {
        const auto& polyline = polylines_[i];

        // Skip polylines with less than 2 points
        if (polyline.size() < 2) {
            continue;
        }

        // Calculate centroid
        Point centroid;
        try {
            centroid = polyline.getCentroid();
        } catch (const std::exception& e) {
            // If centroid calculation fails, skip this polyline
            continue;
        }

        // Check if centroid is on a valid DEM cell
        double centroidElevation = demPtr->valueAt(centroid.x, centroid.y);

        if (std::isnan(centroidElevation)) {
            // Centroid is on null cell, skip this polyline
            continue;
        }

        // Add valid polyline
        result.addPolyline(polylines_[i]);

        // Copy all attributes
        size_t newIndex = result.size() - 1;
        if (i < numeric_attributes_.size()) {
            result.numeric_attributes_[newIndex] = numeric_attributes_[i];
        }
        if (i < string_attributes_.size()) {
            result.string_attributes_[newIndex] = string_attributes_[i];
        }
    }

    // Recreate junctions for the filtered polylines using elevation data
    if (!result.empty()) {
        result.findJunctionsWithElevation(junctionTolerance, demPtr);
    }

    return result;
}

JunctionSet PolylineSet::findSinkJunctions() {

    recalculateFlowDirections();
    JunctionSet sinks;

    // Debug: Check if polylines have the attributes
    //int polylinesWithUpstream = 0;
    //int polylinesWithDownstream = 0;

    /*for (size_t polyIdx = 0; polyIdx < polylines_.size(); ++polyIdx) {
        auto upstreamNodeStr = getPolylineStringAttribute(polyIdx, "u_node");
        auto downstreamNodeStr = getPolylineStringAttribute(polyIdx, "d_node");

        if (upstreamNodeStr) {
            polylinesWithUpstream++;
            std::cout << "Polyline " << polyIdx << " upstream_node: " << *upstreamNodeStr << std::endl;
        }
        if (downstreamNodeStr) {
            polylinesWithDownstream++;
            std::cout << "Polyline " << polyIdx << " downstream_node: " << *downstreamNodeStr << std::endl;
        }
    }*/

    //std::cout << "Polylines with upstream_node: " << polylinesWithUpstream << std::endl;
    //std::cout << "Polylines with downstream_node: " << polylinesWithDownstream << std::endl;
    //std::cout << "Total junctions: " << junctions_.size() << std::endl;

    // Build a map of junction connections
    QMap<int, QPair<int, int>> junctionConnections; // junction_id -> (upstream_count, downstream_count)

    // Initialize all junction IDs
    for (int i = 0; i < junctions_.size(); ++i) {
        int junctionId = junctions_[i].getIntAttribute("id", -1);
        //std::cout << "Junction " << i << " has ID: " << junctionId << std::endl;
        if (junctionId >= 0) {
            junctionConnections[junctionId] = QPair<int, int>(0, 0);
        }
    }

    // Count connections for each junction
    for (size_t polyIdx = 0; polyIdx < polylines_.size(); ++polyIdx) {
        auto upstreamNodeStr = getPolylineStringAttribute(polyIdx, "u_node");
        auto downstreamNodeStr = getPolylineStringAttribute(polyIdx, "d_node");

        if (upstreamNodeStr && upstreamNodeStr->length() > 0) {
            try {
                int upstreamId = std::stoi(*upstreamNodeStr);
                if (junctionConnections.contains(upstreamId)) {
                    junctionConnections[upstreamId].first++;
                    //std::cout << "Junction " << upstreamId << " is upstream for polyline " << polyIdx << std::endl;
                }
            } catch (...) {
                std::cout << "Failed to parse upstream_node: " << *upstreamNodeStr << std::endl;
            }
        }

        if (downstreamNodeStr && downstreamNodeStr->length() > 0) {
            try {
                int downstreamId = std::stoi(*downstreamNodeStr);
                if (junctionConnections.contains(downstreamId)) {
                    junctionConnections[downstreamId].second++;
                    //std::cout << "Junction " << downstreamId << " is downstream for polyline " << polyIdx << std::endl;
                }
            } catch (...) {
                std::cout << "Failed to parse downstream_node: " << *downstreamNodeStr << std::endl;
            }
        }
    }

    // Find sinks
    for (int i = 0; i < junctions_.size(); ++i) {
        const auto& junction = junctions_[i];

        double junctionElev = junction.getNumericAttribute("elevation", std::nan(""));
        int junctionId = junction.getIntAttribute("id", -1);

        if (std::isnan(junctionElev) || junctionId < 0) {
            continue;
        }

        if (junctionConnections.contains(junctionId)) {
            int upstreamCount = junctionConnections[junctionId].first;
            int downstreamCount = junctionConnections[junctionId].second;

            //std::cout << "Junction " << junctionId << " - upstream: " << upstreamCount
            //          << ", downstream: " << downstreamCount << std::endl;

            if (downstreamCount > 0 && upstreamCount == 0) {
                sinks.addJunction(junction);
                //std::cout << "  -> Added as sink" << std::endl;
            }
        }
    }

    return sinks;
}

int PolylineSet::correctSinkJunctionElevations(double elevationOffset) {
    // Find all sink junctions
    JunctionSet sinks = findSinkJunctions();

    if (sinks.isEmpty()) {
        return 0; // No sinks to correct
    }

    int correctedCount = 0;

    // For each sink junction, calculate new elevation
    for (int sinkIdx = 0; sinkIdx < sinks.size(); ++sinkIdx) {
        const auto& sinkJunction = sinks[sinkIdx];
        int sinkJunctionId = sinkJunction.getIntAttribute("id", -1);

        if (sinkJunctionId < 0) {
            continue;
        }

        QPointF sinkLocation = sinkJunction.getLocation();

        // Find all polylines connected to this sink (as downstream)
        std::vector<int> connectedJunctionIds;

        for (size_t polyIdx = 0; polyIdx < polylines_.size(); ++polyIdx) {
            auto downstreamNodeStr = getPolylineStringAttribute(polyIdx, "d_node");

            if (downstreamNodeStr && std::stoi(*downstreamNodeStr) == sinkJunctionId) {
                // This polyline flows into the sink
                auto upstreamNodeStr = getPolylineStringAttribute(polyIdx, "u_node");
                if (upstreamNodeStr) {
                    connectedJunctionIds.push_back(std::stoi(*upstreamNodeStr));
                }
            }
        }

        if (connectedJunctionIds.empty()) {
            continue; // No connected junctions found
        }

        // Calculate distance-weighted average elevation
        double weightedElevSum = 0.0;
        double totalWeight = 0.0;

        for (int connectedId : connectedJunctionIds) {
            if (connectedId < 0 || connectedId >= junctions_.size()) {
                continue;
            }

            const auto& connectedJunction = junctions_[connectedId];
            double connectedElev = connectedJunction.getNumericAttribute("elevation", std::nan(""));

            if (std::isnan(connectedElev)) {
                continue;
            }

            // Calculate distance from sink to connected junction
            QPointF connectedLocation = connectedJunction.getLocation();
            double distance = std::sqrt(
                std::pow(sinkLocation.x() - connectedLocation.x(), 2) +
                std::pow(sinkLocation.y() - connectedLocation.y(), 2)
                );

            // Avoid division by zero
            if (distance < 1e-6) {
                distance = 1e-6;
            }

            // Weight is inverse of distance (closer junctions have more influence)
            double weight = 1.0 / distance;

            weightedElevSum += connectedElev * weight;
            totalWeight += weight;
        }

        if (totalWeight > 0) {
            double newElevation = weightedElevSum / totalWeight;

            // Subtract the elevation offset to ensure it's lower than upstream nodes
            newElevation += elevationOffset;

            // Update the elevation in the actual junctions_ member
            if (sinkJunctionId >= 0 && sinkJunctionId < junctions_.size()) {
                double oldElevation = junctions_.getJunction(sinkJunctionId).getNumericAttribute("elevation", std::nan(""));
                junctions_.getJunction(sinkJunctionId).setNumericAttribute("elevation", newElevation);
                correctedCount++;

                std::cout << "Corrected sink junction " << sinkJunctionId
                          << " elevation from " << oldElevation
                          << " to " << newElevation << std::endl;
            }
        }
    }

    return correctedCount;
}

void PolylineSet::iterativelyCorrectSinks(double elevationOffset, int maxIterations) {
    std::cout << "Starting iterative sink correction with offset=" << elevationOffset
              << ", max iterations=" << maxIterations << std::endl;

    const int STAGNATION_WINDOW = 5; // Number of iterations to check for progress
    std::vector<int> recentSinkCounts;

    for (int iter = 0; iter < maxIterations; ++iter) {
        JunctionSet sinksBefore = findSinkJunctions();
        int sinkCount = sinksBefore.size();

        std::cout << "\nIteration " << (iter + 1) << ": Found " << sinkCount << " sink junctions" << std::endl;

        if (sinkCount == 0) {
            std::cout << "SUCCESS: No more sinks to correct!" << std::endl;
            break;
        }

        // Track recent sink counts
        recentSinkCounts.push_back(sinkCount);
        if (recentSinkCounts.size() > STAGNATION_WINDOW) {
            recentSinkCounts.erase(recentSinkCounts.begin());
        }

        // Check for stagnation: all recent counts are the same
        if (recentSinkCounts.size() == STAGNATION_WINDOW) {
            bool allSame = true;
            int firstCount = recentSinkCounts[0];
            for (int count : recentSinkCounts) {
                if (count != firstCount) {
                    allSame = false;
                    break;
                }
            }

            if (allSame) {
                std::cout << "WARNING: No progress in last " << STAGNATION_WINDOW
                          << " iterations. Stopping." << std::endl;
                break;
            }
        }

        // Correct sink elevations
        int correctedCount = correctSinkJunctionElevations(elevationOffset);
        std::cout << "  Corrected " << correctedCount << " sink elevations" << std::endl;

        if (correctedCount == 0) {
            std::cout << "WARNING: No sinks could be corrected. Stopping iterations." << std::endl;
            break;
        }

        // Recalculate flow directions based on new elevations
        recalculateFlowDirections();
        std::cout << "  Recalculated flow directions" << std::endl;

        // Check if we've eliminated any sinks
        JunctionSet sinksAfter = findSinkJunctions();
        int newSinkCount = sinksAfter.size();

        std::cout << "  Eliminated " << (sinkCount - newSinkCount) << " sinks" << std::endl;
    }

    JunctionSet finalSinks = findSinkJunctions();
    std::cout << "\nFinal result: " << finalSinks.size() << " remaining sink junctions" << std::endl;
}
void PolylineSet::recalculateFlowDirections() {
    // For each polyline, check the elevations of its endpoints and assign u_node/d_node
    for (size_t i = 0; i < polylines_.size(); ++i) {
        auto upstreamNodeStr = getPolylineStringAttribute(i, "u_node");
        auto downstreamNodeStr = getPolylineStringAttribute(i, "d_node");

        // Skip if polyline doesn't have both nodes assigned
        if (!upstreamNodeStr || !downstreamNodeStr) {
            continue;
        }

        int node1Id = std::stoi(*upstreamNodeStr);
        int node2Id = std::stoi(*downstreamNodeStr);

        // Validate junction IDs
        if (node1Id < 0 || node1Id >= junctions_.size() ||
            node2Id < 0 || node2Id >= junctions_.size()) {
            continue;
        }

        // Get elevations of both junctions
        double elev1 = junctions_[node1Id].getNumericAttribute("elevation", std::nan(""));
        double elev2 = junctions_[node2Id].getNumericAttribute("elevation", std::nan(""));

        // Skip if either elevation is invalid
        if (std::isnan(elev1) || std::isnan(elev2)) {
            continue;
        }

        // Assign higher elevation as upstream, lower as downstream
        if (elev1 > elev2) {
            // Current assignment is correct (node1 higher than node2)
            // u_node should be node1, d_node should be node2
            setPolylineStringAttribute(i, "u_node", QString::number(node1Id).toStdString());
            setPolylineStringAttribute(i, "d_node", QString::number(node2Id).toStdString());
        } else if (elev2 > elev1) {
            // Need to swap (node2 higher than node1)
            // u_node should be node2, d_node should be node1
            setPolylineStringAttribute(i, "u_node", QString::number(node2Id).toStdString());
            setPolylineStringAttribute(i, "d_node", QString::number(node1Id).toStdString());
        }
        // If elevations are equal, keep current assignment
    }
}


PolylineSet PolylineSet::traceAndCorrectDownstreamPath(int startJunctionId, double elevationOffset, int maxSteps) {
    PolylineSet result;

    if (startJunctionId < 0 || startJunctionId >= junctions_.size()) {
        return result;
    }

    std::set<int> visitedJunctions;
    int currentJunctionId = startJunctionId;
    int previousJunctionId = -1;

    for (int step = 0; step < maxSteps; ++step) {
        if (visitedJunctions.count(currentJunctionId) > 0) {
            std::cout << "Cycle detected at junction " << currentJunctionId << std::endl;

            auto gradients = getDownstreamGradients(currentJunctionId);
            bool foundAlternative = false;

            std::sort(gradients.begin(), gradients.end(),
                      [](const PolylineSet::JunctionGradient& a, const PolylineSet::JunctionGradient& b) {
                          return a.gradient > b.gradient;
                      });

            for (const auto& grad : gradients) {
                if (grad.gradient > 0 && visitedJunctions.count(grad.downstreamJunctionId) == 0) {
                    std::cout << "  Found alternative path to junction " << grad.downstreamJunctionId
                              << " (gradient: " << grad.gradient << ")" << std::endl;
                    currentJunctionId = grad.downstreamJunctionId;
                    foundAlternative = true;
                    break;
                }
            }

            if (!foundAlternative) {
                std::cout << "  No alternative path available" << std::endl;
                break;
            }

            continue;
        }

        visitedJunctions.insert(currentJunctionId);

        // Get current junction
        const auto& junction = junctions_[currentJunctionId];
        QPointF location = junction.getLocation();
        double elevation = junction.getNumericAttribute("elevation", std::nan(""));

        // Add junction to result's junction set
        Junction pathJunction(location);
        pathJunction.setIntAttribute("id", currentJunctionId);
        pathJunction.setIntAttribute("sequence", step);
        if (!std::isnan(elevation)) {
            pathJunction.setNumericAttribute("elevation", elevation);
        }
        result.getJunctions().addJunction(pathJunction);

        // If we have a previous junction, create a segment polyline
        if (previousJunctionId >= 0) {
            const auto& prevJunction = junctions_[previousJunctionId];
            QPointF prevLocation = prevJunction.getLocation();
            double prevElevation = prevJunction.getNumericAttribute("elevation", std::nan(""));

            // Create segment polyline
            Polyline segment;
            segment.addPoint(prevLocation.x(), prevLocation.y());
            segment.addPoint(location.x(), location.y());

            result.addPolyline(segment);

            // Add attributes to the segment
            size_t segmentIndex = result.size() - 1;
            result.setPolylineStringAttribute(segmentIndex, "from_junc", std::to_string(previousJunctionId));
            result.setPolylineStringAttribute(segmentIndex, "to_junc", std::to_string(currentJunctionId));
            result.setPolylineStringAttribute(segmentIndex, "id", std::to_string(segmentIndex));

            if (!std::isnan(prevElevation) && !std::isnan(elevation)) {
                double elevChange = prevElevation - elevation;
                result.setPolylineNumericAttribute(segmentIndex, "elev_drop", elevChange);

                // Calculate distance and gradient
                double dx = location.x() - prevLocation.x();
                double dy = location.y() - prevLocation.y();
                double distance = std::sqrt(dx * dx + dy * dy);

                if (distance > 0) {
                    result.setPolylineNumericAttribute(segmentIndex, "length", distance);
                    result.setPolylineNumericAttribute(segmentIndex, "gradient", elevChange / distance);
                }
            }
        }

        previousJunctionId = currentJunctionId;

        // Find steepest downstream gradient
        auto steepest = findSteepestDownstreamGradient(currentJunctionId);

        if (!steepest.has_value() || steepest->gradient <= 0) {
            std::cout << "Sink found at junction " << currentJunctionId << ", correcting..." << std::endl;

            bool corrected = correctSinkByGradientAdjustment(currentJunctionId, elevationOffset);

            if (corrected) {
                recalculateFlowDirections();
                steepest = findSteepestDownstreamGradient(currentJunctionId);
            }

            if (!steepest.has_value() || steepest->gradient <= 0) {
                std::cout << "Could not create valid downstream path. Path ends here." << std::endl;
                break;
            }
        }

        currentJunctionId = steepest->downstreamJunctionId;
    }

    std::cout << "Path traced with " << result.getJunctions().size() << " junctions and "
              << result.size() << " segments" << std::endl;

    return result;
}


// 1. Get all downstream junctions and their gradients from a given junction
std::vector<PolylineSet::JunctionGradient> PolylineSet::getDownstreamGradients(int junctionId) const {
    std::vector<JunctionGradient> gradients;

    if (junctionId < 0 || junctionId >= junctions_.size()) {
        return gradients;
    }

    double sourceElev = junctions_[junctionId].getNumericAttribute("elevation", std::nan(""));
    if (std::isnan(sourceElev)) {
        return gradients;
    }

    QPointF sourceLocation = junctions_[junctionId].getLocation();

    // Find all polylines where this junction is the upstream node
    for (size_t polyIdx = 0; polyIdx < polylines_.size(); ++polyIdx) {
        auto upstreamNodeStr = getPolylineStringAttribute(polyIdx, "u_node");
        if (!upstreamNodeStr || std::stoi(*upstreamNodeStr) != junctionId) {
            continue;
        }

        auto downstreamNodeStr = getPolylineStringAttribute(polyIdx, "d_node");
        if (!downstreamNodeStr) {
            continue;
        }

        int downstreamId = std::stoi(*downstreamNodeStr);
        if (downstreamId < 0 || downstreamId >= junctions_.size()) {
            continue;
        }

        double downstreamElev = junctions_[downstreamId].getNumericAttribute("elevation", std::nan(""));
        if (std::isnan(downstreamElev)) {
            continue;
        }

        QPointF downstreamLocation = junctions_[downstreamId].getLocation();
        double distance = std::sqrt(
            std::pow(sourceLocation.x() - downstreamLocation.x(), 2) +
            std::pow(sourceLocation.y() - downstreamLocation.y(), 2)
            );

        if (distance < 1e-6) {
            distance = 1e-6;
        }

        // Gradient = elevation change / distance (positive = downhill)
        double gradient = (sourceElev - downstreamElev) / distance;

        JunctionGradient jg;
        jg.downstreamJunctionId = downstreamId;
        jg.gradient = gradient;
        jg.polylineIndex = polyIdx;
        gradients.push_back(jg);
    }

    return gradients;
}

// 2. Find the steepest downhill gradient
std::optional<PolylineSet::JunctionGradient> PolylineSet::findSteepestDownstreamGradient(int junctionId) const {
    auto gradients = getDownstreamGradients(junctionId);

    if (gradients.empty()) {
        return std::nullopt;
    }

    auto maxGradient = std::max_element(gradients.begin(), gradients.end(),
                                        [](const JunctionGradient& a, const JunctionGradient& b) {
                                            return a.gradient < b.gradient;
                                        });

    return *maxGradient;
}

// 3. Check if a junction is a sink (no positive downstream gradients)
bool PolylineSet::isSink(int junctionId) const {
    auto steepest = findSteepestDownstreamGradient(junctionId);

    // If no downstream connections or all gradients are negative/uphill, it's a sink
    return !steepest.has_value() || steepest->gradient <= 0;
}

// 4. Correct a single sink by adjusting elevations
bool PolylineSet::correctSinkByGradientAdjustment(int sinkJunctionId, double elevationOffset) {
    if (!isSink(sinkJunctionId)) {
        return false;
    }

    // Find all UPSTREAM junctions (where sink is the downstream node)
    std::vector<std::pair<int, double>> upstreamJunctions; // (junctionId, elevation)

    double sinkElev = junctions_[sinkJunctionId].getNumericAttribute("elevation", std::nan(""));
    if (std::isnan(sinkElev)) {
        return false;
    }

    QPointF sinkLocation = junctions_[sinkJunctionId].getLocation();

    for (size_t polyIdx = 0; polyIdx < polylines_.size(); ++polyIdx) {
        auto downstreamNodeStr = getPolylineStringAttribute(polyIdx, "d_node");
        if (!downstreamNodeStr || std::stoi(*downstreamNodeStr) != sinkJunctionId) {
            continue;
        }

        auto upstreamNodeStr = getPolylineStringAttribute(polyIdx, "u_node");
        if (!upstreamNodeStr) {
            continue;
        }

        int upstreamId = std::stoi(*upstreamNodeStr);
        if (upstreamId < 0 || upstreamId >= junctions_.size()) {
            continue;
        }

        double upstreamElev = junctions_[upstreamId].getNumericAttribute("elevation", std::nan(""));
        if (std::isnan(upstreamElev)) {
            continue;
        }

        upstreamJunctions.push_back({upstreamId, upstreamElev});
    }

    if (upstreamJunctions.empty()) {
        std::cout << "  No upstream junctions found for sink " << sinkJunctionId << std::endl;
        return false;
    }

    // Find the upstream junction with LOWEST elevation
    auto lowestUpstream = std::min_element(upstreamJunctions.begin(), upstreamJunctions.end(),
                                           [](const auto& a, const auto& b) {
                                               return a.second < b.second;
                                           });

    int targetUpstreamId = lowestUpstream->first;
    double targetElev = lowestUpstream->second;

    std::cout << "  Sink " << sinkJunctionId << " at " << sinkElev
              << "m, lowest upstream " << targetUpstreamId << " at " << targetElev << "m" << std::endl;

    // Calculate adjustment
    double deltaZ = targetElev - sinkElev;
    double adjustment = deltaZ / 2.0 + elevationOffset / 2.0;

    double newSinkElev = sinkElev + 2*adjustment;
    double newTargetElev = targetElev - 0*adjustment;

    std::cout << "  Adjusting: sink " << sinkElev << " -> " << newSinkElev
              << ", target " << targetElev << " -> " << newTargetElev << std::endl;

    junctions_.getJunction(sinkJunctionId).setNumericAttribute("elevation", newSinkElev);
    junctions_.getJunction(targetUpstreamId).setNumericAttribute("elevation", newTargetElev);

    return true;
}
// 5. Main algorithm - traverse from highest to lowest
void PolylineSet::correctSinksByTopologicalTraversal(double elevationOffset, int maxIterations) {
    std::cout << "Starting topological sink correction with offset=" << elevationOffset << std::endl;

    for (int iter = 0; iter < maxIterations; ++iter) {
        // Sort junctions by elevation (highest first)
        std::vector<int> junctionsByElevation;
        for (int i = 0; i < junctions_.size(); ++i) {
            double elev = junctions_[i].getNumericAttribute("elevation", std::nan(""));
            if (!std::isnan(elev)) {
                junctionsByElevation.push_back(i);
            }
        }

        std::sort(junctionsByElevation.begin(), junctionsByElevation.end(),
                  [this](int a, int b) {
                      double elevA = junctions_[a].getNumericAttribute("elevation", std::nan(""));
                      double elevB = junctions_[b].getNumericAttribute("elevation", std::nan(""));
                      return elevA > elevB;
                  });

        // Process each junction from highest to lowest
        int correctedCount = 0;
        for (int junctionId : junctionsByElevation) {
            if (isSink(junctionId)) {
                if (correctSinkByGradientAdjustment(junctionId, elevationOffset)) {
                    correctedCount++;
                }
            }
        }

        std::cout << "Iteration " << (iter + 1) << ": Corrected " << correctedCount << " sinks" << std::endl;

        if (correctedCount == 0) {
            std::cout << "No more sinks to correct." << std::endl;
            break;
        }
    }

    // Count remaining sinks
    int remainingSinks = 0;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (isSink(i)) {
            remainingSinks++;
        }
    }

    std::cout << "Final result: " << remainingSinks << " remaining sinks" << std::endl;
}

// Find junction with highest elevation
int PolylineSet::getHighestElevationJunction() const {
    if (junctions_.empty()) {
        return -1;
    }

    int highestId = -1;
    double maxElev = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < junctions_.size(); ++i) {
        double elev = junctions_[i].getNumericAttribute("elevation", std::nan(""));

        if (!std::isnan(elev) && elev > maxElev) {
            maxElev = elev;
            highestId = i;
        }
    }

    return highestId;
}

// Find junction with lowest elevation
int PolylineSet::getLowestElevationJunction() const {
    if (junctions_.empty()) {
        return -1;
    }

    int lowestId = -1;
    double minElev = std::numeric_limits<double>::infinity();

    for (int i = 0; i < junctions_.size(); ++i) {
        double elev = junctions_[i].getNumericAttribute("elevation", std::nan(""));

        if (!std::isnan(elev) && elev < minElev) {
            minElev = elev;
            lowestId = i;
        }
    }

    return lowestId;
}

// Get all junctions sorted by elevation
std::vector<int> PolylineSet::getJunctionsSortedByElevation(bool ascending) const {
    std::vector<int> junctionIds;

    for (int i = 0; i < junctions_.size(); ++i) {
        double elev = junctions_[i].getNumericAttribute("elevation", std::nan(""));
        if (!std::isnan(elev)) {
            junctionIds.push_back(i);
        }
    }

    std::sort(junctionIds.begin(), junctionIds.end(),
              [this, ascending](int a, int b) {
                  double elevA = junctions_[a].getNumericAttribute("elevation", std::nan(""));
                  double elevB = junctions_[b].getNumericAttribute("elevation", std::nan(""));
                  return ascending ? (elevA < elevB) : (elevA > elevB);
              });

    return junctionIds;
}

// Get elevation range in the network
std::pair<double, double> PolylineSet::getElevationRange() const {
    double minElev = std::numeric_limits<double>::infinity();
    double maxElev = -std::numeric_limits<double>::infinity();

    for (int i = 0; i < junctions_.size(); ++i) {
        double elev = junctions_[i].getNumericAttribute("elevation", std::nan(""));

        if (!std::isnan(elev)) {
            minElev = std::min(minElev, elev);
            maxElev = std::max(maxElev, elev);
        }
    }

    return {minElev, maxElev};
}

PolylineSet PolylineSet::fromPolyline(const Polyline& polyline) {
    PolylineSet set;
    set.addPolyline(polyline);
    return set;
}

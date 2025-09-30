#include "junctionset.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QIODevice>
#include "geotiffhandler.h"

JunctionSet::JunctionSet() {}

JunctionSet::JunctionSet(std::initializer_list<Junction> junctions)
    : junctions_(junctions) {}

JunctionSet::JunctionSet(const JunctionSet& other)
    : junctions_(other.junctions_) {}

JunctionSet& JunctionSet::operator=(const JunctionSet& other) {
    if (this != &other) {
        junctions_ = other.junctions_;
    }
    return *this;
}

JunctionSet::JunctionSet(JunctionSet&& other) noexcept
    : junctions_(std::move(other.junctions_)) {}

JunctionSet& JunctionSet::operator=(JunctionSet&& other) noexcept {
    if (this != &other) {
        junctions_ = std::move(other.junctions_);
    }
    return *this;
}

// Basic container operations
void JunctionSet::addJunction(const Junction& junction) {
    junctions_.append(junction);
}

void JunctionSet::addJunction(Junction&& junction) {
    junctions_.append(std::move(junction));
}

void JunctionSet::removeJunction(int index) {
    validateIndex(index);
    junctions_.removeAt(index);
}

void JunctionSet::removeJunctionsAt(const QVector<int>& indices) {
    QVector<int> sortedIndices = indices;
    std::sort(sortedIndices.rbegin(), sortedIndices.rend()); // Sort in descending order

    for (int index : sortedIndices) {
        if (index >= 0 && index < junctions_.size()) {
            junctions_.removeAt(index);
        }
    }
}

void JunctionSet::clear() {
    junctions_.clear();
}

// Accessors
const Junction& JunctionSet::getJunction(int index) const {
    validateIndex(index);
    return junctions_[index];
}

Junction& JunctionSet::getJunction(int index) {
    validateIndex(index);
    return junctions_[index];
}

const Junction& JunctionSet::operator[](int index) const {
    return junctions_[index];
}

Junction& JunctionSet::operator[](int index) {
    return junctions_[index];
}

// Container properties
int JunctionSet::size() const {
    return junctions_.size();
}

bool JunctionSet::isEmpty() const {
    return junctions_.isEmpty();
}

bool JunctionSet::empty() const {
    return junctions_.isEmpty();
}

// Iterator support
QVector<Junction>::iterator JunctionSet::begin() {
    return junctions_.begin();
}

QVector<Junction>::iterator JunctionSet::end() {
    return junctions_.end();
}

QVector<Junction>::const_iterator JunctionSet::begin() const {
    return junctions_.begin();
}

QVector<Junction>::const_iterator JunctionSet::end() const {
    return junctions_.end();
}

QVector<Junction>::const_iterator JunctionSet::cbegin() const {
    return junctions_.cbegin();
}

QVector<Junction>::const_iterator JunctionSet::cend() const {
    return junctions_.cend();
}

// Spatial queries
QVector<int> JunctionSet::findJunctionsInRadius(const QPointF& center, double radius) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].distanceTo(center) <= radius) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsInBounds(const QRectF& bounds) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (bounds.contains(junctions_[i].getLocation())) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithinTolerance(const QPointF& point, double tolerance) const {
    return findJunctionsInRadius(point, tolerance);
}

int JunctionSet::findNearestJunction(const QPointF& point) const {
    if (junctions_.isEmpty()) {
        return -1;
    }

    int nearestIndex = 0;
    double minDistance = junctions_[0].distanceTo(point);

    for (int i = 1; i < junctions_.size(); ++i) {
        double distance = junctions_[i].distanceTo(point);
        if (distance < minDistance) {
            minDistance = distance;
            nearestIndex = i;
        }
    }

    return nearestIndex;
}

QVector<int> JunctionSet::findKNearestJunctions(const QPointF& point, int k) const {
    auto indices = sortIndicesByDistance(point, true);
    return indices.mid(0, qMin(k, indices.size()));
}

// Junction analysis
QVector<int> JunctionSet::findJunctionsWithConnectionCount(int count) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].getConnectionCount() == count) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithMinConnections(int minCount) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].getConnectionCount() >= minCount) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithMaxConnections(int maxCount) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].getConnectionCount() <= maxCount) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findIsolatedJunctions() const {
    return findJunctionsWithConnectionCount(0);
}

QVector<int> JunctionSet::findEndJunctions() const {
    return findJunctionsWithConnectionCount(1);
}

QVector<int> JunctionSet::findBranchJunctions() const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].getConnectionCount() > 2) {
            indices.append(i);
        }
    }
    return indices;
}

// Attribute-based queries implementation
QVector<int> JunctionSet::findJunctionsWithAttribute(const QString& attributeName) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].hasAttribute(attributeName)) {
            indices.append(i);
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithNumericValue(const QString& attributeName, double value, double tolerance) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].hasAttribute(attributeName)) {
            double attrValue = junctions_[i].getNumericAttribute(attributeName);
            if (std::abs(attrValue - value) <= tolerance) {
                indices.append(i);
            }
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithStringValue(const QString& attributeName, const QString& value) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].hasAttribute(attributeName)) {
            QString attrValue = junctions_[i].getStringAttribute(attributeName);
            if (attrValue == value) {
                indices.append(i);
            }
        }
    }
    return indices;
}

QVector<int> JunctionSet::findJunctionsWithNumericRange(const QString& attributeName, double minValue, double maxValue) const {
    QVector<int> indices;
    for (int i = 0; i < junctions_.size(); ++i) {
        if (junctions_[i].hasAttribute(attributeName)) {
            double attrValue = junctions_[i].getNumericAttribute(attributeName);
            if (attrValue >= minValue && attrValue <= maxValue) {
                indices.append(i);
            }
        }
    }
    return indices;
}

// Attribute metadata implementation
QStringList JunctionSet::getAllAttributeNames() const {
    QSet<QString> uniqueNames;
    for (const auto& junction : junctions_) {
        QStringList names = junction.getAttributeNames();
        for (const QString& name : names) {
            uniqueNames.insert(name);
        }
    }
    return QStringList(uniqueNames.begin(), uniqueNames.end());
}

QMap<QString, QVariant::Type> JunctionSet::getAttributeTypes() const {
    QMap<QString, QVariant::Type> types;
    QStringList allNames = getAllAttributeNames();

    for (const QString& name : allNames) {
        // Find first non-null occurrence to determine type
        for (const auto& junction : junctions_) {
            if (junction.hasAttribute(name)) {
                QVariant value = junction.getAttribute(name);
                if (!value.isNull()) {
                    types[name] = value.type();
                    break;
                }
            }
        }
    }

    return types;
}

// Bulk attribute operations implementation
void JunctionSet::setAttributeForAll(const QString& name, const QVariant& value) {
    for (auto& junction : junctions_) {
        junction.setAttribute(name, value);
    }
}

void JunctionSet::setNumericAttributeForAll(const QString& name, double value) {
    for (auto& junction : junctions_) {
        junction.setNumericAttribute(name, value);
    }
}

void JunctionSet::setStringAttributeForAll(const QString& name, const QString& value) {
    for (auto& junction : junctions_) {
        junction.setStringAttribute(name, value);
    }
}

void JunctionSet::removeAttributeFromAll(const QString& name) {
    for (auto& junction : junctions_) {
        junction.removeAttribute(name);
    }
}

// Statistics implementation
int JunctionSet::getTotalConnectionCount() const {
    int total = 0;
    for (const auto& junction : junctions_) {
        total += junction.getConnectionCount();
    }
    return total;
}

double JunctionSet::getAverageConnectionCount() const {
    if (junctions_.isEmpty()) {
        return 0.0;
    }
    return static_cast<double>(getTotalConnectionCount()) / junctions_.size();
}

int JunctionSet::getMaxConnectionCount() const {
    int maxCount = 0;
    for (const auto& junction : junctions_) {
        maxCount = std::max(maxCount, junction.getConnectionCount());
    }
    return maxCount;
}

int JunctionSet::getMinConnectionCount() const {
    if (junctions_.isEmpty()) {
        return 0;
    }

    int minCount = junctions_[0].getConnectionCount();
    for (const auto& junction : junctions_) {
        minCount = std::min(minCount, junction.getConnectionCount());
    }
    return minCount;
}

// Attribute statistics implementation
double JunctionSet::getMinNumericAttribute(const QString& name) const {
    double minVal = std::numeric_limits<double>::max();
    bool found = false;

    for (const auto& junction : junctions_) {
        if (junction.hasAttribute(name)) {
            double value = junction.getNumericAttribute(name);
            if (!std::isnan(value)) {
                minVal = std::min(minVal, value);
                found = true;
            }
        }
    }

    return found ? minVal : std::numeric_limits<double>::quiet_NaN();
}

double JunctionSet::getMaxNumericAttribute(const QString& name) const {
    double maxVal = std::numeric_limits<double>::lowest();
    bool found = false;

    for (const auto& junction : junctions_) {
        if (junction.hasAttribute(name)) {
            double value = junction.getNumericAttribute(name);
            if (!std::isnan(value)) {
                maxVal = std::max(maxVal, value);
                found = true;
            }
        }
    }

    return found ? maxVal : std::numeric_limits<double>::quiet_NaN();
}

double JunctionSet::getAverageNumericAttribute(const QString& name) const {
    double sum = 0.0;
    int count = 0;

    for (const auto& junction : junctions_) {
        if (junction.hasAttribute(name)) {
            double value = junction.getNumericAttribute(name);
            if (!std::isnan(value)) {
                sum += value;
                count++;
            }
        }
    }

    return count > 0 ? sum / count : std::numeric_limits<double>::quiet_NaN();
}

QPair<double, double> JunctionSet::getNumericAttributeRange(const QString& name) const {
    return QPair<double, double>(getMinNumericAttribute(name), getMaxNumericAttribute(name));
}

// Helper method
void JunctionSet::validateIndex(int index) const {
    if (index < 0 || index >= junctions_.size()) {
        throw std::out_of_range("Junction index out of range");
    }
}

QVector<int> JunctionSet::sortIndicesByDistance(const QPointF& point, bool ascending) const {
    QVector<int> indices(junctions_.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(), [this, &point, ascending](int a, int b) {
        double distA = junctions_[a].distanceTo(point);
        double distB = junctions_[b].distanceTo(point);
        return ascending ? (distA < distB) : (distA > distB);
    });

    return indices;
}

// Static helper methods
Junction JunctionSet::createJunctionAt(const QPointF& location) {
    return Junction(location);
}

Junction JunctionSet::createJunctionAt(double x, double y) {
    return Junction(x, y);
}

Junction JunctionSet::createJunctionWithAttributes(const QPointF& location, const QMap<QString, QVariant>& attributes) {
    return Junction(location, attributes);
}

// ============================================================================
// GeoJSON Operations
// ============================================================================

void JunctionSet::saveAsGeoJSON(const QString& filename, int crsEPSG) const {
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

    for (int i = 0; i < junctions_.size(); ++i) {
        const auto& junction = junctions_[i];

        QJsonObject feature;
        feature["type"] = "Feature";

        // Create point geometry
        QJsonObject geometry;
        geometry["type"] = "Point";

        QJsonArray coordinates;
        coordinates.append(junction.x());
        coordinates.append(junction.y());
        geometry["coordinates"] = coordinates;
        feature["geometry"] = geometry;

        // Create properties from junction attributes
        QJsonObject properties;

        // Add connection count as a property
        properties["connection_count"] = junction.getConnectionCount();

        // Add all custom attributes
        const auto& attributes = junction.getAllAttributes();
        for (auto it = attributes.begin(); it != attributes.end(); ++it) {
            const QString& key = it.key();
            const QVariant& value = it.value();

            // Convert QVariant to QJsonValue
            switch (value.type()) {
            case QVariant::Bool:
                properties[key] = value.toBool();
                break;
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                properties[key] = value.toLongLong();
                break;
            case QVariant::Double:
                properties[key] = value.toDouble();
                break;
            case QVariant::String:
                properties[key] = value.toString();
                break;
            default:
                // For other types, convert to string
                properties[key] = value.toString();
                break;
            }
        }

        feature["properties"] = properties;
        features.append(feature);
    }

    root["features"] = features;

    // Write to file
    QJsonDocument doc(root);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error("Could not open file for writing: " + filename.toStdString());
    }

    QTextStream stream(&file);
    stream << doc.toJson();
}

void JunctionSet::loadFromGeoJSON(const QString& filename) {
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
        if (geometry["type"].toString() != "Point") {
            continue; // Skip non-point geometries
        }

        // Extract coordinates
        QJsonArray coordinates = geometry["coordinates"].toArray();
        if (coordinates.size() < 2) {
            continue;
        }

        double x = coordinates[0].toDouble();
        double y = coordinates[1].toDouble();

        // Create junction
        Junction junction(x, y);

        // Read properties
        QJsonObject properties = feature["properties"].toObject();
        for (auto it = properties.begin(); it != properties.end(); ++it) {
            const QString& key = it.key();
            const QJsonValue& value = it.value();

            // Skip connection_count as it will be managed by the junction itself
            if (key == "connection_count") {
                continue;
            }

            // Convert QJsonValue to QVariant
            if (value.isBool()) {
                junction.setAttribute(key, value.toBool());
            } else if (value.isDouble()) {
                junction.setAttribute(key, value.toDouble());
            } else if (value.isString()) {
                junction.setAttribute(key, value.toString());
            } else if (value.isArray() || value.isObject()) {
                // Convert complex types to string
                QJsonDocument valueDoc(value.isArray() ? QJsonDocument(value.toArray()) : QJsonDocument(value.toObject()));
                junction.setAttribute(key, valueDoc.toJson(QJsonDocument::Compact));
            }
        }

        addJunction(std::move(junction));
    }
}

// ============================================================================
// Shapefile Operations using GDAL
// ============================================================================

#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include <cpl_conv.h>

// Simple GDAL initialization (same as in PolylineSet)
static void initializeGDAL() {
    static bool initialized = false;
    if (!initialized) {
        GDALAllRegister();
        initialized = true;
    }
}

void JunctionSet::saveAsShapefile(const QString& filename, int crsEPSG) const {
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

        // Create the layer for points
        OGRLayer* layer = dataset->CreateLayer(
            "junctions", &srs, wkbPoint, nullptr
            );

        if (layer == nullptr) {
            throw std::runtime_error("Failed to create layer in shapefile");
        }

        // Create standard fields
        OGRFieldDefn connectionCountField("conn_count", OFTInteger);
        if (layer->CreateField(&connectionCountField) != OGRERR_NONE) {
            throw std::runtime_error("Failed to create connection count field");
        }

        // Get all unique attribute names and create fields
        QStringList allAttributeNames = getAllAttributeNames();
        QMap<QString, QString> fieldNameMapping; // original -> truncated

        for (const QString& attrName : allAttributeNames) {
            // Truncate field name for shapefile (10 char limit)
            QString truncatedName = attrName.left(10);

            // Determine field type based on the first non-null occurrence
            OGRFieldType fieldType = OFTString; // Default to string
            bool foundSample = false;

            for (const auto& junction : junctions_) {
                if (junction.hasAttribute(attrName)) {
                    QVariant value = junction.getAttribute(attrName);
                    if (!value.isNull()) {
                        switch (value.type()) {
                        case QVariant::Bool:
                        case QVariant::Int:
                        case QVariant::UInt:
                        case QVariant::LongLong:
                        case QVariant::ULongLong:
                            fieldType = OFTInteger;
                            break;
                        case QVariant::Double:
                            fieldType = OFTReal;
                            break;
                        default:
                            fieldType = OFTString;
                            break;
                        }
                        foundSample = true;
                        break;
                    }
                }
            }

            // Create the field
            OGRFieldDefn field(truncatedName.toUtf8().constData(), fieldType);
            if (fieldType == OFTReal) {
                field.SetWidth(15);
                field.SetPrecision(6);
            } else if (fieldType == OFTString) {
                field.SetWidth(254);
            }

            if (layer->CreateField(&field) != OGRERR_NONE) {
                throw std::runtime_error("Failed to create field: " + truncatedName.toStdString());
            }

            fieldNameMapping[attrName] = truncatedName;
        }

        // Write each junction as a feature
        for (const auto& junction : junctions_) {
            // Create feature
            OGRFeature* feature = OGRFeature::CreateFeature(layer->GetLayerDefn());

            // Create point geometry
            OGRPoint point(junction.x(), junction.y());
            feature->SetGeometry(&point);

            // Set connection count
            feature->SetField("conn_count", junction.getConnectionCount());

            // Set custom attributes
            for (const QString& attrName : allAttributeNames) {
                if (junction.hasAttribute(attrName)) {
                    QVariant value = junction.getAttribute(attrName);
                    QString fieldName = fieldNameMapping[attrName];

                    int fieldIndex = feature->GetFieldIndex(fieldName.toUtf8().constData());
                    if (fieldIndex >= 0) {
                        if (value.isNull()) {
                            feature->SetFieldNull(fieldIndex);
                        } else {
                            switch (value.type()) {
                            case QVariant::Bool:
                                feature->SetField(fieldIndex, value.toBool() ? 1 : 0);
                                break;
                            case QVariant::Int:
                            case QVariant::UInt:
                            case QVariant::LongLong:
                            case QVariant::ULongLong:
                                feature->SetField(fieldIndex, value.toLongLong());
                                break;
                            case QVariant::Double:
                                feature->SetField(fieldIndex, value.toDouble());
                                break;
                            default:
                                feature->SetField(fieldIndex, value.toString().toUtf8().constData());
                                break;
                            }
                        }
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

void JunctionSet::loadFromShapefile(const QString& filename) {
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
        // Get the first layer
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

            if (geomType == wkbPoint) {
                // Process point geometry
                OGRPoint* point = static_cast<OGRPoint*>(geometry);
                double x = point->getX();
                double y = point->getY();

                // Create junction
                Junction junction(x, y);

                // Process attributes
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

                    // Skip connection count field (it's managed internally)
                    if (QString(fieldName) == "conn_count") {
                        continue;
                    }

                    switch (fieldType) {
                    case OFTInteger:
                    case OFTInteger64:
                    {
                        int value = feature->GetFieldAsInteger(i);
                        junction.setAttribute(QString(fieldName), value);
                    }
                    break;

                    case OFTReal:
                    {
                        double value = feature->GetFieldAsDouble(i);
                        junction.setAttribute(QString(fieldName), value);
                    }
                    break;

                    case OFTString:
                    case OFTDate:
                    case OFTTime:
                    case OFTDateTime:
                    default:
                    {
                        const char* value = feature->GetFieldAsString(i);
                        junction.setAttribute(QString(fieldName), QString(value));
                    }
                    break;
                    }
                }

                addJunction(std::move(junction));
            }
            // Skip non-point geometries

            OGRFeature::DestroyFeature(feature);
        }

    } catch (...) {
        GDALClose(dataset);
        throw;
    }

    GDALClose(dataset);
}

void JunctionSet::assignElevationToJunctions(const GeoTiffHandler* demPtr, const QString& attributeName) {
    if (!demPtr) {
        throw std::runtime_error("GeoTiffHandler pointer is null");
    }

    if (junctions_.isEmpty()) {
        return;
    }

    for (auto& junction : junctions_) {
        try {
            // Get elevation at junction location using bilinear interpolation
            double elevation = demPtr->valueAt(junction.x(), junction.y());

            // Check if elevation is valid (not NaN)
            if (!std::isnan(elevation)) {
                junction.setNumericAttribute(attributeName, elevation);
            } else {
                // Set as null/invalid if interpolation failed
                junction.setAttribute(attributeName, QVariant());
            }
        } catch (const std::exception& e) {
            // If interpolation fails, set as null/invalid
            junction.setAttribute(attributeName, QVariant());
        }
    }
}

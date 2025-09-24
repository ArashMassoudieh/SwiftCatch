#include "polyline.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <algorithm>
#include <cmath>

// ============================================================================
// EnhancedPoint Implementation
// ============================================================================

EnhancedPoint::EnhancedPoint(double xx, double yy) : x(xx), y(yy) {}

EnhancedPoint::EnhancedPoint(double xx, double yy, const std::map<std::string, double>& attrs)
    : x(xx), y(yy), attributes(attrs) {}

Point EnhancedPoint::toPoint() const {
    return Point(x, y);
}

void EnhancedPoint::setAttribute(const std::string& name, double value) {
    attributes[name] = value;
}

std::optional<double> EnhancedPoint::getAttribute(const std::string& name) const {
    auto it = attributes.find(name);
    if (it != attributes.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool EnhancedPoint::hasAttribute(const std::string& name) const {
    return attributes.find(name) != attributes.end();
}

void EnhancedPoint::removeAttribute(const std::string& name) {
    attributes.erase(name);
}

const std::map<std::string, double>& EnhancedPoint::getAttributes() const {
    return attributes;
}

// ============================================================================
// Polyline Implementation
// ============================================================================

Polyline::Polyline(std::initializer_list<EnhancedPoint> pts) {
    for (const auto& pt : pts) {
        addEnhancedPoint(pt);
    }
}

Polyline::Polyline(const Polyline& other) : Path(other), enhanced_points_(other.enhanced_points_) {}

Polyline::Polyline(Polyline&& other) noexcept
    : Path(std::move(other)), enhanced_points_(std::move(other.enhanced_points_)) {}

Polyline& Polyline::operator=(const Polyline& other) {
    if (this != &other) {
        Path::operator=(other);
        enhanced_points_ = other.enhanced_points_;
    }
    return *this;
}

Polyline& Polyline::operator=(Polyline&& other) noexcept {
    if (this != &other) {
        Path::operator=(std::move(other));
        enhanced_points_ = std::move(other.enhanced_points_);
    }
    return *this;
}

void Polyline::addEnhancedPoint(const EnhancedPoint& pt) {
    enhanced_points_.push_back(pt);
    Path::addPoint(pt.x, pt.y);  // Keep base class synchronized
}

void Polyline::addEnhancedPoint(double x, double y, const std::map<std::string, double>& attributes) {
    enhanced_points_.emplace_back(x, y, attributes);
    Path::addPoint(x, y);
}

void Polyline::addPoint(double x, double y) {
    enhanced_points_.emplace_back(x, y);
    Path::addPoint(x, y);
}

const EnhancedPoint& Polyline::getEnhancedPoint(size_t idx) const {
    if (idx >= enhanced_points_.size()) {
        throw std::out_of_range("Point index out of range");
    }
    return enhanced_points_[idx];
}

EnhancedPoint& Polyline::getEnhancedPoint(size_t idx) {
    if (idx >= enhanced_points_.size()) {
        throw std::out_of_range("Point index out of range");
    }
    return enhanced_points_[idx];
}

void Polyline::setPointAttribute(size_t idx, const std::string& name, double value) {
    if (idx >= enhanced_points_.size()) {
        throw std::out_of_range("Point index out of range");
    }
    enhanced_points_[idx].setAttribute(name, value);
}

std::optional<double> Polyline::getPointAttribute(size_t idx, const std::string& name) const {
    if (idx >= enhanced_points_.size()) {
        throw std::out_of_range("Point index out of range");
    }
    return enhanced_points_[idx].getAttribute(name);
}

void Polyline::setAttributeForAllPoints(const std::string& name, double value) {
    for (auto& pt : enhanced_points_) {
        pt.setAttribute(name, value);
    }
}

void Polyline::setAttributeForRange(size_t start, size_t end, const std::string& name, double value) {
    if (start >= enhanced_points_.size() || end > enhanced_points_.size() || start > end) {
        throw std::out_of_range("Invalid range");
    }
    for (size_t i = start; i < end; ++i) {
        enhanced_points_[i].setAttribute(name, value);
    }
}

std::vector<size_t> Polyline::findPointsWithAttribute(const std::string& name) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < enhanced_points_.size(); ++i) {
        if (enhanced_points_[i].hasAttribute(name)) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::vector<size_t> Polyline::findPointsWithAttributeValue(const std::string& name, double value, double tolerance) const {
    std::vector<size_t> indices;
    for (size_t i = 0; i < enhanced_points_.size(); ++i) {
        auto attr = enhanced_points_[i].getAttribute(name);
        if (attr && std::abs(*attr - value) <= tolerance) {
            indices.push_back(i);
        }
    }
    return indices;
}

std::optional<double> Polyline::getMinAttribute(const std::string& name) const {
    std::optional<double> min_val;
    for (const auto& pt : enhanced_points_) {
        auto attr = pt.getAttribute(name);
        if (attr) {
            if (!min_val || *attr < *min_val) {
                min_val = *attr;
            }
        }
    }
    return min_val;
}

std::optional<double> Polyline::getMaxAttribute(const std::string& name) const {
    std::optional<double> max_val;
    for (const auto& pt : enhanced_points_) {
        auto attr = pt.getAttribute(name);
        if (attr) {
            if (!max_val || *attr > *max_val) {
                max_val = *attr;
            }
        }
    }
    return max_val;
}

std::optional<double> Polyline::getAverageAttribute(const std::string& name) const {
    double sum = 0.0;
    size_t count = 0;
    for (const auto& pt : enhanced_points_) {
        auto attr = pt.getAttribute(name);
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

void Polyline::clear() {
    enhanced_points_.clear();
    Path::clear();
}

std::set<std::string> Polyline::getAllAttributeNames() const {
    std::set<std::string> names;
    for (const auto& pt : enhanced_points_) {
        for (const auto& attr : pt.getAttributes()) {
            names.insert(attr.first);
        }
    }
    return names;
}

const std::vector<EnhancedPoint>& Polyline::getEnhancedPoints() const {
    return enhanced_points_;
}

void Polyline::saveAsEnhancedGeoJSON(const QString& filename, int crsEPSG) const {
    QJsonObject root;

    // Set the type
    root["type"] = "Feature";

    // Create geometry object
    QJsonObject geometry;
    geometry["type"] = "LineString";

    // Add coordinates array
    QJsonArray coordinates;
    for (const auto& pt : enhanced_points_) {
        QJsonArray coord;
        coord.append(pt.x);
        coord.append(pt.y);
        coordinates.append(coord);
    }
    geometry["coordinates"] = coordinates;
    root["geometry"] = geometry;

    // Create properties object with point-wise attributes
    QJsonObject properties;

    // Add CRS information
    properties["crs_epsg"] = crsEPSG;

    // Collect all unique attribute names
    auto attributeNames = getAllAttributeNames();

    // Create arrays for each attribute
    for (const auto& attrName : attributeNames) {
        QJsonArray attrArray;
        for (const auto& pt : enhanced_points_) {
            auto attrValue = pt.getAttribute(attrName);
            if (attrValue) {
                attrArray.append(*attrValue);
            } else {
                attrArray.append(QJsonValue::Null);
            }
        }
        properties[QString::fromStdString(attrName)] = attrArray;
    }

    root["properties"] = properties;

    // Create CRS object
    QJsonObject crs;
    crs["type"] = "name";
    QJsonObject crsProperties;
    crsProperties["name"] = QString("EPSG:%1").arg(crsEPSG);
    crs["properties"] = crsProperties;
    root["crs"] = crs;

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

void Polyline::loadFromEnhancedGeoJSON(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        throw std::runtime_error("Could not open file for reading: " + filename.toStdString());
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonObject root = doc.object();

    // Clear existing data
    clear();

    // Check if it's a valid GeoJSON feature
    if (root["type"].toString() != "Feature") {
        throw std::runtime_error("Invalid GeoJSON: not a Feature");
    }

    QJsonObject geometry = root["geometry"].toObject();
    if (geometry["type"].toString() != "LineString") {
        throw std::runtime_error("Invalid GeoJSON: geometry is not a LineString");
    }

    // Read coordinates
    QJsonArray coordinates = geometry["coordinates"].toArray();
    std::vector<EnhancedPoint> points;
    points.reserve(coordinates.size());

    for (const auto& coordValue : coordinates) {
        QJsonArray coord = coordValue.toArray();
        if (coord.size() >= 2) {
            points.emplace_back(coord[0].toDouble(), coord[1].toDouble());
        }
    }

    // Read properties and attributes
    QJsonObject properties = root["properties"].toObject();

    for (auto it = properties.begin(); it != properties.end(); ++it) {
        const QString& key = it.key();

        // Skip special properties
        if (key == "crs_epsg") continue;

        QJsonArray attrArray = it.value().toArray();
        std::string attrName = key.toStdString();

        // Apply attributes to points
        for (int i = 0; i < std::min(int(attrArray.size()), int(points.size())); ++i) {
            if (!attrArray[i].isNull()) {
                points[i].setAttribute(attrName, attrArray[i].toDouble());
            }
        }
    }

    // Add all points to the polyline
    for (const auto& pt : points) {
        addEnhancedPoint(pt);
    }
}

// GeometryBase interface implementations
std::pair<Point, Point> Polyline::getBoundingBox() const {
    if (enhanced_points_.empty()) {
        return {Point(0.0, 0.0), Point(0.0, 0.0)};
    }

    double minX = enhanced_points_[0].x;
    double minY = enhanced_points_[0].y;
    double maxX = minX;
    double maxY = minY;

    for (const auto& point : enhanced_points_) {
        minX = std::min(minX, point.x);
        minY = std::min(minY, point.y);
        maxX = std::max(maxX, point.x);
        maxY = std::max(maxY, point.y);
    }

    return {Point(minX, minY), Point(maxX, maxY)};
}

void Polyline::saveAsGeoJSON(const QString& filename, int crsEPSG) const {
    // Use the existing enhanced GeoJSON method
    saveAsEnhancedGeoJSON(filename, crsEPSG);
}

void Polyline::loadFromGeoJSON(const QString& filename) {
    // Use the existing enhanced GeoJSON method
    loadFromEnhancedGeoJSON(filename);
}

std::string Polyline::getGeometryType() const {
    return "LineString";
}


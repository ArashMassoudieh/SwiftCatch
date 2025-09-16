#include "path.h"
#include <fstream>
#include <string>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

Path::Path(std::initializer_list<Point> pts) : points_(pts) {}

Path::Path(const Path& other) : points_(other.points_) {}

Path::Path(Path&& other) noexcept : points_(std::move(other.points_)) {}

Path& Path::operator=(const Path& other) {
    if (this != &other) {
        points_ = other.points_;
    }
    return *this;
}

Path& Path::operator=(Path&& other) noexcept {
    if (this != &other) {
        points_ = std::move(other.points_);
    }
    return *this;
}

void Path::addPoint(double x, double y) {
    points_.emplace_back(x, y);
}

const Point& Path::at(size_t idx) const {
    if (idx >= points_.size()) {
        throw std::out_of_range("Path::at - index out of range");
    }
    return points_[idx];
}

size_t Path::size() const {
    return points_.size();
}

void Path::clear() {
    points_.clear();
}


void Path::saveAsGeoJSON(const QString& filename, int crsEPSG) const {
    if (points_.size() < 2) {
        throw std::runtime_error("Path must have at least 2 points to form a LineString.");
    }

    QJsonArray coords;
    for (const auto& pt : points_) {
        QJsonArray pair;
        pair.append(pt.x);
        pair.append(pt.y);
        coords.append(pair);
    }

    QJsonObject geometry;
    geometry["type"] = "LineString";
    geometry["coordinates"] = coords;

    QJsonObject feature;
    feature["type"] = "Feature";
    feature["geometry"] = geometry;
    feature["properties"] = QJsonObject();

    QJsonObject fc;
    fc["type"] = "FeatureCollection";
    fc["features"] = QJsonArray{ feature };

    QJsonObject crs;
    crs["type"] = "name";
    crs["properties"] = QJsonObject{ {"name", QString("EPSG:%1").arg(crsEPSG)} };
    fc["crs"] = crs;

    QJsonDocument doc(fc);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open file for writing: " + filename.toStdString());
    }
    file.write(doc.toJson(QJsonDocument::Indented));
}

/// Load from GeoJSON (replaces current contents)
void Path::loadFromGeoJSON(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::runtime_error("Failed to open GeoJSON file: " + filename.toStdString());
    }

    QByteArray data = file.readAll();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError) {
        throw std::runtime_error("Failed to parse GeoJSON: " + err.errorString().toStdString());
    }

    points_.clear();

    QJsonObject root = doc.object();
    QJsonArray features = root["features"].toArray();
    if (features.isEmpty()) {
        throw std::runtime_error("GeoJSON has no features.");
    }

    QJsonObject geom = features[0].toObject()["geometry"].toObject();
    if (geom["type"].toString() != "LineString") {
        throw std::runtime_error("Only LineString geometry is supported.");
    }

    QJsonArray coords = geom["coordinates"].toArray();
    for (const auto& val : coords) {
        QJsonArray pair = val.toArray();
        if (pair.size() >= 2) {
            points_.emplace_back(pair[0].toDouble(), pair[1].toDouble());
        }
    }
}

#include "geometrybase.h"
#include "path.h"

void GeometryBase::clear() {
    // Default: do nothing
}

size_t GeometryBase::size() const {
    return 0;
}

bool GeometryBase::empty() const {
    return size() == 0;
}

std::pair<Point, Point> GeometryBase::getBoundingBox() const {
    return {Point(0.0, 0.0), Point(0.0, 0.0)};
}

size_t GeometryBase::getTotalPointCount() const {
    return size();
}

void GeometryBase::saveAsGeoJSON(const QString& filename, int crsEPSG) const {
    // Default: do nothing
    Q_UNUSED(filename)
    Q_UNUSED(crsEPSG)
}

void GeometryBase::loadFromGeoJSON(const QString& filename) {
    // Default: do nothing
    Q_UNUSED(filename)
}

std::string GeometryBase::getGeometryType() const {
    return "Unknown";
}





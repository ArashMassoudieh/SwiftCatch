#ifndef GEOMETRYBASE_H
#define GEOMETRYBASE_H

#include <QString>
#include <utility>
#include <string>

// Forward declaration
struct Point;

class GeometryBase {
public:
    virtual ~GeometryBase() = default;

    // Basic operations
    virtual void clear();
    virtual size_t size() const;
    virtual bool empty() const;

    // Spatial operations
    virtual std::pair<Point, Point> getBoundingBox() const;
    virtual size_t getTotalPointCount() const;

    // File I/O
    virtual void saveAsGeoJSON(const QString& filename, int crsEPSG = 4326) const;
    virtual void loadFromGeoJSON(const QString& filename);

    // Type identification
    virtual std::string getGeometryType() const;
};

#endif // GEOMETRYBASE_H

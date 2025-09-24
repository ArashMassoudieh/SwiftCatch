#ifndef POLYLINE_H
#define POLYLINE_H

#include "path.h"  // Include the original Path header
#include <map>
#include <string>
#include <optional>
#include <set>
#include <geometrybase.h>

/// \brief Enhanced Point with named attributes for storing additional data
struct EnhancedPoint {
    double x;
    double y;
    std::map<std::string, double> attributes;

    // Constructors
    EnhancedPoint(double xx = 0.0, double yy = 0.0);
    EnhancedPoint(double xx, double yy, const std::map<std::string, double>& attrs);

    // Convert to basic Point for compatibility with base class
    Point toPoint() const;

    // Attribute management
    void setAttribute(const std::string& name, double value);
    std::optional<double> getAttribute(const std::string& name) const;
    bool hasAttribute(const std::string& name) const;
    void removeAttribute(const std::string& name);
    const std::map<std::string, double>& getAttributes() const;
};

/// \brief Polyline class that extends Path functionality with enhanced points
class Polyline : public Path, public GeometryBase {
public:
    // Constructors
    Polyline() = default;
    explicit Polyline(std::initializer_list<EnhancedPoint> pts);

    // Rule of Five
    Polyline(const Polyline& other);
    Polyline(Polyline&& other) noexcept;
    Polyline& operator=(const Polyline& other);
    Polyline& operator=(Polyline&& other) noexcept;
    ~Polyline() = default;

    // Enhanced point management
    void addEnhancedPoint(const EnhancedPoint& pt);
    void addEnhancedPoint(double x, double y, const std::map<std::string, double>& attributes = {});

    // Override addPoint to maintain synchronization
    void addPoint(double x, double y);

    // Enhanced accessors
    const EnhancedPoint& getEnhancedPoint(size_t idx) const;
    EnhancedPoint& getEnhancedPoint(size_t idx);

    // Attribute operations on specific points
    void setPointAttribute(size_t idx, const std::string& name, double value);
    std::optional<double> getPointAttribute(size_t idx, const std::string& name) const;

    // Bulk attribute operations
    void setAttributeForAllPoints(const std::string& name, double value);
    void setAttributeForRange(size_t start, size_t end, const std::string& name, double value);

    // Query operations
    std::vector<size_t> findPointsWithAttribute(const std::string& name) const;
    std::vector<size_t> findPointsWithAttributeValue(const std::string& name, double value, double tolerance = 1e-9) const;

    // Statistical operations on attributes
    std::optional<double> getMinAttribute(const std::string& name) const;
    std::optional<double> getMaxAttribute(const std::string& name) const;
    std::optional<double> getAverageAttribute(const std::string& name) const;

    // Override clear to maintain synchronization
    void clear() override;

    // Get all unique attribute names across all points
    std::set<std::string> getAllAttributeNames() const;

    // Access to enhanced points container
    const std::vector<EnhancedPoint>& getEnhancedPoints() const;

    // Export methods
    void saveAsEnhancedGeoJSON(const QString& filename, int crsEPSG = 4326) const;
    void loadFromEnhancedGeoJSON(const QString& filename);

    size_t size() const override { return Path::size(); }
    bool empty() const override { return Path::size() == 0; }
    std::pair<Point, Point> getBoundingBox() const override;
    size_t getTotalPointCount() const override { return size(); }
    void saveAsGeoJSON(const QString& filename, int crsEPSG = 4326) const override;
    void loadFromGeoJSON(const QString& filename) override;
    std::string getGeometryType() const override;

private:
    std::vector<EnhancedPoint> enhanced_points_;
};

#endif // POLYLINE_H

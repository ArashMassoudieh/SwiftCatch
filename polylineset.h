#ifndef POLYLINESET_H
#define POLYLINESET_H

#include "polyline.h"
#include <vector>
#include <map>
#include <string>
#include <optional>
#include <set>
#include <functional>
#include <geometrybase.h>
#include <GeoDataSetInterface.h>

// Forward declarations for GDAL
class GDALDataset;
class OGRLayer;
class OGRFeature;

/// \brief Container class for managing multiple polylines with per-polyline attributes
class PolylineSet: public GeometryBase {
public:
    // Constructors
    PolylineSet() = default;
    explicit PolylineSet(std::initializer_list<Polyline> polylines);

    // Rule of Five
    PolylineSet(const PolylineSet& other) = default;
    PolylineSet(PolylineSet&& other) noexcept = default;
    PolylineSet& operator=(const PolylineSet& other) = default;
    PolylineSet& operator=(PolylineSet&& other) noexcept = default;
    ~PolylineSet() = default;

    // Polyline management
    void addPolyline(const Polyline& polyline);
    void addPolyline(Polyline&& polyline);
    void removePolyline(size_t index);
    void clear() override;

    std::string getGeometryType() const override;

    // Accessors
    const Polyline& getPolyline(size_t index) const;
    Polyline& getPolyline(size_t index);
    const Polyline& operator[](size_t index) const;
    Polyline& operator[](size_t index);
    size_t size() const override;
    bool empty() const override;

    // Iterator support
    std::vector<Polyline>::iterator begin();
    std::vector<Polyline>::iterator end();
    std::vector<Polyline>::const_iterator begin() const;
    std::vector<Polyline>::const_iterator end() const;
    std::vector<Polyline>::const_iterator cbegin() const;
    std::vector<Polyline>::const_iterator cend() const;

    // Numerical attributes for polylines
    void setPolylineNumericAttribute(size_t polylineIndex, const std::string& name, double value);
    std::optional<double> getPolylineNumericAttribute(size_t polylineIndex, const std::string& name) const;
    bool hasPolylineNumericAttribute(size_t polylineIndex, const std::string& name) const;
    void removePolylineNumericAttribute(size_t polylineIndex, const std::string& name);

    // String attributes for polylines
    void setPolylineStringAttribute(size_t polylineIndex, const std::string& name, const std::string& value);
    std::optional<std::string> getPolylineStringAttribute(size_t polylineIndex, const std::string& name) const;
    bool hasPolylineStringAttribute(size_t polylineIndex, const std::string& name) const;
    void removePolylineStringAttribute(size_t polylineIndex, const std::string& name);

    // Bulk attribute operations
    void setNumericAttributeForAllPolylines(const std::string& name, double value);
    void setStringAttributeForAllPolylines(const std::string& name, const std::string& value);
    void setNumericAttributeForRange(size_t start, size_t end, const std::string& name, double value);
    void setStringAttributeForRange(size_t start, size_t end, const std::string& name, const std::string& value);

    // Query operations
    std::vector<size_t> findPolylinesWithNumericAttribute(const std::string& name) const;
    std::vector<size_t> findPolylinesWithStringAttribute(const std::string& name) const;
    std::vector<size_t> findPolylinesWithNumericValue(const std::string& name, double value, double tolerance = 1e-9) const;
    std::vector<size_t> findPolylinesWithStringValue(const std::string& name, const std::string& value) const;

    // Advanced query operations with predicates
    std::vector<size_t> findPolylinesWhere(std::function<bool(const Polyline&, size_t)> predicate) const;
    std::vector<size_t> findPolylinesWithNumericRange(const std::string& name, double minValue, double maxValue) const;

    // Statistical operations on polyline numeric attributes
    std::optional<double> getMinNumericAttribute(const std::string& name) const;
    std::optional<double> getMaxNumericAttribute(const std::string& name) const;
    std::optional<double> getAverageNumericAttribute(const std::string& name) const;
    std::pair<std::optional<double>, std::optional<double>> getNumericAttributeRange(const std::string& name) const;

    // Attribute metadata
    std::set<std::string> getAllNumericAttributeNames() const;
    std::set<std::string> getAllStringAttributeNames() const;
    std::set<std::string> getAllAttributeNames() const;

    // Statistics about polylines themselves
    size_t getTotalPointCount() const override;
    std::optional<size_t> getMinPolylineSize() const;
    std::optional<size_t> getMaxPolylineSize() const;
    double getAveragePolylineSize() const;

    // Spatial operations
    std::pair<Point, Point> getBoundingBox() const override;
    std::vector<size_t> findPolylinesIntersectingBounds(const Point& minPoint, const Point& maxPoint) const;

    // Filtering operations
    PolylineSet filterByNumericAttribute(const std::string& name, double minValue, double maxValue) const;
    PolylineSet filterByStringAttribute(const std::string& name, const std::string& value) const;
    PolylineSet filterBySize(size_t minSize, size_t maxSize = SIZE_MAX) const;
    PolylineSet filterByPredicate(std::function<bool(const Polyline&, size_t)> predicate) const;

    // Sorting operations
    void sortByNumericAttribute(const std::string& name, bool ascending = true);
    void sortByStringAttribute(const std::string& name, bool ascending = true);
    void sortBySize(bool ascending = true);
    void sortByCustom(std::function<bool(const std::pair<Polyline*, size_t>&, const std::pair<Polyline*, size_t>&)> comparator);

    // File I/O operations
    void saveAsGeoJSON(const QString& filename, int crsEPSG = 4326) const override;
    void loadFromGeoJSON(const QString& filename) override;
    void saveAsEnhancedGeoJSON(const QString& filename, int crsEPSG = 4326) const;
    void loadFromEnhancedGeoJSON(const QString& filename);

    // Shapefile I/O operations using GDAL
    void loadFromShapefile(const QString& filename);
    void saveAsShapefile(const QString& filename, int crsEPSG = 4326) const;

    // Export operations
    void exportNumericAttributesToCSV(const QString& filename, const std::vector<std::string>& attributeNames = {}) const;
    void exportStringAttributesToCSV(const QString& filename, const std::vector<std::string>& attributeNames = {}) const;
    void exportSummaryStatistics(const QString& filename) const;

private:
    std::vector<Polyline> polylines_;
    std::vector<std::map<std::string, double>> numeric_attributes_;  // Per-polyline numeric attributes
    std::vector<std::map<std::string, std::string>> string_attributes_;  // Per-polyline string attributes

    // Helper methods
    void validateIndex(size_t index) const;
    void ensureAttributeVectorSize(size_t requiredSize);
    std::vector<size_t> sortIndicesByNumeric(const std::string& name, bool ascending) const;
    std::vector<size_t> sortIndicesByString(const std::string& name, bool ascending) const;
    void reorderByIndices(const std::vector<size_t>& indices);

    // GDAL helper methods
    void processOGRFeature(OGRFeature* feature, size_t polylineIndex);
    void processLineString(class OGRLineString* lineString);
    std::string ogrFieldTypeToString(int fieldType) const;
};

#endif // POLYLINESET_H

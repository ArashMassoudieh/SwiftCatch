#pragma once
#include "junction.h"
#include <QVector>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QMap>
#include <functional>
#include <memory>

// Forward declaration
class Polyline;
class GeoTiffHandler;

class JunctionSet {
private:
    QVector<Junction> junctions_;

public:
    // Constructors
    JunctionSet();
    JunctionSet(std::initializer_list<Junction> junctions);

    // Copy constructor and assignment operator
    JunctionSet(const JunctionSet& other);
    JunctionSet& operator=(const JunctionSet& other);

    // Move constructor and assignment operator
    JunctionSet(JunctionSet&& other) noexcept;
    JunctionSet& operator=(JunctionSet&& other) noexcept;

    // Destructor
    ~JunctionSet() = default;

    // Basic container operations
    void addJunction(const Junction& junction);
    void addJunction(Junction&& junction);
    void removeJunction(int index);
    void removeJunctionsAt(const QVector<int>& indices);
    void clear();

    // Accessors
    const Junction& getJunction(int index) const;
    Junction& getJunction(int index);
    const Junction& operator[](int index) const;
    Junction& operator[](int index);

    // Container properties
    int size() const;
    bool isEmpty() const;
    bool empty() const; // STL compatibility

    // Iterator support
    QVector<Junction>::iterator begin();
    QVector<Junction>::iterator end();
    QVector<Junction>::const_iterator begin() const;
    QVector<Junction>::const_iterator end() const;
    QVector<Junction>::const_iterator cbegin() const;
    QVector<Junction>::const_iterator cend() const;

    // Spatial queries
    QVector<int> findJunctionsInRadius(const QPointF& center, double radius) const;
    QVector<int> findJunctionsInBounds(const QRectF& bounds) const;
    QVector<int> findJunctionsWithinTolerance(const QPointF& point, double tolerance) const;
    int findNearestJunction(const QPointF& point) const;
    QVector<int> findKNearestJunctions(const QPointF& point, int k) const;

    // Junction analysis
    QVector<int> findJunctionsWithConnectionCount(int count) const;
    QVector<int> findJunctionsWithMinConnections(int minCount) const;
    QVector<int> findJunctionsWithMaxConnections(int maxCount) const;
    QVector<int> findIsolatedJunctions() const; // No connections
    QVector<int> findEndJunctions() const; // Exactly 1 connection
    QVector<int> findBranchJunctions() const; // More than 2 connections

    // Attribute-based queries
    QVector<int> findJunctionsWithAttribute(const QString& attributeName) const;
    QVector<int> findJunctionsWithNumericValue(const QString& attributeName, double value, double tolerance = 1e-6) const;
    QVector<int> findJunctionsWithStringValue(const QString& attributeName, const QString& value) const;
    QVector<int> findJunctionsWithNumericRange(const QString& attributeName, double minValue, double maxValue) const;

    // Custom filtering
    QVector<int> findJunctionsWith(std::function<bool(const Junction&, int)> predicate) const;
    JunctionSet filterJunctions(std::function<bool(const Junction&, int)> predicate) const;

    // Spatial operations
    QRectF getBoundingBox() const;
    QPointF getCentroid() const;
    double getMinDistance(const QPointF& point) const;
    double getMaxDistance(const QPointF& point) const;

    // Bulk attribute operations
    void setAttributeForAll(const QString& name, const QVariant& value);
    void setNumericAttributeForAll(const QString& name, double value);
    void setStringAttributeForAll(const QString& name, const QString& value);
    void removeAttributeFromAll(const QString& name);

    // Attribute metadata
    QStringList getAllAttributeNames() const;
    QMap<QString, QVariant::Type> getAttributeTypes() const;

    // Statistics
    int getTotalConnectionCount() const;
    double getAverageConnectionCount() const;
    int getMaxConnectionCount() const;
    int getMinConnectionCount() const;

    // Attribute statistics (for numeric attributes)
    double getMinNumericAttribute(const QString& name) const;
    double getMaxNumericAttribute(const QString& name) const;
    double getAverageNumericAttribute(const QString& name) const;
    QPair<double, double> getNumericAttributeRange(const QString& name) const;

    // Sorting operations
    void sortByDistance(const QPointF& referencePoint, bool ascending = true);
    void sortByConnectionCount(bool ascending = true);
    void sortByNumericAttribute(const QString& attributeName, bool ascending = true);
    void sortByStringAttribute(const QString& attributeName, bool ascending = true);
    void sortByCustom(std::function<bool(const Junction&, const Junction&)> comparator);

    // Merge operations
    void mergeJunctionsWithinTolerance(double tolerance);
    void mergeWith(const JunctionSet& other);

    // Validation and cleanup
    void removeInvalidJunctions();
    void validateConnections();

    // Export/Import operations
    void saveToFile(const QString& filename) const;
    void loadFromFile(const QString& filename);
    void exportToCSV(const QString& filename, const QStringList& attributesToExport = QStringList()) const;

    // GeoJSON operations
    void saveAsGeoJSON(const QString& filename, int crsEPSG = 4326) const;
    void loadFromGeoJSON(const QString& filename);

    // Shapefile operations
    void saveAsShapefile(const QString& filename, int crsEPSG = 4326) const;
    void loadFromShapefile(const QString& filename);

    // Debug and utility
    QString toString() const;
    void printStatistics() const;

    // Junction creation helpers
    static Junction createJunctionAt(const QPointF& location);
    static Junction createJunctionAt(double x, double y);
    static Junction createJunctionWithAttributes(const QPointF& location, const QMap<QString, QVariant>& attributes);

    // Elevation assignment
    void assignElevationToJunctions(const GeoTiffHandler* demPtr, const QString& attributeName = "elevation");

private:
    // Helper methods
    void validateIndex(int index) const;
    QVector<int> sortIndicesByDistance(const QPointF& point, bool ascending = true) const;
};

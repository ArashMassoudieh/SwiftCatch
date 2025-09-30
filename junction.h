#pragma once
#include <QPointF>
#include <QMap>
#include <QVariant>
#include <QString>
#include <QVector>
#include <memory>

// Forward declaration - adjust based on your actual Polyline class name
class Polyline;

class Junction {
private:
    QPointF location_;
    QVector<std::shared_ptr<Polyline>> connectedPolylines_;
    QMap<QString, QVariant> attributes_;

public:
    // Constructors
    Junction();
    Junction(const QPointF& location);
    Junction(double x, double y);
    Junction(const QPointF& location, const QMap<QString, QVariant>& attributes);

    // Copy constructor and assignment operator
    Junction(const Junction& other);
    Junction& operator=(const Junction& other);

    // Move constructor and assignment operator
    Junction(Junction&& other) noexcept;
    Junction& operator=(Junction&& other) noexcept;

    // Destructor
    ~Junction() = default;

    // Location accessors
    const QPointF& getLocation() const;
    void setLocation(const QPointF& location);
    void setLocation(double x, double y);

    double x() const;
    double y() const;

    // Polyline connection management
    void addConnectedPolyline(std::shared_ptr<Polyline> polyline);
    void removeConnectedPolyline(std::shared_ptr<Polyline> polyline);
    bool isConnectedTo(std::shared_ptr<Polyline> polyline) const;
    const QVector<std::shared_ptr<Polyline>>& getConnectedPolylines() const;
    int getConnectionCount() const;
    bool hasConnections() const;

    // Attribute management
    void setAttribute(const QString& name, const QVariant& value);
    QVariant getAttribute(const QString& name) const;
    QVariant getAttribute(const QString& name, const QVariant& defaultValue) const;
    bool hasAttribute(const QString& name) const;
    void removeAttribute(const QString& name);
    void clearAttributes();

    const QMap<QString, QVariant>& getAllAttributes() const;
    QStringList getAttributeNames() const;

    // Convenience methods for common attribute types
    void setNumericAttribute(const QString& name, double value);
    void setStringAttribute(const QString& name, const QString& value);
    void setIntAttribute(const QString& name, int value);
    void setBoolAttribute(const QString& name, bool value);

    double getNumericAttribute(const QString& name, double defaultValue = 0.0) const;
    QString getStringAttribute(const QString& name, const QString& defaultValue = QString()) const;
    int getIntAttribute(const QString& name, int defaultValue = 0) const;
    bool getBoolAttribute(const QString& name, bool defaultValue = false) const;

    // Distance calculations
    double distanceTo(const QPointF& point) const;
    double distanceTo(const Junction& other) const;
    bool isWithinDistance(const QPointF& point, double tolerance) const;
    bool isWithinDistance(const Junction& other, double tolerance) const;

    // Comparison operators
    bool operator==(const Junction& other) const;
    bool operator!=(const Junction& other) const;

    // Utility methods
    bool isEmpty() const;
    void clear();

    // Debug/serialization support
    QString toString() const;
};

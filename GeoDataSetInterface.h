#pragma once
#include <QVariantMap>
#include <QRectF>
#include <QUuid>
#include <QJsonObject>


class GeoDataEntry  {
public:

    QVector<QPointF> location;         // Holds the point location
    QString primaryKey; 
    QVariantMap attributes; 
    // Constructor to initialize with values
    GeoDataEntry()
    {
        primaryKey = QUuid::createUuid().toString();
    }
    GeoDataEntry(const QVariantMap& attr, const QVector<QPointF> &loc, const QString& Key)
    {
        attributes = attr;
        location = loc;
        primaryKey = Key;
    }
    // Copy Assignment Operator
    GeoDataEntry& operator=(const GeoDataEntry& other) {
        if (this != &other) {  // Prevent self-assignment
            attributes = other.attributes;  // Copy attributes
            location = other.location;      // Copy location data
            primaryKey = other.primaryKey;  // Copy primary key
        }
        return *this;
    }
    ~GeoDataEntry() {};

};

enum class featuretype {Points, MultiPolyline, Polygons};

class GeoDataSetInterface : public QVector<GeoDataEntry> {
public:
    // Default Constructor
    GeoDataSetInterface() = default;

    // Copy Constructor
    GeoDataSetInterface(const GeoDataSetInterface& other)
        : QVector<GeoDataEntry>(other), FeatureType(other.FeatureType) {}

    // Assignment Operator
    GeoDataSetInterface& operator=(const GeoDataSetInterface& other) {
        if (this != &other) {  // Prevent self-assignment
            QVector<GeoDataEntry>::operator=(other);  // Copy base class data
            FeatureType = other.FeatureType;  // Copy additional member
        }
        return *this;
    }

    // Destructor (default behavior is fine since QVector manages memory)
    virtual ~GeoDataSetInterface() = default;

    // Virtual Functions (Fixed Signatures)
    virtual unsigned int count() const { return this->size(); }
    virtual QRectF BoundingBox() { 
        return QRectF(); 
    }
    virtual GeoDataEntry* begin() { return this->data(); }  // Pointer to first element
    virtual GeoDataEntry* end() { return this->data() + this->size(); }  // Pointer to last element
    featuretype FeatureType; 
    virtual QJsonObject toJsonObject() { return QJsonObject(); }
    GeoDataSetInterface filterByAttribute(const QString& key, const QVariant& value);
    GeoDataSetInterface fromGeoJson(const QJsonDocument& geoJsonDoc);
};


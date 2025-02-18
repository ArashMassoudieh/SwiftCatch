#pragma once
#ifndef POLYLINEGEODATASET_H
#define POLYLINEGEODATASET_H
#include "GeoDataSetInterface.h"
#include <QVector>
#include <QVariantMap>
#include <QPoint>
#include <QUuid>
#include <QRectF>


class PolylineGeoDataSet :
    public GeoDataSetInterface
{
public:
    // Default Constructor
    PolylineGeoDataSet() : GeoDataSetInterface() { FeatureType = featuretype::Points; }

    // Copy Constructor
    PolylineGeoDataSet(const PolylineGeoDataSet& other)
        : GeoDataSetInterface(other) {
        FeatureType = featuretype::Points;
    }

    PolylineGeoDataSet(const QJsonDocument& jsonDoc);
    // Assignment Operator
    PolylineGeoDataSet& operator=(const PolylineGeoDataSet& other) {
        if (this != &other) {
            GeoDataSetInterface::operator=(other);  // Copy QVector data
            // GeoDataSetInterface does not have a copy assignment operator by default
        }
        return *this;
    }

    // Destructor
    ~PolylineGeoDataSet() {}

    QJsonObject toJsonObject() override;

    QRectF BoundingBox() override
    {
        QRectF rect = bounding_Box(first().location);
        qreal minX = rect.left();
        qreal maxX = rect.right();
        qreal minY = rect.bottom();
        qreal maxY = rect.top();

        for (const GeoDataEntry& p : *this) {
            rect = bounding_Box(p.location);
            if (rect.left() < minX) minX = rect.left();
            if (rect.right() > maxX) maxX = rect.right();
            if (rect.bottom() < minY) minY = rect.bottom();
            if (rect.top() > maxY) maxY = rect.top();
        }

        return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
    }


};



#endif // POLYLINEGEODATASET_H

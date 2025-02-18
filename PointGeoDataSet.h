#pragma once
#ifndef POINTGEODATASET_H
#define POINTGEODATASET_H

#include "GeoDataSetInterface.h"
#include <QVector>
#include <QVariantMap>
#include <QPoint>
#include <QUuid>
#include <QRectF>



struct station_info
{
    QString agency_cd;
    QString site_no;
    QString station_nm;
    QString site_tp_cd;
    double dec_lat_va;
    double dec_long_va;
    QString coord_acy_cd;
    QString ddec_coord_datum_cd;
    QString alt_va;
    QString alt_acy_va;
    QString alt_datum_cd;
    QString huc_cd;
};


class PointGeoDataSet :
    public GeoDataSetInterface
{
public:
    // Default Constructor
    PointGeoDataSet() : GeoDataSetInterface() { FeatureType = featuretype::Points; }

    // Copy Constructor
    PointGeoDataSet(const PointGeoDataSet& other)
        : GeoDataSetInterface(other) {
        FeatureType = featuretype::Points;
    }

    PointGeoDataSet(const QJsonDocument& jsonDoc);
    // Assignment Operator
    PointGeoDataSet& operator=(const PointGeoDataSet& other) {
        if (this != &other) {
            GeoDataSetInterface::operator=(other);  // Copy QVector data
            // GeoDataSetInterface does not have a copy assignment operator by default
        }
        return *this;
    }
    PointGeoDataSet(const QMap<QString, station_info>& geodataset);

    // Destructor
    ~PointGeoDataSet() {}

    QJsonObject toJsonObject() override;
    
    QRectF BoundingBox() override
    {
        qreal minX = first().location[0].x();
        qreal maxX = first().location[0].x();
        qreal minY = first().location[0].y();
        qreal maxY = first().location[0].y();

        for (const GeoDataEntry& p : *this) {
            if (p.location.first().x() < minX) minX = p.location[0].x();
            if (p.location.first().x() > maxX) maxX = p.location[0].x();
            if (p.location.first().y() < minY) minY = p.location[0].y();
            if (p.location.first().y() > maxY) maxY = p.location[0].y();
        }

        return QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
    }
    
    
};
#endif

#include "GeoDataSetInterface.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

GeoDataSetInterface GeoDataSetInterface::filterByAttribute(const QString& key, const QVariant& value) {
    GeoDataSetInterface filteredDataset;

    for (const GeoDataEntry& item : *this) {
        if (item.attributes.contains(key) && item.attributes.value(key) == value) {
            GeoDataEntry newItem = item;
            newItem.location = item.location;
            newItem.primaryKey = item.primaryKey;
            filteredDataset.append(newItem);  // Copy matching item
        }
    }

    return filteredDataset;
}

GeoDataSetInterface GeoDataSetInterface::fromGeoJson(const QJsonDocument& geoJsonDoc) {
    GeoDataSetInterface dataset;

    if (!geoJsonDoc.isObject()) {
        qCritical() << "Invalid GeoJSON format: Root is not an object.";
        return dataset;
    }

    QJsonObject rootObject = geoJsonDoc.object();
    QJsonArray featuresArray = rootObject.value("features").toArray();

    for (const QJsonValue& featureValue : featuresArray) {
        if (!featureValue.isObject()) continue;

        QJsonObject featureObject = featureValue.toObject();
        QJsonObject geometry = featureObject["geometry"].toObject();
        QJsonObject properties = featureObject["properties"].toObject();

        QString type = geometry["type"].toString();
        QVariantMap attributes = properties.toVariantMap();
        QVector<QPointF> locations;

        if (type == "Point") {
            QJsonArray coordinates = geometry["coordinates"].toArray();
            if (coordinates.size() >= 2) {
                locations.append(QPointF(coordinates[0].toDouble(), coordinates[1].toDouble()));
                dataset.FeatureType = featuretype::Points;
            }
        }
        else if (type == "MultiLineString") {
            QJsonArray lines = geometry["coordinates"].toArray();
            for (const QJsonValue& lineValue : lines) {
                QJsonArray line = lineValue.toArray();
                for (const QJsonValue& pointValue : line) {
                    QJsonArray point = pointValue.toArray();
                    if (point.size() >= 2) {
                        locations.append(QPointF(point[0].toDouble(), point[1].toDouble()));
                    }
                }
            }
            dataset.FeatureType = featuretype::MultiPolyline;
        }
        else if (type == "Polygon") {
            QJsonArray polygons = geometry["coordinates"].toArray();
            for (const QJsonValue& ringValue : polygons) {
                QJsonArray ring = ringValue.toArray();
                for (const QJsonValue& pointValue : ring) {
                    QJsonArray point = pointValue.toArray();
                    if (point.size() >= 2) {
                        locations.append(QPointF(point[0].toDouble(), point[1].toDouble()));
                    }
                }
            }
            dataset.FeatureType = featuretype::Polygons;
        }

        if (!locations.isEmpty()) {
            dataset.append(GeoDataEntry(attributes, locations, QUuid::createUuid().toString()));
        }
    }

    return dataset;
}

#include "PointGeoDataSet.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>




QJsonObject PointGeoDataSet::toJsonObject() {
    QJsonObject jsonObject;
    QJsonArray featuresArray;

    // Convert each QVariantMap item into a QJsonObject
    for (const GeoDataEntry& item : *this) {
        QJsonObject featureObject;

        for (auto it = item.attributes.begin(); it != item.attributes.end(); ++it) {
            featureObject.insert(it.key(), QJsonValue::fromVariant(it.value()));
            featureObject.insert("x", item.location[0].x());
            featureObject.insert("y", item.location[0].y());
            featureObject.insert("primaryKey", item.primaryKey);
        }

        featuresArray.append(featureObject);
    }

    // Add features and point location to the final object
    jsonObject.insert("features", featuresArray);
    
    return jsonObject;
}

PointGeoDataSet::PointGeoDataSet(const QJsonDocument& jsonDoc) {
    if (!jsonDoc.isObject()) {
        qCritical() << "Invalid JSON format: Root is not an object.";
        return;
    }

    QJsonObject rootObject = jsonDoc.object();
    QJsonArray featuresArray = rootObject.value("features").toArray();

    for (const QJsonValue& featureValue : featuresArray) {
        if (!featureValue.isObject()) continue; // Skip invalid entries

        QJsonObject featureObject = featureValue.toObject();
        GeoDataEntry datapoint; 
        QPointF location; 
        // Extract attributes and location separately
        for (auto it = featureObject.begin(); it != featureObject.end(); ++it) {
            if (it.key() == "x") {
                location.setX(it.value().toInt());
            }
            else if (it.key() == "y") {
                location.setY(it.value().toInt());
            }
            else if (it.key() == "primaryKey")
            {
                datapoint.primaryKey = it.value().toString(); 
            }
            else {
                datapoint.attributes.insert(it.key(), it.value().toVariant());
            }
        }

        // Add entry to dataset
        datapoint.location.append(location);
        append(datapoint);
        FeatureType = featuretype::Points;
    }
}

PointGeoDataSet::PointGeoDataSet(const QMap<QString, station_info>& geodataset)
{
    for (const station_info item : geodataset)
    {
        GeoDataEntry entry;
        entry.attributes["Agency Code"] = item.agency_cd;
        entry.attributes["Station Name"] = item.station_nm;
        entry.attributes["Site No"] = item.site_no;
        QPointF location;
        location.setX(item.dec_long_va);
        location.setY(item.dec_lat_va);
        entry.location.append(location);
        append(entry);
    }
    FeatureType = featuretype::Points;
    
}


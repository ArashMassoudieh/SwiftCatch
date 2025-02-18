#pragma once
#include <QAbstractTableModel>
#include <QVector>
#include <QVariantMap>
#include <QPointF>
#include "PointGeoDataSet.h"

class GeoDataModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit GeoDataModel(GeoDataSetInterface *dataset, QObject* parent = nullptr)
        : QAbstractTableModel(parent), dataSet(dataset) {

        if (!dataset->isEmpty()) {
            // Extract column names from the first entry
            columnNames = dataset->first().attributes.keys();
            columnNames.prepend("x");  // Add x coordinate column
            columnNames.prepend("y");  // Add y coordinate column
            columnNames.prepend("Primary Key");  // Add Primary Key column
        }
    }

    // Number of rows
    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return dataSet->count(); 
            ;
    }

    // Number of columns
    int columnCount(const QModelIndex& parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return columnNames.size();
    }

    // Header names
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            return columnNames.at(section);
        }
        return QVariant();
    }

    // Data for each cell
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override {
        if (!index.isValid() || role != Qt::DisplayRole) {
            return QVariant();
        }

        const GeoDataEntry& entry = dataSet->at(index.row());

        if (index.column() == 0) return entry.primaryKey;  // Primary Key
        if (index.column() == 1) return entry.location[0].x();  // X Coordinate
        if (index.column() == 2) return entry.location[0].y();  // Y Coordinate
        
        // Fetch attribute values dynamically
        QString key = columnNames.at(index.column());
        return entry.attributes.value(key, QVariant());
    }

private:
    GeoDataSetInterface *dataSet;
    QStringList columnNames;
};


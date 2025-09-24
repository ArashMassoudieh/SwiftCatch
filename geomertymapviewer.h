#ifndef GEOMETRYMAPVIEWER_H
#define GEOMETRYMAPVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>
#include <QGraphicsPathItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QRandomGenerator>
#include <QMap>
#include <QColor>
#include <memory>

#include "geometrybase.h"
#include "polyline.h"
#include "polylineset.h"

struct GeometryLayerStyle {
    QString name;
    std::shared_ptr<GeometryBase> geometry;
    QColor color;
    int lineWidth;
    int pointSize;
    bool visible;
    Qt::PenStyle lineStyle;

    // Add default constructor
    GeometryLayerStyle() : color(Qt::blue), lineWidth(2), pointSize(4), visible(true), lineStyle(Qt::SolidLine) {}

    GeometryLayerStyle(const QString& layerName, std::shared_ptr<GeometryBase> geom,
                       const QColor& layerColor = Qt::blue, int width = 2, int ptSize = 4,
                       Qt::PenStyle style = Qt::SolidLine)
        : name(layerName), geometry(geom), color(layerColor),
        lineWidth(width), pointSize(ptSize), visible(true), lineStyle(style) {}
};

class GeometryMapViewer : public QGraphicsView {
    Q_OBJECT

public:
    explicit GeometryMapViewer(QWidget* parent = nullptr);
    explicit GeometryMapViewer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                               const QString& attributeKey = "", QWidget* parent = nullptr);

    // Layer management
    void addGeometryLayer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                          const QColor& color = Qt::blue, int lineWidth = 2, int pointSize = 4,
                          const QString& attributeKey = "");
    void removeGeometryLayer(const QString& layerName);
    void clearAllLayers();

    // Layer properties
    void setLayerVisible(const QString& layerName, bool visible);
    void setLayerColor(const QString& layerName, const QColor& color);
    void setLayerLineWidth(const QString& layerName, int width);
    void setLayerPointSize(const QString& layerName, int size);
    void setLayerLineStyle(const QString& layerName, Qt::PenStyle style);

    // Navigation modes
    void togglePanMode();
    void toggleSelectMode();
    void enableZoomWindowMode();

    // View control
    void zoomIn();
    void zoomOut();
    void zoomExtent();
    void zoomToLayer(const QString& layerName);

protected:
    // Mouse events
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

signals:
    void boundingBoxSelected(double minX, double minY, double maxX, double maxY);
    void geometryClicked(const QString& layerName, double x, double y);
    void mousePositionUpdated(double x, double y);

private slots:
    void refreshScene();

private:
    // Data members
    QGraphicsScene* scene_;
    QMap<QString, GeometryLayerStyle> layers_;
    QMap<QString, QList<QGraphicsItem*>> layerItems_;

    // Interaction state
    bool selecting_;
    bool panMode_;
    bool zoomWindowMode_;
    QPointF selectionStart_;
    QPointF selectionEnd_;
    QGraphicsRectItem* selectionRect_;

    // Helper methods
    void initializeViewer();
    void renderGeometry(const QString& layerName, const GeometryLayerStyle& layer);
    void renderPolyline(const QString& layerName, const Polyline* polyline, const GeometryLayerStyle& layer);
    void renderPolylineSet(const QString& layerName, const PolylineSet* polylineSet, const GeometryLayerStyle& layer);
    void clearLayerItems(const QString& layerName);
    void zoomIntoSelection();
    QColor generateRandomColor();

    // Coordinate transformation (if needed - can be customized)
    QPointF transformCoordinate(const QPointF& worldPoint);
    QPointF reverseTransformCoordinate(const QPointF& scenePoint);
};

#endif // GEOMETRYMAPVIEWER_H

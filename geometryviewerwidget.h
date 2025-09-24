#ifndef GEOMETRYVIEWERWIDGET_H
#define GEOMETRYVIEWERWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QRectF>
#include <QPointF>
#include <QColor>
#include <QMap>
#include <QRandomGenerator>
#include <memory>

#include "geometrybase.h"
#include "polyline.h"
#include "polylineset.h"

struct GeometryLayer {
    QString name;
    std::shared_ptr<GeometryBase> geometry;
    QColor color;
    int lineWidth;
    int pointSize;
    bool visible;

    // Add default constructor
    GeometryLayer() : color(Qt::blue), lineWidth(2), pointSize(4), visible(true) {}

    GeometryLayer(const QString& layerName, std::shared_ptr<GeometryBase> geom,
                  const QColor& layerColor = Qt::blue, int width = 2, int ptSize = 4)
        : name(layerName), geometry(geom), color(layerColor),
        lineWidth(width), pointSize(ptSize), visible(true) {}
};

class GeometryViewerWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit GeometryViewerWidget(QWidget* parent = nullptr);
    ~GeometryViewerWidget() = default;

    // Layer management
    void addGeometry(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                     const QColor& color = Qt::blue, int lineWidth = 2, int pointSize = 4);
    void removeGeometry(const QString& layerName);
    void clearAllGeometries();

    // Layer properties
    void setLayerVisible(const QString& layerName, bool visible);
    void setLayerColor(const QString& layerName, const QColor& color);
    void setLayerLineWidth(const QString& layerName, int width);
    void setLayerPointSize(const QString& layerName, int size);

    // View control
    void zoomExtents();
    void zoomToLayer(const QString& layerName);
    void setBackgroundColor(const QColor& color);

public slots:
    void refresh() { update(); }

signals:
    void geometryClicked(const QString& layerName, const QPointF& worldPoint);
    void viewChanged(const QRectF& viewBounds);

protected:
    // OpenGL overrides
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    // Mouse events
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    // Data
    QMap<QString, GeometryLayer> layers_;
    QRectF worldBounds_;
    QRectF viewBounds_;
    QColor backgroundColor_;

    // Interaction state
    enum InteractionMode {
        Pan,
        ZoomWindow,
        Select
    };
    InteractionMode currentMode_;
    bool isInteracting_;
    QPointF lastMousePos_;
    QPointF zoomStartPos_;
    QPointF zoomEndPos_;

    // View transformation
    QPointF worldOffset_;
    double scaleFactor_;

    // Helper methods
    void calculateWorldBounds();
    void updateViewTransform();
    QPointF screenToWorld(const QPointF& screenPoint) const;
    QPointF worldToScreen(const QPointF& worldPoint) const;
    QRectF screenToWorld(const QRectF& screenRect) const;

    // Drawing methods
    void drawPolyline(QPainter& painter, const Polyline* polyline, const GeometryLayer& layer);
    void drawPolylineSet(QPainter& painter, const PolylineSet* polylineSet, const GeometryLayer& layer);
    void drawZoomRectangle(QPainter& painter);

    // Utility
    QColor generateRandomColor();
    void panView(const QPointF& delta);
    void zoomView(double factor, const QPointF& center = QPointF());
    void applyZoomWindow();

    // Constants
    static constexpr double MIN_ZOOM = 0.01;
    static constexpr double MAX_ZOOM = 1000.0;
    static constexpr double ZOOM_WHEEL_FACTOR = 1.2;
    static constexpr int MIN_ZOOM_RECT_SIZE = 5;
};

#endif // GEOMETRYVIEWERWIDGET_H

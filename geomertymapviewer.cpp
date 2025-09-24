#include "geomertymapviewer.h"
#include <QApplication>
#include <QTimer>
#include <cmath>
#include <algorithm>

GeometryMapViewer::GeometryMapViewer(QWidget* parent)
    : QGraphicsView(parent)
    , scene_(nullptr)
    , selecting_(false)
    , panMode_(true)
    , zoomWindowMode_(false)
    , selectionRect_(nullptr)
{
    initializeViewer();
}

GeometryMapViewer::GeometryMapViewer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                                     const QString& attributeKey, QWidget* parent)
    : QGraphicsView(parent)
    , scene_(nullptr)
    , selecting_(false)
    , panMode_(true)
    , zoomWindowMode_(false)
    , selectionRect_(nullptr)
{
    initializeViewer();
    addGeometryLayer(layerName, geometry, Qt::blue, 2, 4, attributeKey);
}

// ============================================================================
// Initialization
// ============================================================================

void GeometryMapViewer::initializeViewer() {
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing);
    setDragMode(QGraphicsView::ScrollHandDrag);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // Set background
    setBackgroundBrush(QBrush(Qt::white));
}

// ============================================================================
// Layer Management
// ============================================================================

// Add this to the end of your addGeometryLayer method:
void GeometryMapViewer::addGeometryLayer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                                         const QColor& color, int lineWidth, int pointSize,
                                         const QString& attributeKey) {
    if (!geometry) {
        qWarning() << "Cannot add null geometry to layer" << layerName;
        return;
    }

    // Remove existing layer if it exists
    if (layers_.contains(layerName)) {
        removeGeometryLayer(layerName);
    }

    // Create layer style
    QColor layerColor = color.isValid() ? color : generateRandomColor();
    layers_[layerName] = GeometryLayerStyle(layerName, geometry, layerColor, lineWidth, pointSize);

    // Render the geometry
    renderGeometry(layerName, layers_[layerName]);

    // FORCE scene rect update - this is crucial for large coordinates
    QRectF itemsBounds = scene_->itemsBoundingRect();
    qDebug() << "Items bounding rect:" << itemsBounds;

    // Explicitly set the scene rectangle to encompass all items
    scene_->setSceneRect(itemsBounds);

    // Auto-zoom to new content if this is the first layer
    if (layers_.size() == 1) {
        QTimer::singleShot(100, this, &GeometryMapViewer::zoomExtent);
    }
}
void GeometryMapViewer::removeGeometryLayer(const QString& layerName) {
    if (!layers_.contains(layerName)) return;

    clearLayerItems(layerName);
    layers_.remove(layerName);
}

void GeometryMapViewer::clearAllLayers() {
    for (const QString& layerName : layers_.keys()) {
        clearLayerItems(layerName);
    }
    layers_.clear();
    scene_->clear();
}

// ============================================================================
// Layer Properties
// ============================================================================

void GeometryMapViewer::setLayerVisible(const QString& layerName, bool visible) {
    if (!layers_.contains(layerName)) return;

    layers_[layerName].visible = visible;

    // Update visibility of graphics items
    if (layerItems_.contains(layerName)) {
        for (QGraphicsItem* item : layerItems_[layerName]) {
            item->setVisible(visible);
        }
    }
}

void GeometryMapViewer::setLayerColor(const QString& layerName, const QColor& color) {
    if (!layers_.contains(layerName)) return;

    layers_[layerName].color = color;

    // Re-render the layer with new color
    clearLayerItems(layerName);
    renderGeometry(layerName, layers_[layerName]);
}

void GeometryMapViewer::setLayerLineWidth(const QString& layerName, int width) {
    if (!layers_.contains(layerName)) return;

    layers_[layerName].lineWidth = std::max(1, width);

    // Re-render the layer
    clearLayerItems(layerName);
    renderGeometry(layerName, layers_[layerName]);
}

void GeometryMapViewer::setLayerPointSize(const QString& layerName, int size) {
    if (!layers_.contains(layerName)) return;

    layers_[layerName].pointSize = std::max(1, size);

    // Re-render the layer
    clearLayerItems(layerName);
    renderGeometry(layerName, layers_[layerName]);
}

void GeometryMapViewer::setLayerLineStyle(const QString& layerName, Qt::PenStyle style) {
    if (!layers_.contains(layerName)) return;

    layers_[layerName].lineStyle = style;

    // Re-render the layer
    clearLayerItems(layerName);
    renderGeometry(layerName, layers_[layerName]);
}

// ============================================================================
// Navigation Control
// ============================================================================

void GeometryMapViewer::togglePanMode() {
    panMode_ = true;
    zoomWindowMode_ = false;
    setDragMode(QGraphicsView::ScrollHandDrag);
    setCursor(Qt::ArrowCursor);
}

void GeometryMapViewer::toggleSelectMode() {
    panMode_ = false;
    zoomWindowMode_ = false;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
}

void GeometryMapViewer::enableZoomWindowMode() {
    zoomWindowMode_ = true;
    panMode_ = false;
    setDragMode(QGraphicsView::NoDrag);
    setCursor(Qt::CrossCursor);
}

void GeometryMapViewer::zoomIn() {
    scale(1.2, 1.2);
}

void GeometryMapViewer::zoomOut() {
    scale(0.8, 0.8);
}

void GeometryMapViewer::zoomExtent() {
    if (!scene_ || scene_->items().isEmpty()) {
        qDebug() << "No items in scene to zoom to";
        return;
    }

    QRectF boundingBox = scene_->itemsBoundingRect();
    qDebug() << "=== Zoom Extent Debug ===";
    qDebug() << "Items count:" << scene_->items().size();
    qDebug() << "Bounding box:" << boundingBox;
    qDebug() << "Bounding box size:" << boundingBox.width() << "x" << boundingBox.height();
    qDebug() << "Current view rect:" << mapToScene(viewport()->rect()).boundingRect();

    if (boundingBox.isEmpty()) {
        qDebug() << "Bounding box is empty";
        return;
    }

    // Add some padding around the content (10% margin)
    QRectF paddedBounds = boundingBox;
    double margin = std::max(boundingBox.width(), boundingBox.height()) * 0.1;
    paddedBounds.adjust(-margin, -margin, margin, margin);

    qDebug() << "Padded bounds:" << paddedBounds;

    // Force the scene to update its bounds
    scene_->setSceneRect(paddedBounds);

    // Fit the view to the padded bounds
    fitInView(paddedBounds, Qt::KeepAspectRatio);

    qDebug() << "After fitInView - View rect:" << mapToScene(viewport()->rect()).boundingRect();
    qDebug() << "Transform:" << transform();
}

void GeometryMapViewer::zoomToLayer(const QString& layerName) {
    if (!layers_.contains(layerName) || !layerItems_.contains(layerName)) return;

    QRectF layerBounds;
    bool first = true;

    for (QGraphicsItem* item : layerItems_[layerName]) {
        if (first) {
            layerBounds = item->boundingRect();
            first = false;
        } else {
            layerBounds = layerBounds.united(item->boundingRect());
        }
    }

    if (!layerBounds.isEmpty()) {
        fitInView(layerBounds, Qt::KeepAspectRatio);
    }
}

// ============================================================================
// Mouse Events
// ============================================================================

void GeometryMapViewer::mousePressEvent(QMouseEvent* event) {
    if (!panMode_ && event->button() == Qt::LeftButton) {
        selecting_ = true;
        selectionStart_ = mapToScene(event->pos());

        // Create selection rectangle
        selectionRect_ = new QGraphicsRectItem();
        selectionRect_->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        selectionRect_->setBrush(QBrush(Qt::blue, Qt::NoBrush));
        scene_->addItem(selectionRect_);
    }

    QGraphicsView::mousePressEvent(event);
}

void GeometryMapViewer::mouseMoveEvent(QMouseEvent* event) {
    QPointF currentPos = mapToScene(event->pos());

    // Emit mouse position updates
    QPointF worldPos = reverseTransformCoordinate(currentPos);
    emit mousePositionUpdated(worldPos.x(), worldPos.y());

    if (selecting_ && selectionRect_) {
        selectionEnd_ = currentPos;
        QRectF rect(selectionStart_, selectionEnd_);
        selectionRect_->setRect(rect.normalized());
    }

    QGraphicsView::mouseMoveEvent(event);
}

void GeometryMapViewer::mouseReleaseEvent(QMouseEvent* event) {
    if (!panMode_ && event->button() == Qt::LeftButton && selecting_) {
        selecting_ = false;
        selectionEnd_ = mapToScene(event->pos());

        if (zoomWindowMode_) {
            zoomIntoSelection();
            zoomWindowMode_ = false;
        } else {
            // Convert to world coordinates and emit selection
            QPointF worldStart = reverseTransformCoordinate(selectionStart_);
            QPointF worldEnd = reverseTransformCoordinate(selectionEnd_);

            double minX = std::min(worldStart.x(), worldEnd.x());
            double maxX = std::max(worldStart.x(), worldEnd.x());
            double minY = std::min(worldStart.y(), worldEnd.y());
            double maxY = std::max(worldStart.y(), worldEnd.y());

            emit boundingBoxSelected(minX, minY, maxX, maxY);
        }

        // Clean up selection rectangle
        if (selectionRect_) {
            scene_->removeItem(selectionRect_);
            delete selectionRect_;
            selectionRect_ = nullptr;
        }
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void GeometryMapViewer::wheelEvent(QWheelEvent* event) {
    // Zoom with mouse wheel
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
        scale(scaleFactor, scaleFactor);
    } else {
        scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
}

// ============================================================================
// Rendering Methods
// ============================================================================

void GeometryMapViewer::renderGeometry(const QString& layerName, const GeometryLayerStyle& layer) {
    qDebug() << "=== renderGeometry called ===" << layerName;

    if (!layer.geometry) {
        qDebug() << "ERROR: layer.geometry is null!";
        return;
    }

    if (!layer.visible) {
        qDebug() << "Layer is not visible, skipping";
        return;
    }

    std::string geomType = layer.geometry->getGeometryType();
    qDebug() << "Geometry type:" << QString::fromStdString(geomType);

    if (geomType == "LineString") {
        qDebug() << "Rendering as single LineString";
        auto* polyline = dynamic_cast<const Polyline*>(layer.geometry.get());
        if (polyline) {
            renderPolyline(layerName, polyline, layer);
        } else {
            qDebug() << "ERROR: Failed to cast to Polyline!";
        }
    }
    else if (geomType == "MultiLineString") {
        qDebug() << "Rendering as MultiLineString";
        auto* polylineSet = dynamic_cast<const PolylineSet*>(layer.geometry.get());
        if (polylineSet) {
            qDebug() << "PolylineSet has" << polylineSet->size() << "polylines";
            renderPolylineSet(layerName, polylineSet, layer);
        } else {
            qDebug() << "ERROR: Failed to cast to PolylineSet!";
        }
    }
    else {
        qDebug() << "ERROR: Unknown geometry type:" << QString::fromStdString(geomType);
    }

    qDebug() << "Scene items count after rendering:" << scene_->items().size();
}

void GeometryMapViewer::renderPolyline(const QString& layerName, const Polyline* polyline,
                                       const GeometryLayerStyle& layer) {

    if (!polyline) {
        qDebug() << "ERROR: polyline is null!";
        return;
    }

    if (polyline->empty()) {
        qDebug() << "ERROR: polyline is empty!";
        return;
    }

    const auto& points = polyline->getEnhancedPoints();
    //qDebug() << "Polyline has" << points.size() << "enhanced points";

    if (points.empty()) {
        qDebug() << "ERROR: getEnhancedPoints() returned empty vector!";
        return;
    }

    // Try to create just one line segment for testing
    if (points.size() >= 2) {
        QPointF start = QPointF(points[0].x, points[0].y);
        QPointF end = QPointF(points[points.size()-1].x, points[points.size()-1].y);

        //qDebug() << "Creating line from" << start << "to" << end;

        QGraphicsLineItem* lineItem = new QGraphicsLineItem(
            start.x(), start.y(), end.x(), end.y()
            );

        lineItem->setPen(QPen(layer.color, layer.lineWidth, layer.lineStyle,
                              Qt::RoundCap, Qt::RoundJoin));

        //qDebug() << "Line item created, adding to scene";
        scene_->addItem(lineItem);
        //qDebug() << "Added to scene, items count:" << scene_->items().size();

        layerItems_[layerName].append(lineItem);
        //qDebug() << "Added to layer items list";

        // Check the line item's bounding rect
        //qDebug() << "Line item bounding rect:" << lineItem->boundingRect();
        //qDebug() << "Scene bounding rect:" << scene_->itemsBoundingRect();
    }
}
void GeometryMapViewer::renderPolylineSet(const QString& layerName, const PolylineSet* polylineSet,
                                          const GeometryLayerStyle& layer) {
    qDebug() << "=== renderPolylineSet called ===" << layerName;

    if (!polylineSet) {
        qDebug() << "ERROR: polylineSet is null!";
        return;
    }

    if (polylineSet->empty()) {
        qDebug() << "ERROR: polylineSet is empty!";
        return;
    }

    qDebug() << "PolylineSet size:" << polylineSet->size();

    // Try to render just the first few polylines for debugging
    size_t maxToRender = polylineSet->size();


    for (size_t i = 0; i < maxToRender; ++i) {
        //qDebug() << "Rendering polyline" << i;
        const Polyline& polyline = polylineSet->getPolyline(i);

        //qDebug() << "Polyline" << i << "has" << polyline.size() << "points";
        if (polyline.empty()) {
            qDebug() << "Polyline" << i << "is empty, skipping";
            continue;
        }

        renderPolyline(layerName, &polyline, layer);
        //qDebug() << "Scene items after rendering polyline" << i << ":" << scene_->items().size();
    }
}

// ============================================================================
// Helper Methods
// ============================================================================

void GeometryMapViewer::clearLayerItems(const QString& layerName) {
    if (!layerItems_.contains(layerName)) return;

    for (QGraphicsItem* item : layerItems_[layerName]) {
        scene_->removeItem(item);
        delete item;
    }
    layerItems_[layerName].clear();
}

void GeometryMapViewer::zoomIntoSelection() {
    if (!selectionRect_) return;

    QRectF zoomArea = selectionRect_->rect().normalized();
    if (!zoomArea.isEmpty()) {
        fitInView(zoomArea, Qt::KeepAspectRatio);
    }
}

QColor GeometryMapViewer::generateRandomColor() {
    return QColor(
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256)
        );
}

QPointF GeometryMapViewer::transformCoordinate(const QPointF& worldPoint) {
    // Default: no transformation (can be customized for map projections)
    return worldPoint;
}

QPointF GeometryMapViewer::reverseTransformCoordinate(const QPointF& scenePoint) {
    // Default: no transformation (can be customized for map projections)
    return scenePoint;
}

void GeometryMapViewer::refreshScene() {
    // Clear and re-render all layers
    scene_->clear();
    layerItems_.clear();

    for (auto it = layers_.begin(); it != layers_.end(); ++it) {
        renderGeometry(it.key(), it.value());
    }
}

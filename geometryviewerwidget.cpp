#include "geometryviewerwidget.h"
#include <QDebug>
#include <QPolygonF>
#include <cmath>
#include <algorithm>

GeometryViewerWidget::GeometryViewerWidget(QWidget* parent)
    : QOpenGLWidget(parent)
    , backgroundColor_(Qt::white)
    , currentMode_(Pan)
    , isInteracting_(false)
    , scaleFactor_(1.0)
    , worldOffset_(0, 0)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

// ============================================================================
// Layer Management
// ============================================================================

void GeometryViewerWidget::addGeometry(const QString& layerName,
                                       std::shared_ptr<GeometryBase> geometry,
                                       const QColor& color, int lineWidth, int pointSize) {
    if (!geometry) {
        qWarning() << "Cannot add null geometry to layer" << layerName;
        return;
    }

    QColor layerColor = color.isValid() ? color : generateRandomColor();
    layers_[layerName] = GeometryLayer(layerName, geometry, layerColor, lineWidth, pointSize);

    calculateWorldBounds();
    update();
}

void GeometryViewerWidget::removeGeometry(const QString& layerName) {
    if (layers_.remove(layerName) > 0) {
        calculateWorldBounds();
        update();
    }
}

void GeometryViewerWidget::clearAllGeometries() {
    layers_.clear();
    worldBounds_ = QRectF();
    viewBounds_ = QRectF();
    update();
}

// ============================================================================
// Layer Properties
// ============================================================================

void GeometryViewerWidget::setLayerVisible(const QString& layerName, bool visible) {
    if (layers_.contains(layerName)) {
        layers_[layerName].visible = visible;
        update();
    }
}

void GeometryViewerWidget::setLayerColor(const QString& layerName, const QColor& color) {
    if (layers_.contains(layerName)) {
        layers_[layerName].color = color;
        update();
    }
}

void GeometryViewerWidget::setLayerLineWidth(const QString& layerName, int width) {
    if (layers_.contains(layerName)) {
        layers_[layerName].lineWidth = std::max(1, width);
        update();
    }
}

void GeometryViewerWidget::setLayerPointSize(const QString& layerName, int size) {
    if (layers_.contains(layerName)) {
        layers_[layerName].pointSize = std::max(1, size);
        update();
    }
}

// ============================================================================
// View Control
// ============================================================================

void GeometryViewerWidget::zoomExtents() {
    if (worldBounds_.isValid() && !worldBounds_.isEmpty()) {
        // Add some padding around the geometries
        QRectF paddedBounds = worldBounds_;
        double padding = std::max(paddedBounds.width(), paddedBounds.height()) * 0.05;
        paddedBounds.adjust(-padding, -padding, padding, padding);

        viewBounds_ = paddedBounds;
        updateViewTransform();
        update();
        emit viewChanged(viewBounds_);
    }
}

void GeometryViewerWidget::zoomToLayer(const QString& layerName) {
    if (!layers_.contains(layerName)) return;

    auto geometry = layers_[layerName].geometry;
    auto bounds = geometry->getBoundingBox();

    if (bounds.first.x != bounds.second.x || bounds.first.y != bounds.second.y) {
        QRectF layerBounds(QPointF(bounds.first.x, bounds.first.y),
                           QPointF(bounds.second.x, bounds.second.y));

        // Add padding
        double padding = std::max(layerBounds.width(), layerBounds.height()) * 0.05;
        layerBounds.adjust(-padding, -padding, padding, padding);

        viewBounds_ = layerBounds;
        updateViewTransform();
        update();
        emit viewChanged(viewBounds_);
    }
}

void GeometryViewerWidget::setBackgroundColor(const QColor& color) {
    backgroundColor_ = color;
    update();
}

// ============================================================================
// OpenGL Implementation
// ============================================================================

void GeometryViewerWidget::initializeGL() {
    initializeOpenGLFunctions();
    glClearColor(backgroundColor_.redF(), backgroundColor_.greenF(),
                 backgroundColor_.blueF(), backgroundColor_.alphaF());
}

void GeometryViewerWidget::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    updateViewTransform();
}

void GeometryViewerWidget::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw each visible layer
    for (const auto& layer : layers_) {
        if (!layer.visible) continue;

        if (layer.geometry->getGeometryType() == "LineString") {
            auto* polyline = dynamic_cast<const Polyline*>(layer.geometry.get());
            if (polyline) {
                drawPolyline(painter, polyline, layer);
            }
        }
        else if (layer.geometry->getGeometryType() == "MultiLineString") {
            auto* polylineSet = dynamic_cast<const PolylineSet*>(layer.geometry.get());
            if (polylineSet) {
                drawPolylineSet(painter, polylineSet, layer);
            }
        }
    }

    // Draw interaction overlays
    if (currentMode_ == ZoomWindow && isInteracting_) {
        drawZoomRectangle(painter);
    }
}

// ============================================================================
// Mouse Events
// ============================================================================

void GeometryViewerWidget::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isInteracting_ = true;
        lastMousePos_ = event->position().toPoint();

        if (currentMode_ == ZoomWindow) {
            zoomStartPos_ = event->position().toPoint();
            zoomEndPos_ = zoomStartPos_;
        }

        setCursor(currentMode_ == Pan ? Qt::ClosedHandCursor : Qt::CrossCursor);
    }
    else if (event->button() == Qt::RightButton) {
        // Right-click context menu could go here
        QPointF worldPoint = screenToWorld(event->position().toPoint());
        emit geometryClicked("", worldPoint);
    }
}

void GeometryViewerWidget::mouseMoveEvent(QMouseEvent* event) {
    QPointF currentPos = event->position().toPoint();

    if (isInteracting_ && event->buttons() & Qt::LeftButton) {
        if (currentMode_ == Pan) {
            QPointF delta = currentPos - lastMousePos_;
            panView(delta);
        }
        else if (currentMode_ == ZoomWindow) {
            zoomEndPos_ = currentPos;
            update();
        }
    }

    lastMousePos_ = currentPos;
}

void GeometryViewerWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isInteracting_ = false;
        setCursor(Qt::ArrowCursor);

        if (currentMode_ == ZoomWindow) {
            applyZoomWindow();
        }
    }
}

void GeometryViewerWidget::wheelEvent(QWheelEvent* event) {
    double degrees = event->angleDelta().y() / 8.0;
    double steps = degrees / 15.0;
    double factor = std::pow(ZOOM_WHEEL_FACTOR, steps);

    QPointF center = event->position();
    zoomView(factor, center);
}

// ============================================================================
// Helper Methods
// ============================================================================

void GeometryViewerWidget::calculateWorldBounds() {
    if (layers_.isEmpty()) {
        worldBounds_ = QRectF();
        return;
    }

    bool first = true;
    double minX = 0, minY = 0, maxX = 0, maxY = 0;

    for (const auto& layer : layers_) {
        auto bounds = layer.geometry->getBoundingBox();

        if (first) {
            minX = bounds.first.x;
            minY = bounds.first.y;
            maxX = bounds.second.x;
            maxY = bounds.second.y;
            first = false;
        } else {
            minX = std::min(minX, bounds.first.x);
            minY = std::min(minY, bounds.first.y);
            maxX = std::max(maxX, bounds.second.x);
            maxY = std::max(maxY, bounds.second.y);
        }
    }

    worldBounds_ = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));

    // Initialize view bounds if not set
    if (!viewBounds_.isValid()) {
        viewBounds_ = worldBounds_;
    }
}

void GeometryViewerWidget::updateViewTransform() {
    if (!viewBounds_.isValid() || viewBounds_.isEmpty()) return;

    // Calculate scale and offset to fit viewBounds in widget
    double scaleX = width() / viewBounds_.width();
    double scaleY = height() / viewBounds_.height();
    scaleFactor_ = std::min(scaleX, scaleY);

    // Calculate offset to center the view
    worldOffset_ = QPointF(
        (width() - viewBounds_.width() * scaleFactor_) / 2.0 - viewBounds_.left() * scaleFactor_,
        (height() - viewBounds_.height() * scaleFactor_) / 2.0 - viewBounds_.top() * scaleFactor_
        );
}

QPointF GeometryViewerWidget::screenToWorld(const QPointF& screenPoint) const {
    return QPointF(
        (screenPoint.x() - worldOffset_.x()) / scaleFactor_,
        (screenPoint.y() - worldOffset_.y()) / scaleFactor_
        );
}

QPointF GeometryViewerWidget::worldToScreen(const QPointF& worldPoint) const {
    return QPointF(
        worldPoint.x() * scaleFactor_ + worldOffset_.x(),
        worldPoint.y() * scaleFactor_ + worldOffset_.y()
        );
}

QRectF GeometryViewerWidget::screenToWorld(const QRectF& screenRect) const {
    QPointF topLeft = screenToWorld(screenRect.topLeft());
    QPointF bottomRight = screenToWorld(screenRect.bottomRight());
    return QRectF(topLeft, bottomRight);
}

// ============================================================================
// Drawing Methods
// ============================================================================

void GeometryViewerWidget::drawPolyline(QPainter& painter, const Polyline* polyline,
                                        const GeometryLayer& layer) {
    if (!polyline || polyline->empty()) return;

    painter.setPen(QPen(layer.color, layer.lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    QPolygonF screenLine;
    for (const auto& point : polyline->getEnhancedPoints()) {
        QPointF worldPoint(point.x, point.y);
        screenLine << worldToScreen(worldPoint);
    }

    if (screenLine.size() > 1) {
        painter.drawPolyline(screenLine);
    }

    // Optionally draw points as well
    if (layer.pointSize > 0 && screenLine.size() > 0) {
        painter.setBrush(layer.color);
        painter.setPen(Qt::NoPen);

        for (const QPointF& screenPoint : screenLine) {
            painter.drawEllipse(screenPoint, layer.pointSize, layer.pointSize);
        }
    }
}

void GeometryViewerWidget::drawPolylineSet(QPainter& painter, const PolylineSet* polylineSet,
                                           const GeometryLayer& layer) {
    if (!polylineSet || polylineSet->empty()) return;

    painter.setPen(QPen(layer.color, layer.lineWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);

    for (size_t i = 0; i < polylineSet->size(); ++i) {
        const Polyline& polyline = polylineSet->getPolyline(i);

        QPolygonF screenLine;
        for (const auto& point : polyline.getEnhancedPoints()) {
            QPointF worldPoint(point.x, point.y);
            screenLine << worldToScreen(worldPoint);
        }

        if (screenLine.size() > 1) {
            painter.drawPolyline(screenLine);
        }

        // Optionally draw points
        if (layer.pointSize > 0) {
            painter.setBrush(layer.color);
            painter.setPen(Qt::NoPen);

            for (const QPointF& screenPoint : screenLine) {
                painter.drawEllipse(screenPoint, layer.pointSize, layer.pointSize);
            }

            painter.setBrush(Qt::NoBrush);
            painter.setPen(QPen(layer.color, layer.lineWidth));
        }
    }
}

void GeometryViewerWidget::drawZoomRectangle(QPainter& painter) {
    painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
    painter.setBrush(QBrush(Qt::blue, Qt::DiagCrossPattern));
    painter.setOpacity(0.3);

    QRectF zoomRect(zoomStartPos_, zoomEndPos_);
    painter.drawRect(zoomRect.normalized());

    painter.setOpacity(1.0);
}

// ============================================================================
// Utility Methods
// ============================================================================

QColor GeometryViewerWidget::generateRandomColor() {
    return QColor(
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256),
        QRandomGenerator::global()->bounded(256)
        );
}

void GeometryViewerWidget::panView(const QPointF& delta) {
    QPointF worldDelta = QPointF(delta.x() / scaleFactor_, delta.y() / scaleFactor_);
    viewBounds_.translate(-worldDelta.x(), -worldDelta.y());

    updateViewTransform();
    update();
    emit viewChanged(viewBounds_);
}

void GeometryViewerWidget::zoomView(double factor, const QPointF& center) {
    factor = std::clamp(factor, MIN_ZOOM / scaleFactor_, MAX_ZOOM / scaleFactor_);

    QPointF worldCenter = center.isNull() ?
                              QPointF(width() / 2.0, height() / 2.0) : center;
    QPointF worldPoint = screenToWorld(worldCenter);

    // Scale the view bounds
    QPointF boundsCenter = viewBounds_.center();
    QSizeF newSize(viewBounds_.width() / factor, viewBounds_.height() / factor);

    // Keep the point under the cursor fixed
    QPointF offset = worldPoint - boundsCenter;
    offset /= factor;

    viewBounds_ = QRectF(
        boundsCenter - offset - QPointF(newSize.width() / 2, newSize.height() / 2),
        newSize
        );

    updateViewTransform();
    update();
    emit viewChanged(viewBounds_);
}

void GeometryViewerWidget::applyZoomWindow() {
    QRectF zoomRect(zoomStartPos_, zoomEndPos_);
    zoomRect = zoomRect.normalized();

    // Ensure zoom rectangle is large enough
    if (zoomRect.width() < MIN_ZOOM_RECT_SIZE || zoomRect.height() < MIN_ZOOM_RECT_SIZE) {
        return;
    }

    // Convert to world coordinates
    viewBounds_ = screenToWorld(zoomRect);

    updateViewTransform();
    update();
    emit viewChanged(viewBounds_);
}

﻿#pragma once
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector>
#include <QPointF>
#include <QMouseEvent>
#include <QRectF>
#include "GeoDataSetInterface.h"
#include "PointGeoDataSet.h"
#include <QRandomGenerator>
#include <QMatrix4x4>
#include <QPainter>
#include <QTransform>

class OpenGLGeoWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit OpenGLGeoWidget(QWidget* parent = nullptr)
        : QOpenGLWidget(parent), isZooming(false) {
        calculateBoundingBox();  // Compute initial bounding box
    }

    void plotGeoDataEntries(GeoDataSetInterface *entries, const QString& attributeKey) {
        geoDataEntries = entries;
        assignColorsByAttribute(attributeKey);
        calculateBoundingBox();  // Compute initial bounding box
        update();  // Refresh OpenGL rendering
    }

    // Zoom Extents: Reset view to fit all points
    void zoomExtents() {
        viewBox = boundingBox;
        update();
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glClearColor(1.0, 1.0, 1.0, 1.0);
    }

    void resizeGL(int w, int h) override {
        glViewport(0, 0, w, h);
        updateProjection();
    }

    void paintGL() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        for (const GeoDataEntry& entry : *geoDataEntries) {
            QColor color = attributeColorMap.value(entry.attributes.value(selectedAttribute, "").toString(), Qt::black);
            painter.setBrush(color);
            painter.setPen(Qt::NoPen);

            for (const QPointF& point : entry.location) {
                QPointF screenPoint = mapToScreen(point);
                painter.drawEllipse(screenPoint, 3, 3);  // ✅ Qt-based drawing
            }
        }

        if (isZooming) {
            drawZoomRectangle(painter);
        }
    }



    // Mouse press for Zoom Window
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton) {
            isZooming = true;
            zoomStart = event->pos();
            zoomEnd = zoomStart;
        }
    }

    // Mouse move for updating Zoom Rectangle
    void mouseMoveEvent(QMouseEvent* event) override {
        if (isZooming) {
            zoomEnd = event->pos();
            update();
        }
    }

    // Mouse release for applying Zoom Window
    void mouseReleaseEvent(QMouseEvent* event) override {
        if (event->button() == Qt::LeftButton && isZooming) {
            isZooming = false;
            applyZoomWindow();
        }
    }

private:
    GeoDataSetInterface *geoDataEntries;
    QRectF boundingBox;   // Bounding box of all points
    QRectF viewBox;       // Current viewport
    QPoint zoomStart, zoomEnd;  // Points for zoom window selection
    QString selectedAttribute;  // Attribute used for color coding
    QMap<QString, QColor> attributeColorMap;  // Maps attributes to colors
    QMatrix4x4 projectionMatrix;
    bool isZooming;

    // Calculate bounding box of all points
    void calculateBoundingBox() {
        if (!geoDataEntries) return;
        if (geoDataEntries->count()==0) return; 

        if (geoDataEntries->FeatureType == featuretype::Points)
            boundingBox = static_cast<PointGeoDataSet*>(geoDataEntries)->BoundingBox(); 
        viewBox = boundingBox;
    }

    // Update projection based on viewBox
    void updateProjection() {
        projectionMatrix.setToIdentity();  // ✅ Reset projection matrix
        projectionMatrix.ortho(viewBox.left(), viewBox.right(), viewBox.bottom(), viewBox.top(), -1, 1);  // ✅ Qt-based projection

        update();  
    }


    

    // Apply Zoom Window
    void applyZoomWindow() {
        QPointF p1 = mapToOpenGL(zoomStart);
        QPointF p2 = mapToOpenGL(zoomEnd);
        QRectF zoomRect = QRectF(p1, p2).normalized();

        qDebug() << "Zoom Start:" << zoomStart << "Zoom End:" << zoomEnd;
        qDebug() << "Zoom Rect in OpenGL:" << zoomRect;

        // Ensure the zoom region is valid (avoid extreme zoom)
        if (zoomRect.width() > 1e-3 && zoomRect.height() > 1e-3) {
            viewBox = zoomRect;  // 
            update();  // 
        }
        else {
            qDebug() << "Zoom region too small, ignoring zoom.";
        }
    }

    // Convert screen coordinates to OpenGL coordinates
    QPointF mapToOpenGL(const QPoint& point) {
        qreal xRatio = static_cast<qreal>(point.x()) / width();
        qreal yRatio = static_cast<qreal>(point.y()) / height();

        qreal x = viewBox.left() + xRatio * viewBox.width();
        qreal y = viewBox.bottom() - yRatio * viewBox.height();  // 

        qDebug() << "Mapped Screen Point" << point << "to OpenGL" << QPointF(x, y);
        return QPointF(x, y);
    }


    QPointF mapToScreen(const QPointF& worldPoint) {
        qreal x = (worldPoint.x() - viewBox.left()) / viewBox.width() * width();
        qreal y = (1.0 - (worldPoint.y() - viewBox.top()) / viewBox.height()) * height();  // ✅ Fix Y-axis flip

        return QPointF(x, y);
    }

        

    // Draw Zoom Selection Rectangle
    void drawZoomRectangle(QPainter& painter) {
        painter.setPen(Qt::blue);
        painter.drawRect(QRectF(zoomStart, zoomEnd));
    }

    void assignColorsByAttribute(const QString& attributeKey) {
        selectedAttribute = attributeKey;
        attributeColorMap.clear();

        for (const GeoDataEntry& entry : *geoDataEntries) {
            QString attributeValue = entry.attributes.value(attributeKey, "").toString();
            if (!attributeColorMap.contains(attributeValue)) {
                attributeColorMap[attributeValue] = generateRandomColor();
            }
        }
    }

    QColor generateRandomColor() {
        return QColor(QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256));
    }

};

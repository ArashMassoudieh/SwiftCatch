#pragma once
#ifndef OPENGLGEOWIDGET_H
#define OPENGLGEOWIDGET_H
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

    void plotGeoDataEntries(const QString LayerName, GeoDataSetInterface *entries, const QString& attributeKey) {
        geoDataCollection[LayerName] = entries;
        assignColorsByAttribute(LayerName,attributeKey);
        calculateBoundingBox();  // Compute initial bounding box
        viewBox = boundingBox;
        //qDebug() << "Initial bounding box: " << boundingBox;
        update();  // Refresh OpenGL rendering
    }

    // Zoom Extents: Reset view to fit all points
    void zoomExtents() {
        viewBox = boundingBox;
        //qDebug() << "Bounding box after Zoom Extend" << boundingBox;
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

    void paintGL() override {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);


        for (QMap<QString, GeoDataSetInterface*>::iterator it = geoDataCollection.begin(); it != geoDataCollection.end(); ++it)
        {
            if (it.value()->FeatureType==featuretype::Points)
            for (const GeoDataEntry& entry : *it.value()) {
                QColor color = attributeColorMap[it.key()].value(entry.attributes.value(selectedAttribute[it.key()], "").toString(), Qt::black);
                painter.setBrush(color);
                painter.setPen(Qt::NoPen);

                for (const QPointF& point : entry.location) {
                    QPointF screenPoint = mapToScreen(point);
                    painter.drawEllipse(screenPoint, 3, 3);  // ✅ Qt-based drawing
                }
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
    QMap<QString, GeoDataSetInterface*> geoDataCollection;
    QRectF boundingBox;   // Bounding box of all points
    QRectF viewBox;       // Current viewport
    QPoint zoomStart, zoomEnd;  // Points for zoom window selection
    QMap<QString, QString> selectedAttribute;  // Attribute used for color coding
    QMap<QString, QMap<QString, QColor>> attributeColorMap;  // Maps attributes to colors
    QMatrix4x4 projectionMatrix;
    bool isZooming;

    // Calculate bounding box of all points
    void calculateBoundingBox() {
        if (geoDataCollection.count()==0) return;


        qreal minX = 1e12;
        qreal maxX = -1e12;
        qreal minY = 1e12;
        qreal maxY = -1e12;

        for (QMap<QString, GeoDataSetInterface*>::iterator it = geoDataCollection.begin(); it != geoDataCollection.end(); ++it)
        {
            QRectF rect = it.value()->BoundingBox();
            if (rect.left() < minX) minX = rect.left();
            if (rect.right() > maxX) maxX = rect.right();
            if (rect.bottom() < minY) minY = rect.bottom();
            if (rect.top() > maxY) maxY = rect.top();

        }
        boundingBox = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));;
        
    }

    // Update projection based on viewBox
    void updateProjection() {
        projectionMatrix.setToIdentity();  // Reset projection matrix
        projectionMatrix.ortho(viewBox.left(), viewBox.right(), viewBox.bottom(), viewBox.top(), -1, 1);  // 

        update();  
    }


    

    // Apply Zoom Window
    void applyZoomWindow() {
        QPointF p1_clicked = mapToOpenGL(zoomStart);
        QPointF p2_clicked = mapToOpenGL(zoomEnd);
        QPointF p1 = QPointF(std::min(p1_clicked.x(), p2_clicked.x()), std::max(p1_clicked.y(), p2_clicked.y()));
        QPointF p2 = QPointF(std::max(p1_clicked.x(), p2_clicked.x()), std::min(p1_clicked.y(), p2_clicked.y()));
        QRectF zoomRect = QRectF(p1, p2);
        QRectF zoomRect_Clicked = QRectF(p1_clicked, p2_clicked);

        // Ensure the zoom region is valid (avoid extreme zoom)
        if (fabs(zoomRect.width()) > 1e-3 && fabs(zoomRect.height()) > 1e-3) {
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

        //qDebug() << "xRatio: " << xRatio << "yRatio: " << yRatio;
        qreal x = viewBox.left() + xRatio * viewBox.width();
        qreal y = viewBox.top() + yRatio * viewBox.height();  // 
        //qDebug() << "x: " << x << "y: " << y;
        //qDebug() << "Mapped Screen Point" << point << "to OpenGL" << QPointF(x, y);
        return QPointF(x, y);
    }


    QPointF mapToScreen(const QPointF& worldPoint) {
        qreal x = (worldPoint.x() - viewBox.left()) / viewBox.width() * width();
        qreal y = (worldPoint.y() - viewBox.top()) / viewBox.height() * height();  

        return QPointF(x, y);
    }

        

    // Draw Zoom Selection Rectangle
    void drawZoomRectangle(QPainter& painter) {
        painter.setPen(Qt::blue);
        painter.drawRect(QRectF(zoomStart, zoomEnd));
    }

    void assignColorsByAttribute(const QString &LayerName, const QString& attributeKey) {
        selectedAttribute[LayerName] = attributeKey;
        attributeColorMap[LayerName].clear();

        for (const GeoDataEntry& entry : *geoDataCollection[LayerName]) {
            QString attributeValue = entry.attributes.value(attributeKey, "").toString();
            if (!attributeColorMap[LayerName].contains(attributeValue)) {
                attributeColorMap[LayerName][attributeValue] = generateRandomColor();
            }
        }
    }

    QColor generateRandomColor() {
        return QColor(QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256),
            QRandomGenerator::global()->bounded(256));
    }

};
#endif

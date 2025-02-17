#include <QApplication>
#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPolygonItem>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QPen>
#include <QBrush>
#include <QDebug>
#include <QHBoxLayout>
#include <QGraphicsPathItem>

class MapViewer : public QGraphicsView {
    Q_OBJECT

public:
    explicit MapViewer(QWidget* parent = nullptr) : QGraphicsView(parent), selecting(false), panMode(true), zoomWindowMode(false) {
        scene = new QGraphicsScene(this);
        this->setScene(scene);
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        //loadGeoJSON("C:/Projects/SwiftCatch/us-states.json");  // Load GeoJSON file from resources
    }

    explicit MapViewer(const QString &LayerFileName, QWidget* parent = nullptr) : QGraphicsView(parent), selecting(false), panMode(true), zoomWindowMode(false) {
        scene = new QGraphicsScene(this);
        this->setScene(scene);
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        AddLayer(LayerFileName);
    }

    explicit MapViewer(const QStringList& LayerFileNames, QWidget* parent = nullptr) : QGraphicsView(parent), selecting(false), panMode(true), zoomWindowMode(false) {
        scene = new QGraphicsScene(this);
        this->setScene(scene);
        setRenderHint(QPainter::Antialiasing);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        for (const QString& item : LayerFileNames)
        {
            AddLayer(item);
        }
    }

    void loadGeoJSON(const QString& filePath) {
        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open GeoJSON file.";
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();

        if (!doc.isObject()) {
            qCritical() << "Invalid GeoJSON format.";
            return;
        }

        QJsonObject rootObj = doc.object();
        QJsonArray features = rootObj["features"].toArray();

        std::srand(QDateTime::currentMSecsSinceEpoch());

        for (const QJsonValue& feature : features) {
            QJsonObject featureObj = feature.toObject();
            QJsonObject geometry = featureObj["geometry"].toObject();
            QString type = geometry["type"].toString();

            if (type == "Polygon") {
                QJsonArray coordinates = geometry["coordinates"].toArray();
                drawPolygon(coordinates);
            }
            else if (type == "MultiPolygon") {
                QJsonArray polygons = geometry["coordinates"].toArray();
                for (const QJsonValue& polygon : polygons) {
                    drawPolygon(polygon.toArray());
                }
            }
        }
    }

    void drawPolygon(const QJsonArray& coordinates) {
        QPolygonF polygon;
        for (const QJsonValue& ring : coordinates) {
            QJsonArray points = ring.toArray();
            for (const QJsonValue& point : points) {
                QJsonArray coord = point.toArray();
                double lon = coord[0].toDouble();
                double lat = coord[1].toDouble();
                polygon << QPointF(mapLongitude(lon), mapLatitude(lat));
            }
        }

        QColor color(std::rand() % 256, std::rand() % 256, std::rand() % 256, 150);
        QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
        polygonItem->setBrush(QBrush(color));
        polygonItem->setPen(QPen(Qt::black, 1));

        scene->addItem(polygonItem);
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (!panMode && event->button() == Qt::LeftButton) {
            selecting = true;
            selectionStart = mapToScene(event->pos());
            selectionRect = new QGraphicsRectItem();
            selectionRect->setPen(QPen(Qt::red, 2));
            scene->addItem(selectionRect);
        }
        QGraphicsView::mousePressEvent(event);
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        QPointF currentPos = mapToScene(event->pos());

        // Convert current position to geographical coordinates
        double lon = reverseMapLongitude(currentPos.x());
        double lat = reverseMapLatitude(currentPos.y());

        emit mousePositionUpdated(lon, lat);

        if (selecting) {
            QRectF rect(selectionStart, currentPos);
            selectionRect->setRect(rect);
        }
        QGraphicsView::mouseMoveEvent(event);
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (!panMode && event->button() == Qt::LeftButton && selecting) {
            selecting = false;
            selectionEnd = mapToScene(event->pos());

            if (zoomWindowMode) {
                zoomIntoSelection();
                zoomWindowMode = false;  // Reset after zooming
            }
            else {
                double minX = reverseMapLongitude(selectionStart.x());
                double maxX = reverseMapLongitude(selectionEnd.x());
                double minY = reverseMapLatitude(selectionEnd.y());
                double maxY = reverseMapLatitude(selectionStart.y());

                emit boundingBoxSelected(minX, minY, maxX, maxY);
            }
        }
        QGraphicsView::mouseReleaseEvent(event);
    }

    void zoomIn() { scale(1.2, 1.2); }
    void zoomOut() { scale(0.8, 0.8); }
    void togglePanMode() {
        panMode = true;
        zoomWindowMode = false;
        setDragMode(QGraphicsView::ScrollHandDrag);
    }
    void toggleSelectMode() {
        panMode = false;
        zoomWindowMode = false;
        setDragMode(QGraphicsView::NoDrag);
    }
    void enableZoomWindowMode() {
        zoomWindowMode = true;
        panMode = false;
        setDragMode(QGraphicsView::NoDrag);
    }
    void zoomExtend() {
        if (!scene || scene->items().isEmpty())
            return;

        QRectF boundingBox = scene->itemsBoundingRect();  // Get bounding box of all items
        fitInView(boundingBox, Qt::KeepAspectRatio);       // Adjust the view to fit the scene
    }
    void AddLayer(const QJsonDocument& doc, const QString &attributeKey = "") {
        
        if (!doc.isObject()) {
            qCritical() << "Invalid GeoJSON format in file:";
            return;
        }

        QJsonObject rootObj = doc.object();
        QJsonArray features = rootObj["features"].toArray();
        QMap<QString, QColor> colorMap;
        QMap<double, QColor> colorMapNumeric;

        for (const QJsonValue& feature : features) {
            QJsonObject featureObj = feature.toObject();
            QJsonObject geometry = featureObj["geometry"].toObject();
            QString type = geometry["type"].toString();

            bool numeric_attribute = false;
            qDebug()<<featureObj["properties"].toObject()[attributeKey].type(); 
            if (featureObj["properties"].toObject()[attributeKey].isDouble())
                numeric_attribute = true; 
            QString attributeValueStr = featureObj["properties"].toObject()[attributeKey].toString();
            double attributeValue = featureObj["properties"].toObject()[attributeKey].toDouble();
            qDebug() << featureObj;
            qDebug() << featureObj["properties"].toObject()[attributeKey];
            // Generate or retrieve a random color for the attribute
            QColor featureColor = Qt::transparent;
            if (!numeric_attribute)
            {
                if (!colorMap.contains(attributeValueStr)) {
                    colorMap[attributeValueStr] = generateRandomColor();
                }
                featureColor = colorMap[attributeValueStr];
            }
            else
            {
                if (!colorMapNumeric.contains(attributeValue)) {
                    colorMapNumeric[attributeValue] = generateRandomColor();
                }
                featureColor = colorMapNumeric[attributeValue];
            }

            

            if (type == "Point") {
                QJsonArray coordinates = geometry["coordinates"].toArray();
                if (coordinates.size() == 2) {
                    double lon = coordinates[0].toDouble();
                    double lat = coordinates[1].toDouble();
                    AddPoint(lon, lat, featureColor);
                }
            }
            else if (type == "LineString") {
                QJsonArray coordinates = geometry["coordinates"].toArray();
                qDebug() << coordinates;
                AddPolyline(coordinates, featureColor);
            }
            else if (type == "MultiLineString") {
                QJsonArray coordinates = geometry["coordinates"].toArray();
                qDebug() << coordinates;
                AddMultiLineString(coordinates, featureColor);
            }
            else if (type == "Polygon") {
                QJsonArray coordinates = geometry["coordinates"].toArray();
                AddPolygon(coordinates, featureColor);
            }
        }
        zoomExtend(); 
    }

    void AddLayer(const QString& GeoJsonFileName, const QString &attributekey = "") {

        QFile file(GeoJsonFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical() << "Failed to open GeoJSON file:" << GeoJsonFileName;
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        AddLayer(doc,attributekey);

    }

    void AddPoint(double lon, double lat, const QColor& featureColor = Qt::transparent) {
        QPointF point(mapLongitude(lon), mapLatitude(lat));

        QGraphicsEllipseItem* marker = new QGraphicsEllipseItem(point.x() - 3, point.y() - 3, 6, 6);
        marker->setBrush(QBrush(Qt::red));
        if (featureColor != Qt::transparent)
            marker->setPen(QPen(featureColor, 1));
        else
            marker->setPen(QPen(Qt::green, 1));

        scene->addItem(marker);
    }
    void AddPolyline(const QJsonArray& coordinates, const QColor &featureColor = Qt::transparent) {
        if (coordinates.size() < 2) return; // Ensure at least two points
        
        qDebug() << coordinates;
        QVector<QPointF> path;
        bool first = true;

        for (const QJsonValue& point : coordinates) {
            QJsonArray coord = point.toArray();
            double lon = coord[0].toDouble();
            double lat = coord[1].toDouble();
            QPointF mappedPoint(mapLongitude(lon), mapLatitude(lat));
            path.append(mappedPoint);
        }
        QRectF boundingBox = scene->itemsBoundingRect();
        qDebug() << boundingBox;
        qreal width = fabs(double(boundingBox.topLeft().x() - boundingBox.bottomRight().x())/1000.0);
        for (int i = 0; i < path.size() - 1; ++i) {

            QGraphicsLineItem* lineItem = new QGraphicsLineItem(path[i].x(), path[i].y(), path[i + 1].x(), path[i + 1].y());
            if (featureColor != Qt::transparent)
                lineItem->setPen(QPen(featureColor, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                lineItem->setPen(QPen(Qt::blue, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            scene->addItem(lineItem);
        }
    }

    void AddMultiLineString(const QJsonArray& multiLineCoordinates, const QColor& featureColor = Qt::transparent) {
        
        if (multiLineCoordinates[0].toArray().size() < 2) return; // Ensure at least two points
        QJsonArray multilinecontents = multiLineCoordinates[0].toArray();
        //qDebug() << multilinecontents;
        QVector<QPointF> path;
        bool first = true;

        for (const QJsonValue& point : multilinecontents) {
            QJsonArray coord = point.toArray();
            double lon = coord[0].toDouble();
            double lat = coord[1].toDouble();
            QPointF mappedPoint(mapLongitude(lon), mapLatitude(lat));
            path.append(mappedPoint);
        }

        for (int i = 0; i < path.size() - 1; ++i) {
            
            QGraphicsLineItem* lineItem = new QGraphicsLineItem(path[i].x(), path[i].y(), path[i+1].x(), path[i+1].y());
            if (featureColor != Qt::transparent)
                lineItem->setPen(QPen(featureColor, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                lineItem->setPen(QPen(Qt::darkCyan, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            scene->addItem(lineItem);
        }
    }
    

    void AddPolygon(const QJsonArray& coordinates, const QColor& featureColor = Qt::transparent) {
        QPolygonF polygon;
        for (const QJsonValue& ring : coordinates) {
            QJsonArray points = ring.toArray();
            for (const QJsonValue& point : points) {
                QJsonArray coord = point.toArray();
                double lon = coord[0].toDouble();
                double lat = coord[1].toDouble();
                polygon << QPointF(mapLongitude(lon), mapLatitude(lat));
            }
        }

        QColor color(std::rand() % 256, std::rand() % 256, std::rand() % 256, 150);
        QGraphicsPolygonItem* polygonItem = new QGraphicsPolygonItem(polygon);
        polygonItem->setBrush(QBrush(color));
        if (featureColor != Qt::transparent)
            polygonItem->setPen(QPen(featureColor, 1));
        else
            polygonItem->setPen(QPen(Qt::red, 1));

        scene->addItem(polygonItem);
    }

    QColor generateRandomColor() {
        return QColor(std::rand() % 256, std::rand() % 256, std::rand() % 256);
    }

private:
    QGraphicsScene* scene;
    bool selecting;
    bool panMode;
    bool zoomWindowMode;
    QPointF selectionStart, selectionEnd;
    QGraphicsRectItem* selectionRect;

    void zoomIntoSelection() {
        if (!selectionRect)
            return;

        QRectF zoomArea = selectionRect->rect();
        fitInView(zoomArea, Qt::KeepAspectRatio);
        scene->removeItem(selectionRect);
        delete selectionRect;
        selectionRect = nullptr;
    }

    double mapLongitude(double lon) { return (lon + 125) * 10; }
    double mapLatitude(double lat) { return (50 - lat) * 10; }
    double reverseMapLongitude(double x) { return (x / 10) - 125; }
    double reverseMapLatitude(double y) { return 50 - (y / 10); }

signals:
    void boundingBoxSelected(double minX, double minY, double maxX, double maxY);
    void mousePositionUpdated(double lon, double lat);
};


class MapDialog : public QDialog {
    Q_OBJECT

public:
    explicit MapDialog(QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle("U.S. GeoJSON Map Picker");
        resize(900, 600);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        QHBoxLayout* buttonLayout = new QHBoxLayout();

        mapViewer = new MapViewer(this);
        mousePositionLabel = new QLabel("Mouse Position: N/A", this);
        infoLabel = new QLabel("Select a region on the map", this);

        QPushButton* zoomInButton = new QPushButton("Zoom In", this);
        QPushButton* zoomOutButton = new QPushButton("Zoom Out", this);
        QPushButton* panModeButton = new QPushButton("Pan Mode", this);
        QPushButton* selectBoxButton = new QPushButton("Select Box Mode", this);
        QPushButton* closeButton = new QPushButton("Close", this);
        QPushButton* zoomWindowButton = new QPushButton("Zoom Window", this);
        QPushButton* zoomExtends = new QPushButton("Zoom Extends", this);

        buttonLayout->addWidget(zoomInButton);
        buttonLayout->addWidget(zoomOutButton);
        buttonLayout->addWidget(zoomWindowButton);
        buttonLayout->addWidget(zoomExtends);
        buttonLayout->addWidget(panModeButton);
        buttonLayout->addWidget(selectBoxButton);
        buttonLayout->addWidget(closeButton);


        mainLayout->addWidget(mapViewer);
        mainLayout->addWidget(mousePositionLabel);
        mainLayout->addWidget(infoLabel);
        mainLayout->addLayout(buttonLayout);

        connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
        connect(zoomInButton, &QPushButton::clicked, mapViewer, &MapViewer::zoomIn);
        connect(zoomOutButton, &QPushButton::clicked, mapViewer, &MapViewer::zoomOut);
        connect(panModeButton, &QPushButton::clicked, mapViewer, &MapViewer::togglePanMode);
        connect(selectBoxButton, &QPushButton::clicked, mapViewer, &MapViewer::toggleSelectMode);
        connect(mapViewer, &MapViewer::boundingBoxSelected, this, &MapDialog::onBoundingBoxSelected);
        connect(mapViewer, &MapViewer::mousePositionUpdated, this, &MapDialog::onMousePositionUpdated);
        connect(zoomWindowButton, &QPushButton::clicked, mapViewer, &MapViewer::enableZoomWindowMode);
        connect(zoomExtends, &QPushButton::clicked, mapViewer, &MapViewer::zoomExtend);
    }

    void AddLayer(const QJsonDocument& doc, const QString attributeKey = "")
    {
        mapViewer->AddLayer(doc,attributeKey);
    }

    void AddLayer(const QString& GeoJsonFileName, const QString &attributekey = "")
    {
        mapViewer->AddLayer(GeoJsonFileName,attributekey);
    }

private slots:
    void onBoundingBoxSelected(double minX, double minY, double maxX, double maxY) {
        infoLabel->setText(QString("Bounding Box: [%1, %2, %3, %4]").arg(minX).arg(minY).arg(maxX).arg(maxY));
    }

    void onMousePositionUpdated(double lon, double lat) {
        mousePositionLabel->setText(QString("Mouse Position: Lon: %1, Lat: %2").arg(lon).arg(lat));
    }
    


private:
    MapViewer* mapViewer;
    QLabel* infoLabel;
    QLabel* mousePositionLabel;
};



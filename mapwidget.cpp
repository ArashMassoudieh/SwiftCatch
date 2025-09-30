#include "mapwidget.h"
#include <QQmlContext>
#include <QQmlEngine>

MapWidget::MapWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    connectSignals();
}

void MapWidget::setupUI()
{
    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(this);

    // Create coordinate display label
    coordinateLabel_ = new QLabel("Click on map to get coordinates", this);
    coordinateLabel_->setStyleSheet("QLabel { padding: 5px; background-color: #f0f0f0; }");

    // Create QML map view
    mapView_ = new QQuickWidget(this);
    mapView_->setResizeMode(QQuickWidget::SizeRootObjectToView);
    mapView_->setSource(QUrl("qrc:/MapView.qml"));

    // Add to layout
    layout->addWidget(coordinateLabel_);
    layout->addWidget(mapView_, 1); // Give map most of the space

    setLayout(layout);

    // Add debug output
    connect(mapView_->engine(), &QQmlEngine::warnings,
            [](const QList<QQmlError> &warnings) {
                for (const auto &warning : warnings) {
                    qDebug() << "QML Warning:" << warning.toString();
                }
            });

    mapView_->setSource(QUrl("qrc:/MapView.qml"));

    // Check loading status
    connect(mapView_, &QQuickWidget::statusChanged, [this](QQuickWidget::Status status) {
        switch(status) {
        case QQuickWidget::Ready:
            qDebug() << "QML loaded successfully";
            break;
        case QQuickWidget::Error:
            qDebug() << "QML loading failed";
            for (const auto &error : mapView_->errors()) {
                qDebug() << "Error:" << error.toString();
            }
            break;
        default:
            qDebug() << "QML status:" << status;
        }
    });
}

void MapWidget::connectSignals()
{
    // QQuickItem inherits from QObject, so no cast needed
    QObject* qmlObject = mapView_->rootObject();
    if (qmlObject) {
        connect(qmlObject, SIGNAL(coordinateClicked(double, double)),
                this, SLOT(onMapClicked(double, double)));
    }
}

void MapWidget::onMapClicked(double latitude, double longitude)
{
    // Update coordinate display
    coordinateLabel_->setText(QString("Clicked: Lat: %1, Lon: %2")
                                  .arg(latitude, 0, 'f', 6)
                                  .arg(longitude, 0, 'f', 6));

    // Show marker on map
    QObject* qmlObject = mapView_->rootObject();
    if (qmlObject) {
        QMetaObject::invokeMethod(qmlObject, "showMarker",
                                  Q_ARG(QVariant, latitude),
                                  Q_ARG(QVariant, longitude));
    }

    // Emit signals for external use
    emit coordinateClicked(longitude, latitude); // Note: X=longitude, Y=latitude
    emit coordinateChanged(longitude, latitude);
}

void MapWidget::setCenter(double latitude, double longitude)
{
    QObject* qmlObject = mapView_->rootObject();
    if (qmlObject) {
        QMetaObject::invokeMethod(qmlObject, "setCenter",
                                  Q_ARG(QVariant, latitude),
                                  Q_ARG(QVariant, longitude));
    }
}

void MapWidget::setZoomLevel(int zoom)
{
    QObject* qmlObject = mapView_->rootObject();
    if (qmlObject) {
        QMetaObject::invokeMethod(qmlObject, "setZoomLevel",
                                  Q_ARG(QVariant, zoom));
    }
}

#pragma once
#include <QWidget>
#include <QQuickWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QQuickItem>

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);

    // Methods to interact with map
    void setCenter(double latitude, double longitude);
    void setZoomLevel(int zoom);

signals:
    void coordinateClicked(double x, double y);
    void coordinateChanged(double x, double y);

private slots:
    void onMapClicked(double latitude, double longitude);

private:
    QQuickWidget* mapView_;
    QLabel* coordinateLabel_;
    void setupUI();
    void connectSignals();
};

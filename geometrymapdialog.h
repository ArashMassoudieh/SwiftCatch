#ifndef GEOMETRYMAPDIALOG_H
#define GEOMETRYMAPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QCheckBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QGroupBox>
#include <QListWidget>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <memory>

#include "geomertymapviewer.h"
#include "geometrybase.h"
#include "polyline.h"
#include "polylineset.h"

class GeometryMapDialog : public QDialog {
    Q_OBJECT

public:
    explicit GeometryMapDialog(QWidget* parent = nullptr);

    // Layer management
    void addGeometryLayer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                          const QColor& color = Qt::blue, int lineWidth = 2, int pointSize = 4,
                          const QString& attributeKey = "");

    // Convenience methods for loading from files
    void loadPolylineFromShapefile(const QString& layerName, const QString& filename);
    void loadPolylineSetFromShapefile(const QString& layerName, const QString& filename);
    void loadFromGeoJSON(const QString& layerName, const QString& filename);

private slots:
    // Navigation controls
    void onZoomIn();
    void onZoomOut();
    void onZoomExtent();
    void onZoomWindow();
    void onPanMode();
    void onSelectMode();

    // Layer controls
    void onLayerVisibilityChanged();
    void onLayerColorChanged();
    void onLineWidthChanged(int width);
    void onPointSizeChanged(int size);
    void onLayerSelectionChanged();

    // Map interactions
    void onBoundingBoxSelected(double minX, double minY, double maxX, double maxY);
    void onGeometryClicked(const QString& layerName, double x, double y);
    void onMousePositionUpdated(double x, double y);

    // File operations
    void onLoadShapefile();
    void onLoadGeoJSON();
    void onExportCurrentView();

private:
    // UI setup
    void setupUI();
    void setupNavigationControls();
    void setupLayerControls();
    void connectSignals();
    void updateLayerList();
    void updateLayerControls();

    // UI components
    GeometryMapViewer* mapViewer_;

    // Navigation controls
    QPushButton* zoomInBtn_;
    QPushButton* zoomOutBtn_;
    QPushButton* zoomExtentBtn_;
    QPushButton* zoomWindowBtn_;
    QPushButton* panModeBtn_;
    QPushButton* selectModeBtn_;

    // Layer controls
    QListWidget* layerList_;
    QCheckBox* visibilityCheck_;
    QPushButton* colorBtn_;
    QSpinBox* lineWidthSpin_;
    QSpinBox* pointSizeSpin_;
    QComboBox* lineStyleCombo_;

    // Information displays
    QLabel* mousePositionLabel_;
    QLabel* selectionLabel_;
    QLabel* layerInfoLabel_;

    // File operations
    QPushButton* loadShapefileBtn_;
    QPushButton* loadGeoJSONBtn_;
    QPushButton* exportBtn_;
    QPushButton* closeBtn_;

    // State
    QString currentSelectedLayer_;
    QMap<QString, QColor> layerColors_;
};

#endif // GEOMETRYMAPDIALOG_H

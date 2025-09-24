#include "geometrymapdialog.h"

GeometryMapDialog::GeometryMapDialog(QWidget* parent) : QDialog(parent) {
    setupUI();
    connectSignals();

    setWindowTitle("Geometry Map Viewer");
    resize(1200, 800);
}

// ============================================================================
// Public Methods
// ============================================================================

void GeometryMapDialog::addGeometryLayer(const QString& layerName, std::shared_ptr<GeometryBase> geometry,
                                         const QColor& color, int lineWidth, int pointSize,
                                         const QString& attributeKey) {
    mapViewer_->addGeometryLayer(layerName, geometry, color, lineWidth, pointSize, attributeKey);
    layerColors_[layerName] = color;
    updateLayerList();
    updateLayerControls();
}

void GeometryMapDialog::loadPolylineFromShapefile(const QString& layerName, const QString& filename) {
    try {
        auto polyline = std::make_shared<Polyline>();
        // Note: You'll need to implement this loading method in Polyline class
        // polyline->loadFromShapefile(filename);
        addGeometryLayer(layerName, polyline);
    } catch (const std::exception& e) {
        qWarning() << "Failed to load polyline from" << filename << ":" << e.what();
    }
}

void GeometryMapDialog::loadPolylineSetFromShapefile(const QString& layerName, const QString& filename) {
    try {
        auto polylineSet = std::make_shared<PolylineSet>();
        polylineSet->loadFromShapefile(filename);
        addGeometryLayer(layerName, polylineSet);
    } catch (const std::exception& e) {
        qWarning() << "Failed to load polyline set from" << filename << ":" << e.what();
    }
}

void GeometryMapDialog::loadFromGeoJSON(const QString& layerName, const QString& filename) {
    try {
        auto polylineSet = std::make_shared<PolylineSet>();
        polylineSet->loadFromGeoJSON(filename);
        addGeometryLayer(layerName, polylineSet);
    } catch (const std::exception& e) {
        qWarning() << "Failed to load from GeoJSON" << filename << ":" << e.what();
    }
}

// ============================================================================
// Private Slots - Navigation
// ============================================================================

void GeometryMapDialog::onZoomIn() {
    mapViewer_->zoomIn();
}

void GeometryMapDialog::onZoomOut() {
    mapViewer_->zoomOut();
}

void GeometryMapDialog::onZoomExtent() {
    mapViewer_->zoomExtent();
}

void GeometryMapDialog::onZoomWindow() {
    mapViewer_->enableZoomWindowMode();
    zoomWindowBtn_->setChecked(true);
    panModeBtn_->setChecked(false);
    selectModeBtn_->setChecked(false);
}

void GeometryMapDialog::onPanMode() {
    mapViewer_->togglePanMode();
    panModeBtn_->setChecked(true);
    selectModeBtn_->setChecked(false);
    zoomWindowBtn_->setChecked(false);
}

void GeometryMapDialog::onSelectMode() {
    mapViewer_->toggleSelectMode();
    selectModeBtn_->setChecked(true);
    panModeBtn_->setChecked(false);
    zoomWindowBtn_->setChecked(false);
}

// ============================================================================
// Private Slots - Layer Controls
// ============================================================================

void GeometryMapDialog::onLayerSelectionChanged() {
    currentSelectedLayer_ = layerList_->currentItem() ? layerList_->currentItem()->text() : QString();
    updateLayerControls();
}

void GeometryMapDialog::onLayerVisibilityChanged() {
    if (!currentSelectedLayer_.isEmpty()) {
        mapViewer_->setLayerVisible(currentSelectedLayer_, visibilityCheck_->isChecked());
    }
}

void GeometryMapDialog::onLayerColorChanged() {
    if (currentSelectedLayer_.isEmpty()) return;

    QColor currentColor = layerColors_.value(currentSelectedLayer_, Qt::blue);
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Layer Color");

    if (newColor.isValid()) {
        mapViewer_->setLayerColor(currentSelectedLayer_, newColor);
        layerColors_[currentSelectedLayer_] = newColor;
        colorBtn_->setStyleSheet(QString("background-color: %1;").arg(newColor.name()));
    }
}

void GeometryMapDialog::onLineWidthChanged(int width) {
    if (!currentSelectedLayer_.isEmpty()) {
        mapViewer_->setLayerLineWidth(currentSelectedLayer_, width);
    }
}

void GeometryMapDialog::onPointSizeChanged(int size) {
    if (!currentSelectedLayer_.isEmpty()) {
        mapViewer_->setLayerPointSize(currentSelectedLayer_, size);
    }
}

// ============================================================================
// Private Slots - Map Interactions
// ============================================================================

void GeometryMapDialog::onBoundingBoxSelected(double minX, double minY, double maxX, double maxY) {
    selectionLabel_->setText(QString("Selection: [%.2f, %.2f] to [%.2f, %.2f]")
                                 .arg(minX).arg(minY).arg(maxX).arg(maxY));
}

void GeometryMapDialog::onGeometryClicked(const QString& layerName, double x, double y) {
    selectionLabel_->setText(QString("Clicked %1 at (%1, %2)")
                                 .arg(layerName.isEmpty() ? "map" : layerName).arg(x).arg(y));
}

void GeometryMapDialog::onMousePositionUpdated(double x, double y) {
    mousePositionLabel_->setText(QString("Mouse: (%1, %2)").arg(x).arg(y));
}

// ============================================================================
// Private Slots - File Operations
// ============================================================================

void GeometryMapDialog::onLoadShapefile() {
    QString filename = QFileDialog::getOpenFileName(
        this, "Load Shapefile", "", "Shapefiles (*.shp);;All Files (*)");

    if (!filename.isEmpty()) {
        QString layerName = QFileInfo(filename).baseName();
        loadPolylineSetFromShapefile(layerName, filename);
    }
}

void GeometryMapDialog::onLoadGeoJSON() {
    QString filename = QFileDialog::getOpenFileName(
        this, "Load GeoJSON", "", "GeoJSON Files (*.geojson *.json);;All Files (*)");

    if (!filename.isEmpty()) {
        QString layerName = QFileInfo(filename).baseName();
        loadFromGeoJSON(layerName, filename);
    }
}

void GeometryMapDialog::onExportCurrentView() {
    QString filename = QFileDialog::getSaveFileName(
        this, "Export View", "", "PNG Images (*.png);;PDF Files (*.pdf);;All Files (*)");

    if (!filename.isEmpty()) {
        // Export current view as image
        QPixmap pixmap = mapViewer_->grab();
        pixmap.save(filename);
    }
}

// ============================================================================
// UI Setup Methods
// ============================================================================

void GeometryMapDialog::setupUI() {
    auto* mainLayout = new QHBoxLayout(this);

    // Create map viewer
    mapViewer_ = new GeometryMapViewer(this);
    mapViewer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Create control panel
    auto* controlPanel = new QWidget(this);
    controlPanel->setFixedWidth(300);
    controlPanel->setStyleSheet("QWidget { background-color: #f5f5f5; }");

    auto* controlLayout = new QVBoxLayout(controlPanel);

    // Navigation controls
    setupNavigationControls();
    auto* navGroup = new QGroupBox("Navigation", this);
    auto* navLayout = new QVBoxLayout(navGroup);

    auto* zoomLayout = new QHBoxLayout();
    zoomInBtn_ = new QPushButton("Zoom In", this);
    zoomOutBtn_ = new QPushButton("Zoom Out", this);
    zoomLayout->addWidget(zoomInBtn_);
    zoomLayout->addWidget(zoomOutBtn_);

    zoomExtentBtn_ = new QPushButton("Zoom Extent", this);
    zoomWindowBtn_ = new QPushButton("Zoom Window", this);

    auto* modeLayout = new QHBoxLayout();
    panModeBtn_ = new QPushButton("Pan", this);
    selectModeBtn_ = new QPushButton("Select", this);
    panModeBtn_->setCheckable(true);
    selectModeBtn_->setCheckable(true);
    panModeBtn_->setChecked(true);
    modeLayout->addWidget(panModeBtn_);
    modeLayout->addWidget(selectModeBtn_);

    navLayout->addLayout(zoomLayout);
    navLayout->addWidget(zoomExtentBtn_);
    navLayout->addWidget(zoomWindowBtn_);
    navLayout->addLayout(modeLayout);

    // Layer controls
    setupLayerControls();
    auto* layerGroup = new QGroupBox("Layers", this);
    auto* layerLayout = new QVBoxLayout(layerGroup);

    layerList_ = new QListWidget(this);
    layerList_->setMaximumHeight(150);

    auto* layerPropsLayout = new QVBoxLayout();
    visibilityCheck_ = new QCheckBox("Visible", this);

    auto* colorLayout = new QHBoxLayout();
    colorLayout->addWidget(new QLabel("Color:", this));
    colorBtn_ = new QPushButton(this);
    colorBtn_->setFixedSize(30, 20);
    colorBtn_->setStyleSheet("background-color: blue;");
    colorLayout->addWidget(colorBtn_);
    colorLayout->addStretch();

    auto* widthLayout = new QHBoxLayout();
    widthLayout->addWidget(new QLabel("Line Width:", this));
    lineWidthSpin_ = new QSpinBox(this);
    lineWidthSpin_->setRange(1, 20);
    lineWidthSpin_->setValue(2);
    widthLayout->addWidget(lineWidthSpin_);

    auto* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("Point Size:", this));
    pointSizeSpin_ = new QSpinBox(this);
    pointSizeSpin_->setRange(0, 20);
    pointSizeSpin_->setValue(4);
    sizeLayout->addWidget(pointSizeSpin_);

    layerLayout->addWidget(layerList_);
    layerLayout->addWidget(visibilityCheck_);
    layerLayout->addLayout(colorLayout);
    layerLayout->addLayout(widthLayout);
    layerLayout->addLayout(sizeLayout);

    // File operations
    auto* fileGroup = new QGroupBox("File Operations", this);
    auto* fileLayout = new QVBoxLayout(fileGroup);

    loadShapefileBtn_ = new QPushButton("Load Shapefile", this);
    loadGeoJSONBtn_ = new QPushButton("Load GeoJSON", this);
    exportBtn_ = new QPushButton("Export View", this);

    fileLayout->addWidget(loadShapefileBtn_);
    fileLayout->addWidget(loadGeoJSONBtn_);
    fileLayout->addWidget(exportBtn_);

    // Information display
    auto* infoGroup = new QGroupBox("Information", this);
    auto* infoLayout = new QVBoxLayout(infoGroup);

    mousePositionLabel_ = new QLabel("Mouse: (0, 0)", this);
    selectionLabel_ = new QLabel("No selection", this);
    layerInfoLabel_ = new QLabel("No layers", this);

    infoLayout->addWidget(mousePositionLabel_);
    infoLayout->addWidget(selectionLabel_);
    infoLayout->addWidget(layerInfoLabel_);

    // Close button
    closeBtn_ = new QPushButton("Close", this);

    // Add to control layout
    controlLayout->addWidget(navGroup);
    controlLayout->addWidget(layerGroup);
    controlLayout->addWidget(fileGroup);
    controlLayout->addWidget(infoGroup);
    controlLayout->addStretch();
    controlLayout->addWidget(closeBtn_);

    // Add to main layout
    mainLayout->addWidget(mapViewer_, 1);
    mainLayout->addWidget(controlPanel);
}

void GeometryMapDialog::setupNavigationControls() {
    // Navigation controls are created in setupUI
}

void GeometryMapDialog::setupLayerControls() {
    // Layer controls are created in setupUI
}

void GeometryMapDialog::connectSignals() {
    // Navigation
    connect(zoomInBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onZoomIn);
    connect(zoomOutBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onZoomOut);
    connect(zoomExtentBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onZoomExtent);
    connect(zoomWindowBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onZoomWindow);
    connect(panModeBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onPanMode);
    connect(selectModeBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onSelectMode);

    // Layer controls
    connect(layerList_, &QListWidget::currentTextChanged, this, &GeometryMapDialog::onLayerSelectionChanged);
    connect(visibilityCheck_, &QCheckBox::toggled, this, &GeometryMapDialog::onLayerVisibilityChanged);
    connect(colorBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onLayerColorChanged);
    connect(lineWidthSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &GeometryMapDialog::onLineWidthChanged);
    connect(pointSizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged), this, &GeometryMapDialog::onPointSizeChanged);

    // File operations
    connect(loadShapefileBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onLoadShapefile);
    connect(loadGeoJSONBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onLoadGeoJSON);
    connect(exportBtn_, &QPushButton::clicked, this, &GeometryMapDialog::onExportCurrentView);

    // Map viewer signals
    connect(mapViewer_, &GeometryMapViewer::boundingBoxSelected, this, &GeometryMapDialog::onBoundingBoxSelected);
    connect(mapViewer_, &GeometryMapViewer::geometryClicked, this, &GeometryMapDialog::onGeometryClicked);
    connect(mapViewer_, &GeometryMapViewer::mousePositionUpdated, this, &GeometryMapDialog::onMousePositionUpdated);

    // Close button
    connect(closeBtn_, &QPushButton::clicked, this, &QDialog::accept);
}

// ============================================================================
// Helper Methods
// ============================================================================

void GeometryMapDialog::updateLayerList() {
    layerList_->clear();

    // This would need to be implemented to get layer names from the viewer
    // For now, we'll track them in layerColors_
    for (auto it = layerColors_.begin(); it != layerColors_.end(); ++it) {
        layerList_->addItem(it.key());
    }

    layerInfoLabel_->setText(QString("Layers: %1").arg(layerList_->count()));
}

void GeometryMapDialog::updateLayerControls() {
    bool hasSelection = !currentSelectedLayer_.isEmpty();

    visibilityCheck_->setEnabled(hasSelection);
    colorBtn_->setEnabled(hasSelection);
    lineWidthSpin_->setEnabled(hasSelection);
    pointSizeSpin_->setEnabled(hasSelection);

    if (hasSelection) {
        QColor layerColor = layerColors_.value(currentSelectedLayer_, Qt::blue);
        colorBtn_->setStyleSheet(QString("background-color: %1;").arg(layerColor.name()));
        visibilityCheck_->setChecked(true); // Default to visible
    } else {
        colorBtn_->setStyleSheet("background-color: gray;");
    }
}


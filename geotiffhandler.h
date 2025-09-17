#ifndef GEOTIFFHANDLER_H
#define GEOTIFFHANDLER_H

#include <string>
#include <vector>
#include <gdal_priv.h>
#include <QString>
#include <QFileDialog>
#include <QWidget>
#include "node.h"

/**
 * @class GeoTiffHandler
 * @brief A utility class for reading and processing GeoTIFF raster files using GDAL.
 *
 * This class loads raster data from a GeoTIFF file and provides access to
 * pixel values, spatial coordinates, and metadata. Data are stored in both
 * a flat 1D vector and a 2D grid for convenience.
 */

class Path;

enum class FlowDirType { D4, D8 };

class GeoTiffHandler {
public:
    /**
     * @brief Constructor that loads a GeoTIFF file into memory.
     * @param filename Path to the GeoTIFF file.
     * @throw std::runtime_error if the file cannot be opened or read.
     */
    explicit GeoTiffHandler(const std::string& filename);
    GeoTiffHandler(int width, int height);
    GeoTiffHandler();
    // Custom copy constructor and assignment
    GeoTiffHandler(const GeoTiffHandler& other);
    GeoTiffHandler& operator=(const GeoTiffHandler& other);

    // Move constructor/assignment (default is fine)
    GeoTiffHandler(GeoTiffHandler&&) = default;
    GeoTiffHandler& operator=(GeoTiffHandler&&) = default;

    /**
     * @brief Destructor. Closes the GDAL dataset.
     */
    ~GeoTiffHandler();

    /** @name Raster Information */
    ///@{
    /**
     * @brief Get raster width (number of columns).
     * @return Width in pixels.
     */
    int width() const;

    /**
     * @brief Get raster height (number of rows).
     * @return Height in pixels.
     */
    int height() const;

    /**
     * @brief Get number of raster bands.
     * @return Number of bands.
     */
    int bands() const;
    ///@}

    /** @name Data Access */
    ///@{
    /**
     * @brief Get the 1D raster data buffer.
     * @return Constant reference to the flat raster values (row-major order).
     */
    const std::vector<float>& data1D() const;

    /**
     * @brief Get the 2D raster data buffer.
     * @return Constant reference to 2D raster values [i][j],
     *         where i = column index (x), j = row index (y).
     */
    const std::vector<std::vector<double>>& data2D() const;
    ///@}

    /** @name Statistics */
    ///@{
    /**
     * @brief Compute the minimum raster value.
     * @return Minimum value in the raster.
     */
    float minValue() const;

    /**
     * @brief Compute the maximum raster value.
     * @return Maximum value in the raster.
     */
    float maxValue() const;
    ///@}

    /** @name GeoTransform */
    ///@{
    /**
     * @brief Get one element of the GDAL GeoTransform array.
     * @param idx Index of the transform element (0–5).
     * @return The requested transform value.
     * @throw std::runtime_error if GeoTransform is unavailable.
     *
     * GDAL GeoTransform array (gt):
     * - gt[0] = top left x
     * - gt[1] = w-e pixel resolution
     * - gt[2] = row rotation (typically 0)
     * - gt[3] = top left y
     * - gt[4] = column rotation (typically 0)
     * - gt[5] = n-s pixel resolution (negative for north-up)
     */
    double getGeoTransform(int idx) const;
    ///@}

    /** @name Data Processing */
    ///@{
    /**
     * @brief Normalize raster values to the [0, 1] range.
     *
     * Both the 1D and 2D buffers are updated.
     */
    void normalize();
    ///@}

    /** @name Coordinate Accessors */
    ///@{
    /**
     * @brief Get the x-coordinates of cell centers.
     * @return Constant reference to the x array.
     */
    const std::vector<double>& x() const;

    /**
     * @brief Set the x-coordinates manually.
     * @param x Vector of x values.
     */
    void setX(const std::vector<double>& x);

    /**
     * @brief Get the y-coordinates of cell centers.
     * @return Constant reference to the y array.
     */
    const std::vector<double>& y() const;

    /**
     * @brief Set the y-coordinates manually.
     * @param y Vector of y values.
     */
    void setY(const std::vector<double>& y);

    /**
     * @brief Get the cell size in x-direction.
     * @return Cell width (dx).
     */
    double dx() const;

    /**
     * @brief Set the cell size in x-direction.
     * @param dx Cell width.
     */
    void setDx(double dx);

    /**
     * @brief Get the cell size in y-direction.
     * @return Cell height (dy).
     */
    double dy() const;

    /**
     * @brief Set the cell size in y-direction.
     * @param dy Cell height.
     */
    void setDy(double dy);
    ///@}
    ///
    ///     /** @name Interpolation */
    ///@{
    /**
     * @brief Interpolate raster value at a given world coordinate (x, y).
     *
     * Uses bilinear interpolation between the four surrounding cells.
     *
     * @param xCoord X coordinate in the same CRS as the GeoTIFF.
     * @param yCoord Y coordinate in the same CRS as the GeoTIFF.
     * @return Interpolated value.
     * @throw std::out_of_range if the coordinate is outside the raster bounds.
     */
    double valueAt(double xCoord, double yCoord) const;
    ///@}


    /** @name Resampling */
    ///@{
    /**
     * @brief Create a resampled GeoTiffHandler with new resolution.
     *
     * Interpolates values to match the requested number of cells in x and y.
     *
     * @param newNx Number of cells in x-direction.
     * @param newNy Number of cells in y-direction.
     * @return A new GeoTiffHandler object with resampled data.
     */
    GeoTiffHandler resample(int newNx, int newNy) const;
    ///@}


    /** @name Output */
    ///@{
    /**
     * @brief Save the current raster (data_2d) to a GeoTIFF file.
     *
     * @param filename Path to the output GeoTIFF file.
     * @throw std::runtime_error if saving fails.
     */
    void saveAs(const std::string& filename) const;
    ///@}


    /** @name Indexing */
    ///@{
    /**
     * @brief Find the grid indices (i, j) corresponding to a given world coordinate (x, y).
     *
     * @param xCoord World x coordinate.
     * @param yCoord World y coordinate.
     * @return Pair (i, j) of grid indices. i = column index, j = row index.
     * @throw std::out_of_range if coordinate is outside raster extent.
     */
    std::pair<int,int> indicesAt(double xCoord, double yCoord) const;
    ///@}

    /** @name ASCII I/O */
    ///@{
    /**
     * @brief Save the raster to ESRI ASCII format.
     * @param filename Output ASCII filename.
     * @param nodata NODATA value to write where needed (default -9999).
     */
    void saveAsAscii(const std::string& filename, double nodata = -9999.0) const;

    /**
     * @brief Load raster from ESRI ASCII format.
     * Overwrites current data_2d_, x_, y_, dx_, dy_, width_, height_.
     * @param filename Input ASCII filename.
     * @throw std::runtime_error if parsing fails.
     */
    void loadFromAscii(const std::string& filename);
    ///@}

    /** @name Flow Accumulation */
    ///@{
    /**
     * @brief Initialize flow accumulation grid (each cell = 1.0).
     */
    void initFlowAccum();

    /**
     * @brief Perform one iteration of flow accumulation update.
     * Each cell receives contributions from higher-elevation neighbors.
     * @return true if any updates occurred, false if stable.
     */
    bool updateFlowAccum();

    /**
     * @brief Run flow accumulation until convergence.
     * @param maxIter Maximum iterations (default 1000).
     */
    void computeFlowAccum(int maxIter = 1000);

    /** @name Flow Routing */
    ///@{
    /**
     * @brief Find the steepest downslope neighbor.
     * @param i Column index of the cell.
     * @param j Row index of the cell.
     * @param type Neighborhood type: FlowDirType::D4 (N, S, E, W) or FlowDirType::D8 (diagonals included).
     * @return Pair (ni, nj) of downslope neighbor, or (-1,-1) if no downslope neighbor exists (pit/flat).
     */
    std::pair<int,int> downslope(int i, int j, FlowDirType type = FlowDirType::D4) const;


    /**
     * @brief Check if cell (i0,j0) eventually drains to target cell (itarget,jtarget).
     * Uses D4 steepest descent flow directions.
     * @param i0 Column index of source cell.
     * @param j0 Row index of source cell.
     * @param itarget Column index of target cell.
     * @param jtarget Row index of target cell.
     * @return True if source drains to target, false otherwise.
     */
    bool drainsTo(int i0, int j0, int itarget, int jtarget, FlowDirType type) const;
    ///@}

    /**
     * @brief Extract watershed (upslope area) draining to a target cell.
     * @param itarget Target cell column index.
     * @param jtarget Target cell row index.
     * @param type Neighborhood type (D4 or D8).
     * @return New GeoTiffHandler cropped to bounding box of watershed.
     *         Inside watershed = DEM values, outside = NaN.
     */
    GeoTiffHandler watershed(int itarget, int jtarget, FlowDirType type = FlowDirType::D4) const;

    GeoTiffHandler watershedMFD(int itarget, int jtarget, FlowDirType type = FlowDirType::D4) const;
    GeoTiffHandler cropMasked(double nodataThreshold) const;

    static std::vector<std::vector<std::vector<std::pair<int,int>>>> buildInflowMFD(
        const std::vector<std::vector<double>>& dem,
        int width, int height,
        FlowDirType type);

    bool drainsToMFD(int i0, int j0, int itarget, int jtarget, FlowDirType type) const;


    Path downstreamPath(int i0, int j0, FlowDirType type) const;
    /**
     * @brief Compute the watershed for a target cell. If its size exceeds minSize,
     *        return it immediately. Otherwise, evaluate all D8 neighbors and return
     *        the watershed (among target + neighbors) with the maximum number of pixels.
     * @param i Target column index.
     * @param j Target row index.
     * @param minSize Minimum acceptable number of pixels in the watershed.
     * @param type Flow direction type (D4 or D8) used for delineating watersheds.
     * @return The selected watershed (GeoTiffHandler).
     */
    GeoTiffHandler watershedWithThreshold(int i, int j, int minSize, FlowDirType type) const;


    /** @name Cell Value Queries */
    ///@{
    /**
     * @brief Find the cell with the maximum value in the raster.
     * @return A tuple (i, j, value).
     */
    std::tuple<int,int,double> maxCell() const;

    /**
     * @brief Find the cell with the minimum value in the raster.
     * @return A tuple (i, j, value).
     */
    std::tuple<int,int,double> minCell() const;
    /**
     * @brief Find the indices of the cell with the maximum value.
     * @return A pair (i, j).
     */
    std::pair<int,int> maxCellIndex() const;

    /**
     * @brief Find the indices of the cell with the minimum value.
     * @return A pair (i, j).
     */
    std::pair<int,int> minCellIndex() const;
    ///@}

    QString info(const QString& fileName) const;


    /**
     * @brief Detect single-pixel sinks in the DEM.
     *
     * A sink pixel is defined as one whose elevation is strictly less than all
     * of its valid neighbors (D4 or D8).
     *
     * @param type Neighborhood type: FlowDirType::D4 or FlowDirType::D8.
     * @return A new GeoTiffHandler object where sinks are marked as 1.0, others as 0.0.
     */
    GeoTiffHandler detectSinks(FlowDirType type = FlowDirType::D8) const;

    /**
     * @brief Iteratively fill single-pixel sinks by replacing them with the average of their neighbors.
     *
     * Boundary pixels are never modified. Iterates until no sinks remain or maxIter is reached.
     *
     * @param type Neighborhood type: FlowDirType::D4 or FlowDirType::D8.
     * @param maxIter Maximum iterations (default 1000).
     * @return A new GeoTiffHandler object with sinks filled.
     */
    GeoTiffHandler fillSinksIterative(FlowDirType type = FlowDirType::D8, int maxIter = 1000) const;

    /**
     * @brief Count the number of non-NaN cells in the raster.
     * @return Number of valid cells.
     */
    int countValidCells() const;

    /**
     * @brief Compute flow accumulation using Multiple Flow Direction (MFD).
     *
     * Flow is proportionally distributed to all downslope neighbors.
     * Each cell contributes 1 (itself) plus inflow from upslope neighbors.
     *
     * @param type Neighborhood type: FlowDirType::D4 or FlowDirType::D8.
     * @param exponent Exponent on slope weighting (default 1.1, common in literature).
     * @return A new GeoTiffHandler with flow accumulation values.
     */
    GeoTiffHandler flowAccumulationMFD(FlowDirType type = FlowDirType::D8, double exponent = 1.1) const;

    enum class FilterMode { Greater, Smaller };

    /**
     * @brief Filter cells by a threshold.
     *        - Greater: keeps values > threshold
     *        - Smaller: keeps values < threshold
     *        Other cells set to NaN.
     *
     * @param threshold Threshold value.
     * @param mode Filter mode (Greater or Smaller).
     * @return A new GeoTiffHandler object with filtered values.
     */
    GeoTiffHandler filterByThreshold(double threshold, FilterMode mode) const;

     /**
     * @brief Compute the total area of valid cells.
     * @return Area = countValidCells() * dx_ * |dy_|
     */
    double area() const;

    /**
     * @brief Resample the raster by averaging values of all source pixels
     *        that fall inside each target pixel footprint.
     *
     * This is useful for downsampling: each new cell value is the average
     * of the original cells it covers (ignoring NaN).
     *
     * @param newNx Number of cells in x-direction (target width).
     * @param newNy Number of cells in y-direction (target height).
     * @return A new GeoTiffHandler object with aggregated values.
     */
    GeoTiffHandler resampleAverage(int newNx, int newNy) const;

    /**
     * @brief Extract the center coordinates of all pixels in the grid.
     * @return Vector of (x,y) pairs representing pixel centers.
     */
    std::vector<std::pair<double,double>> cellCenters() const;

        // ...

        /**
     * @brief Extract nodes (x,y,value) from raster.
     *
     * If an optional valueRaster is provided, values are taken from it.
     * Otherwise, values come from this raster.
     *
     * @param valueRaster Optional pointer to another GeoTiffHandler with same size.
     * @return Vector of Node objects.
     * @throw std::runtime_error if valueRaster dimensions are inconsistent.
     */
    std::vector<Node> nodes(const GeoTiffHandler* valueRaster = nullptr) const;




private:
    std::string filename_;   ///< Path to the GeoTIFF file.
    GDALDataset* dataset_;   ///< GDAL dataset handle.
    int width_;              ///< Raster width in pixels.
    int height_;             ///< Raster height in pixels.
    int bands_;              ///< Number of raster bands.

    std::vector<float> data_;                   ///< 1D raster data buffer.
    std::vector<std::vector<double>> data_2d_;  ///< 2D raster data buffer [i][j].
    std::vector<double> x_;                     ///< X coordinates of cell centers.
    std::vector<double> y_;                     ///< Y coordinates of cell centers.
    double dx_;                                 ///< Cell size in x-direction.
    double dy_;                                 ///< Cell size in y-direction.


};

#endif // GEOTIFFHANDLER_H

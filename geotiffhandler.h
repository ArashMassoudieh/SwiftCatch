#ifndef GEOTIFFHANDLER_H
#define GEOTIFFHANDLER_H

#include <string>
#include <vector>
#include <gdal_priv.h>

/**
 * @class GeoTiffHandler
 * @brief A utility class for reading and processing GeoTIFF raster files using GDAL.
 *
 * This class loads raster data from a GeoTIFF file and provides access to
 * pixel values, spatial coordinates, and metadata. Data are stored in both
 * a flat 1D vector and a 2D grid for convenience.
 */
class GeoTiffHandler {
public:
    /**
     * @brief Constructor that loads a GeoTIFF file into memory.
     * @param filename Path to the GeoTIFF file.
     * @throw std::runtime_error if the file cannot be opened or read.
     */
    explicit GeoTiffHandler(const std::string& filename);

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

    /** @name Flow Routing (D4) */
    ///@{
    /**
     * @brief Find the steepest downslope neighbor using D4 (N, S, E, W).
     * @param i Column index of the cell.
     * @param j Row index of the cell.
     * @return Pair (ni, nj) of downslope neighbor, or (-1,-1) if no downslope.
     */
    std::pair<int,int> downslopeD4(int i, int j) const;

    /**
     * @brief Check if cell (i0,j0) eventually drains to target cell (itarget,jtarget).
     * Uses D4 steepest descent flow directions.
     * @param i0 Column index of source cell.
     * @param j0 Row index of source cell.
     * @param itarget Column index of target cell.
     * @param jtarget Row index of target cell.
     * @return True if source drains to target, false otherwise.
     */
    bool drainsToD4(int i0, int j0, int itarget, int jtarget) const;
    ///@}

    /**
     * @brief Extract watershed (upslope area) draining to a target cell using D4 flow routing.
     * @param itarget Target cell column index.
     * @param jtarget Target cell row index.
     * @return New GeoTiffHandler containing only the watershed region,
     *         cropped to bounding box. Pixels outside watershed are NaN.
     */
    GeoTiffHandler watershedD4(int itarget, int jtarget) const;

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

private:
    // Special constructor for in-memory rasters
    GeoTiffHandler(int width, int height);

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

#ifndef GEOTIFFHANDLER_H
#define GEOTIFFHANDLER_H

#include <string>
#include <vector>
#include <gdal_priv.h>
#include <cpl_conv.h> // CPLMalloc

/**
 * @class GeoTiffHandler
 * @brief A simple C++ wrapper for reading and processing single-band GeoTIFF files using GDAL.
 *
 * This class loads a GeoTIFF raster into memory and provides basic operations
 * such as computing statistics and normalization. It is intended for lightweight
 * raster processing tasks. Currently, only the first raster band is read into memory.
 *
 * Example usage:
 * @code
 * GeoTiffHandler tif("dem.tif");
 * std::cout << "Width: " << tif.width() << std::endl;
 * std::cout << "Min value: " << tif.minValue() << std::endl;
 * @endcode
 */
class GeoTiffHandler {
private:
    std::string filename_;        /**< Path to the GeoTIFF file */
    GDALDataset* dataset_;        /**< Pointer to the GDAL dataset object */
    int width_;                   /**< Raster width in pixels */
    int height_;                  /**< Raster height in pixels */
    int bands_;                   /**< Number of raster bands in the dataset */
    std::vector<float> data_;     /**< Raster data values for the first band */

public:
    /**
     * @brief Constructor: Opens the GeoTIFF and loads the first band into memory.
     * @param filename Path to the GeoTIFF file
     * @throw std::runtime_error if the file cannot be opened or read
     */
    explicit GeoTiffHandler(const std::string& filename);

    /**
     * @brief Destructor: Closes the GDAL dataset and frees resources.
     */
    ~GeoTiffHandler();

    /**
     * @brief Get the raster width (number of columns).
     * @return Width in pixels
     */
    int width() const;

    /**
     * @brief Get the raster height (number of rows).
     * @return Height in pixels
     */
    int height() const;

    /**
     * @brief Get the number of raster bands in the dataset.
     * @return Band count
     */
    int bands() const;

    /**
     * @brief Get a const reference to the raster data values.
     *
     * The raster is stored in row-major order (line by line).
     * For a given row r and column c, the index is `r * width() + c`.
     *
     * @return Const reference to the vector of raster values
     */
    const std::vector<float>& data() const;

    /**
     * @brief Compute the minimum value in the raster data.
     * @return Minimum pixel value
     */
    float minValue() const;

    /**
     * @brief Compute the maximum value in the raster data.
     * @return Maximum pixel value
     */
    float maxValue() const;

    /**
     * @brief Retrieve a GeoTransform coefficient from the dataset.
     *
     * GDAL GeoTransform has 6 coefficients:
     * - gt[0]: Top-left X coordinate
     * - gt[1]: Pixel width (x resolution)
     * - gt[2]: Rotation (0 if north up)
     * - gt[3]: Top-left Y coordinate
     * - gt[4]: Rotation (0 if north up)
     * - gt[5]: Pixel height (y resolution, usually negative)
     *
     * @param idx Index of the coefficient (0–5)
     * @return The GeoTransform value at the given index
     * @throw std::runtime_error if GeoTransform is not available
     */
    double getGeoTransform(int idx) const;

    /**
     * @brief Normalize the raster values to the range [0, 1].
     *
     * The normalization is performed as:
     * @f[
     * v_{norm} = \frac{v - v_{min}}{v_{max} - v_{min}}
     * @f]
     * where v is the original value, and v_min and v_max are the
     * global minimum and maximum values in the raster.
     */
    void normalize();
};

#endif // GEOTIFFHANDLER_H

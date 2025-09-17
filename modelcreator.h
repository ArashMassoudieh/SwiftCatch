#ifndef MODELCREATOR_H
#define MODELCREATOR_H

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QString>
#include <string>

class GeoTiffHandler;
class StreamNetwork;

/**
 * @brief The ModelCreator class
 *
 * Responsible for building an OpenHydroQual model JSON
 * from a DEM (GeoTiffHandler) and a derived StreamNetwork.
 */
class ModelCreator {
public:
    ModelCreator(const GeoTiffHandler& dem, const StreamNetwork& network);

    /**
     * @brief Builds the model JSON document
     *
     * @return QJsonDocument
     */
    QJsonDocument buildModel() const;

    /**
     * @brief Save the model to a JSON file (.ohq)
     *
     * @param filePath Path to save the JSON file
     */
    void saveModel(const QString& filePath) const;

private:
    const GeoTiffHandler& dem_;
    const StreamNetwork& network_;

    void validateInputs() const;

    // --- modular creators ---
    QJsonArray CreateTemplates() const;
    QJsonObject CreateSettings() const;

    struct CatchmentProperties {
        double manningCoeff = 0.011;
        double depressionStorage = 0.0;
        double depth = 0.0;
        double lossCoefficient = 0.0;
    } CatchmentProperties_;

    QJsonObject CreateCatchmentBlocks() const;
    QJsonObject CreateCatchmentLinks() const;


};

#endif // MODELCREATOR_H

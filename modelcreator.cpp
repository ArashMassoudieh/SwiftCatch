// ModelCreator.cpp

#include "modelcreator.h"
#include <QFile>
#include <QTextStream>
#include <stdexcept>
#include <iostream>
#include "streamnetwork.h"
#include "geotiffhandler.h"

ModelCreator::ModelCreator(const GeoTiffHandler& dem, const StreamNetwork& network)
    : dem_(dem), network_(network)
{
    validateInputs();
}

void ModelCreator::validateInputs() const {
    if (dem_.width() == 0 || dem_.height() == 0) {
        throw std::runtime_error("ModelCreator: DEM is empty or invalid.");
    }
    if (network_.edges().empty()) {
        throw std::runtime_error("ModelCreator: Stream network is empty.");
    }
}

QJsonArray ModelCreator::CreateTemplates() const {
    QJsonArray templates;
    templates.append("rainfall_runoff.json");
    return templates;
}

QJsonObject ModelCreator::CreateSettings() const {
    QJsonObject settings;

    settings["acceptance_rate"] = "0.15";
    settings["add_noise_to_realizations"] = "No";
    settings["alloutputfile"] = "output.txt";
    settings["c_n_weight"] = "1";
    settings["continue_based_on_file_name"] = "";
    settings["initial_purturbation"] = "No";
    settings["initial_time_step"] = "0.01";
    settings["initual_purturbation_factor"] = "0.05";
    settings["jacobian_method"] = "Inverse Jacobian";
    settings["maximum_number_of_matrix_inverstions"] = "200000";
    settings["maximum_time_allowed"] = "86400";
    settings["maxpop"] = "40";
    settings["minimum_timestep"] = "1e-06";
    settings["n_threads"] = "8";
    settings["ngen"] = "40";
    settings["nr_timestep_reduction_factor"] = "0.75";
    settings["nr_timestep_reduction_factor_fail"] = "0.2";
    settings["nr_tolerance"] = "0.001";
    settings["number_of_burnout_samples"] = "0";
    settings["number_of_chains"] = "8";
    settings["number_of_post_estimate_realizations"] = "10";
    settings["number_of_samples"] = "1000";
    settings["number_of_threads"] = "1";
    settings["numthreads"] = "8";
    settings["observed_outputfile"] = "observedoutput.txt";
    settings["outputfile"] = "GA_output.txt";
    settings["pcross"] = "1";
    settings["perform_global_sensitivity"] = "No";
    settings["pmute"] = "0.02";
    settings["purturbation_change_scale"] = "0.75";
    settings["record_interval"] = "1";
    settings["samples_filename"] = "mcmc.txt";
    settings["shakescale"] = "0.05";
    settings["shakescalered"] = "0.75";
    settings["simulation_end_time"] = "1";
    settings["simulation_start_time"] = "0";
    settings["write_interval"] = "100";
    settings["write_solution_details"] = "No";

    return settings;
}

QJsonDocument ModelCreator::buildModel() const {
    QJsonObject root;

    // Templates + Settings
    root["Templates"] = CreateTemplates();
    root["Settings"]  = CreateSettings();

    // --- Blocks ---
    QJsonObject blocks;
    QJsonObject catchments = CreateCatchmentBlocks();
    // later: merge soil and stream blocks here
    for (auto it = catchments.begin(); it != catchments.end(); ++it)
        blocks[it.key()] = it.value();
    root["Blocks"] = blocks;

    // --- Links ---
    QJsonObject links;
    QJsonObject catchmentLinks = CreateCatchmentLinks();
    // later: merge soil and stream links here
    for (auto it = catchmentLinks.begin(); it != catchmentLinks.end(); ++it)
        links[it.key()] = it.value();
    root["Links"] = links;

    return QJsonDocument(root);
}

void ModelCreator::saveModel(const QString& filePath) const {
    QJsonDocument doc = buildModel();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        throw std::runtime_error("ModelCreator: Unable to open file for writing.");
    }

    file.write(doc.toJson(QJsonDocument::Indented));
}

QJsonObject ModelCreator::CreateCatchmentBlocks() const {
    QJsonObject blocks;

    double dx = fabs(dem_.dx());
    double dy = fabs(dem_.dy());
    double width = 0.7 * dx;
    double height = 0.7 * dy;
    double area = dx * dy;

    int count = 0;
    for (int j = 0; j < dem_.height(); ++j) {
        for (int i = 0; i < dem_.width(); ++i) {
            double val = dem_.data2D()[i][j];
            if (std::isnan(val)) continue; // skip invalid cells

            ++count;
            QString name = QString("Catchment (%1@%2)").arg(i).arg(j);

            QJsonObject block;
            block["Evapotranspiration"] = "";
            block["ManningCoeff"] = QString::number(CatchmentProperties_.manningCoeff);
            block["Precipitation"] = "";
            block["_height"] = QString::number(height);
            block["_width"] = QString::number(width);
            block["area"] = QString::number(area);
            block["depression_storage"] = QString::number(CatchmentProperties_.depressionStorage);
            block["depth"] = QString::number(0.0);
            block["elevation"] = QString::number(val);
            block["loss_coefficient"] = QString::number(CatchmentProperties_.lossCoefficient);
            block["name"] = name;
            block["type"] = "catchment-distributed";
            block["x"] = QString::number(dem_.x()[i]);
            block["y"] = QString::number(dem_.y()[j]);

            blocks[name] = block;
        }
    }

    return blocks;
}

QJsonObject ModelCreator::CreateCatchmentLinks() const {
    QJsonObject links;

    double dx = fabs(dem_.dx());
    double dy = fabs(dem_.dy());

    for (int j = 0; j < dem_.height(); ++j) {
        for (int i = 0; i < dem_.width(); ++i) {
            if (std::isnan(dem_.data2D()[i][j])) continue;

            QString fromName = QString("Catchment (%1@%2)").arg(i).arg(j);

            // 4-connected neighbors: right, left, up, down
            const int dirs[4][2] = {
                { 1, 0 },  // right
                {-1, 0 },  // left
                { 0, 1 },  // up
                { 0,-1 }   // down
            };

            for (auto& d : dirs) {
                int ni = i + d[0];
                int nj = j + d[1];
                if (ni < 0 || nj < 0 || ni >= dem_.width() || nj >= dem_.height()) continue;
                if (std::isnan(dem_.data2D()[ni][nj])) continue;

                QString toName = QString("Catchment (%1@%2)").arg(ni).arg(nj);
                QString linkName = fromName + " - " + toName;

                QJsonObject link;

                if (d[0] != 0) { // horizontal neighbor
                    link["Length"] = QString::number(fabs(dx));
                    link["Width"]  = QString::number(fabs(dy));
                } else {         // vertical neighbor
                    link["Length"] = QString::number(fabs(dy));
                    link["Width"]  = QString::number(fabs(dx));
                }

                link["from"] = fromName;
                link["to"]   = toName;
                link["name"] = linkName;
                link["type"] = "distributed_catchment_link";

                links[linkName] = link;
            }
        }
    }

    return links;
}



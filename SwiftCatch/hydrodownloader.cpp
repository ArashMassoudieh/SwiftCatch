#include "hydrodownloader.h"
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>




HydroDownloader::HydroDownloader()
{

}

QVector<station_info> HydroDownloader::fetchAllStations(const QString &state) {
    QVector<station_info> stationList;

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Define the URL for fetching all stations
    QUrl url("https://waterservices.usgs.gov/nwis/site/?format=rdb&stateCD="+state.split('-')[0].trimmed());

    // Create the request
    QNetworkRequest request(url);

    // Create an event loop to wait for the request to finish
    QEventLoop loop;

    // Send the request
    QNetworkReply* reply = manager.get(request);

    // Connect the finished signal to the event loop's quit slot
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Block and wait for the reply
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        // Parse the response (RDB format)
        QByteArray responseData = reply->readAll();
        QString responseString = QString::fromUtf8(responseData);

        // Split into lines
        QStringList lines = responseString.split("\n");

        // Parse each line
        for (const QString& line : lines) {
            if (line.startsWith("#") || line.isEmpty()) continue; // Skip comments and empty lines
            QStringList fields = line.split("\t");
            if (!fields.isEmpty()) {
                station_info statinfo;
                statinfo.agency_cd = fields[0]; // Station ID is in the first column

                statinfo.site_no = fields[1];
                statinfo.station_nm = fields[2];
                statinfo.site_tp_cd = fields[2];
                statinfo.dec_lat_va = fields[2].toDouble();
                statinfo.dec_long_va = fields[2].toDouble();
                statinfo.coord_acy_cd = fields[2];
                statinfo.ddec_coord_datum_cd = fields[2];
                statinfo.alt_va = fields[2];
                statinfo.alt_acy_va = fields[2];
                statinfo.alt_datum_cd = fields[2];
                statinfo.huc_cd = fields[2];
                if (statinfo.agency_cd!="agency_cd" && statinfo.agency_cd!="5s")
                    stationList.append(statinfo);
            }
        }

        qDebug() << "Total stations fetched:" << stationList.size();
    } else {
        // Handle errors
        qDebug() << "Error fetching station list:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();

    return stationList;
}


QVector<FlowData> fetchFlowData(const QString& stationId, const QString& startDate, const QString& endDate) {
    QVector<FlowData> flowDataList;

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Define the URL for the flow data request
    QUrl url(QString("https://waterservices.usgs.gov/nwis/iv/?sites=%1&parameterCd=00060&startDT=%2&endDT=%3&format=json")
                 .arg(stationId)
                 .arg(startDate)
                 .arg(endDate));

    // Create the request
    QNetworkRequest request(url);

    // Create an event loop to wait for the request to finish
    QEventLoop loop;

    // Send the request
    QNetworkReply* reply = manager.get(request);

    // Connect the finished signal to the event loop's quit slot
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Block and wait for the reply
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        // Parse the JSON response
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray timeSeriesArray = rootObj["value"].toObject()["timeSeries"].toArray();

            if (!timeSeriesArray.isEmpty()) {
                QJsonObject firstTimeSeries = timeSeriesArray[0].toObject();
                QJsonArray valuesArray = firstTimeSeries["values"].toArray();
                if (!valuesArray.isEmpty()) {
                    QJsonArray flowValues = valuesArray[0].toObject()["value"].toArray();
                    for (const QJsonValue& value : flowValues) {
                        QJsonObject dataPoint = value.toObject();
                        QString dateTimeStr = dataPoint["dateTime"].toString();
                        QString flowRateStr = dataPoint["value"].toString();

                        FlowData flowData;
                        flowData.dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
                        flowData.flowRate = flowRateStr.toDouble();

                        flowDataList.append(flowData);
                    }
                }
            }
        } else {
            qDebug() << "Error: Unable to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching flow data:" << reply->errorString();
    }

    reply->deleteLater();
    return flowDataList;
}

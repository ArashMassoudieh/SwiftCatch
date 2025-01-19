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
#include <QChartView>
#include <QChart>



HydroDownloader::HydroDownloader()
{

}

QMap<QString, station_info> HydroDownloader::fetchAllStations(const QString &state) {
    QMap<QString,station_info> stationList;

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
                    stationList[statinfo.station_nm] = statinfo;
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


QVector<FlowData> HydroDownloader::fetchFlowData(const QString& stationId, const QString& startDate, const QString& endDate) {
    QVector<FlowData> flowDataList;

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Define the URL for the flow data request

    QUrl url(QString("https://nwis.waterservices.usgs.gov/nwis/iv/?sites=%1&parameterCd=00060&startDT=%2&endDT=%3&format=json")
                 .arg(stationId)
                 .arg(startDate)
                 .arg(endDate));
    qDebug()<<url;
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
    qDebug()<<reply->errorString();
    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        // Parse the JSON response
        QByteArray responseData = reply->readAll();
        //qDebug()<<responseData;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        //qDebug()<<responseData;
        if (jsonDoc.isNull())
        {
            qDebug()<<"Json data is empty!";
        }
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

QVector<StationData> HydroDownloader::fetchAllStationData(const QString& stationId, const QString& startDate, const QString& endDate) {
    QVector<StationData> stationDataList;

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Define the base URL for the USGS API
    QString urlString = QString("https://waterservices.usgs.gov/nwis/iv/?sites=%1&format=json").arg(stationId);
    if (!startDate.isEmpty() && !endDate.isEmpty()) {
        urlString += QString("&startDT=%1&endDT=%2").arg(startDate, endDate);
    }

    QUrl url(urlString);
    QNetworkRequest request(url);

    // Create an event loop to block execution until the network reply is received
    QEventLoop loop;

    // Send the network request
    QNetworkReply* reply = manager.get(request);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec(); // Wait for the request to complete

    // Process the network reply
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray timeSeriesArray = rootObj["value"].toObject()["timeSeries"].toArray();

            // Parse each time series in the JSON response
            for (const QJsonValue& tsValue : timeSeriesArray) {
                QJsonObject timeSeriesObj = tsValue.toObject();

                // Extract parameter information
                QString parameterName = timeSeriesObj["variable"].toObject()["variableName"].toString();
                QString parameterCode = timeSeriesObj["variable"].toObject()["variableCode"].toArray()[0].toObject()["value"].toString();

                // Extract data values
                QJsonArray valuesArray = timeSeriesObj["values"].toArray();
                QVector<QPair<QDateTime, double>> dataValues;

                if (!valuesArray.isEmpty()) {
                    QJsonArray valueArray = valuesArray[0].toObject()["value"].toArray();
                    for (const QJsonValue& value : valueArray) {
                        QJsonObject valueObj = value.toObject();
                        QString dateTimeStr = valueObj["dateTime"].toString();
                        QString valueStr = valueObj["value"].toString();

                        QDateTime dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
                        double numericValue = valueStr.toDouble();

                        dataValues.append(qMakePair(dateTime, numericValue));
                    }
                }

                // Add the parameter and its values to the list
                StationData stationData;
                stationData.parameterName = parameterName;
                stationData.parameterCode = parameterCode;
                stationData.values = dataValues;
                stationDataList.append(stationData);
            }
        } else {
            qDebug() << "Error: Unable to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching data for station" << stationId << ":" << reply->errorString();
    }

    reply->deleteLater();
    return stationDataList;
}

double qDateTimeToExcel(const QDateTime& dateTime) {
    // Define the base date for Excel (January 1, 1900)
    QDate excelBaseDate(1900, 1, 1);

    // Check if the date is valid and before the Excel base date
    if (!dateTime.isValid() || dateTime.date() < excelBaseDate) {
        qDebug() << "Invalid date or date is before the Excel base date.";
        return 0.0;
    }

    // Calculate the number of days between the Excel base date and the given date
    qint64 daysSinceExcelBase = excelBaseDate.daysTo(dateTime.date());

    // Excel incorrectly treats 1900 as a leap year; we need to add an extra day for dates after February 28, 1900
    if (dateTime.date() > QDate(1900, 2, 28)) {
        daysSinceExcelBase += 1;
    }

    // Calculate the fraction of the day for the time portion
    qint64 secondsSinceMidnight = dateTime.time().secsTo(QTime(0, 0, 0));
    double fractionalDay = 1.0 - static_cast<double>(secondsSinceMidnight) / 86400.0;

    // Combine the days and the fractional day to get the Excel date/time number
    double excelDateTime = static_cast<double>(daysSinceExcelBase) + fractionalDay;

    return excelDateTime;
}

QDateTime excelToQDateTime(double excelDate) {
    if (excelDate < 1.0) {
        qDebug() << "Invalid Excel date. Must be >= 1.0.";
        return QDateTime();
    }

    // Excel base date: January 1, 1900
    QDate excelBaseDate(1900, 1, 1);

    // Excel leap year bug: Skip February 29, 1900 (nonexistent day)
    int days = static_cast<int>(excelDate); // Integer part represents days
    if (days >= 60) {
        days -= 1; // Correct for Excel's leap year bug
    }

    // Add the integer days to the base date
    QDate convertedDate = excelBaseDate.addDays(days - 1);

    // Extract the fractional part for time
    double fractionalDay = excelDate - static_cast<int>(excelDate);
    int totalSeconds = static_cast<int>(fractionalDay * 86400); // Convert fraction to seconds
    QTime convertedTime = QTime(0, 0, 0).addSecs(totalSeconds);

    // Combine the date and time
    return QDateTime(convertedDate, convertedTime);
}



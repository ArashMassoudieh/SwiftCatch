#include "hydrodownloader.h"
#include <QCoreApplication>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
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

QMap<QString, station_info> HydroDownloader::fetchAllHydroStations(const QString &state) {
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

QVector<HydroStationData> HydroDownloader::fetchAllStationData(const QString& stationId, const QString& startDate, const QString& endDate) {
    QVector<HydroStationData> stationDataList;

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
                HydroStationData stationData;
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


//Token: AuOQEjHeTwRMJeUjLpoXmneFKxUDdred
QMap<QString,WeatherStationData> HydroDownloader::fetchNOAAStations(const QString& stateCode, const QString& apiToken) {
    QMap<QString,WeatherStationData> stationList;

    // NOAA API endpoint
    int offset = 0;
    bool contin = true;
    while (contin) {

        QString url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/stations?locationid=FIPS:%1&limit=1000&offset=%2").arg(stateCode).arg(offset);

        // Create a QNetworkAccessManager
        QNetworkAccessManager manager;

        // Create the request with API token
        QNetworkRequest request;
        request.setUrl(QUrl(url));
        request.setRawHeader("token", apiToken.toUtf8());

        // Make the GET request
        QNetworkReply* reply = manager.get(request);

        // Create an event loop to wait for the network reply
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec(); // Block until the network request is complete

        // Process the reply
        if (reply->error() == QNetworkReply::NoError) {
            // Parse the JSON response
            QByteArray responseData = reply->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

            if (!jsonDoc.isNull() && jsonDoc.isObject()) {
                QJsonObject rootObj = jsonDoc.object();
                QJsonArray stationsArray = rootObj["results"].toArray();

                // Iterate over the stations and populate the QVector
                for (const QJsonValue& stationValue : stationsArray) {
                    QJsonObject station = stationValue.toObject();
                    qDebug()<<station;
                    WeatherStationData data;
                    data.id = station["id"].toString();
                    data.name = station["name"].toString();
                    data.latitude = station.contains("latitude") ? station["latitude"].toDouble() : 0.0;
                    data.longitude = station.contains("longitude") ? station["longitude"].toDouble() : 0.0;
                    data.elevation = station.contains("elevation") ? station["elevation"].toDouble() : 0.0;

                    stationList[data.name] = data;
                }
            } else {
                qDebug() << "Failed to parse JSON response.";
                contin = false;
            }
        } else {
            qDebug() << "Error:" << reply->errorString();
            contin = false;
        }

        if (stationList.count()%1000!=0)
            contin = false;
        else
            offset += 1000;
        reply->deleteLater();
    }
    return stationList;
}

QVector<PrecipitationData> HydroDownloader::fetchPrecipitationData(const QString& stationId, const QString& startDate, const QString& endDate, const QString& apiToken, precip_time_interval interval) {
    QVector<PrecipitationData> dataList;

    // NOAA API endpoint
    QString url;

    if (interval == precip_time_interval::HLY)
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/data?datasetid=PRECIP_HLY&stationid=%1&datatypeid=HPCP&startdate=%2&enddate=%3&limit=1000").arg(stationId).arg(startDate).arg(endDate);
    else if (interval == precip_time_interval::PRECIP_15)
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/data?datasetid=PRECIP_15&stationid=%1&datatypeid=HPCP&startdate=%2&enddate=%3&limit=1000").arg(stationId).arg(startDate).arg(endDate);
    else
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/data?datasetid=PRECIP_5&stationid=%1&datatypeid=HPCP&startdate=%2&enddate=%3&limit=1000").arg(stationId).arg(startDate).arg(endDate);

    qDebug()<<url;
    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        qDebug()<<"No Error";
        QByteArray responseData = reply->readAll();
        qDebug()<<responseData;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Iterate over the results and extract precipitation data
            for (const QJsonValue& resultValue : resultsArray) {
                QJsonObject resultObj = resultValue.toObject();
                QString dateTimeStr = resultObj["date"].toString();
                double precipitation = resultObj["value"].toDouble() / 10.0; // NOAA stores precipitation in tenths of mm

                PrecipitationData data;
                data.dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);
                data.precipitation = precipitation;

                dataList.append(data);
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching precipitation data:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return dataList;
}

QSet<DatasetDatatype> HydroDownloader::fetchDatasetAndDatatype(const QString& stationId, const QString& apiToken) {
    QSet<DatasetDatatype> resultSet;

    // NOAA API endpoint
    QString StationID = stationId;
    if (stationId.contains(':'));
        StationID = stationId.split(':')[1];
    QString url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/data?stationid=%1&limit=1000").arg(StationID);

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Iterate over the results and extract datasetid and datatypeid
            for (const QJsonValue& resultValue : resultsArray) {
                QJsonObject resultObj = resultValue.toObject();
                qDebug()<<resultObj;
                QString datasetId = resultObj["datasetid"].toString();
                QString datatypeId = resultObj["datatypeid"].toString();

                DatasetDatatype data;
                data.datasetId = datasetId;
                data.datatypeId = datatypeId;

                resultSet.insert(data); // Use QSet to avoid duplicates
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching dataset and datatype IDs:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return resultSet;
}

bool operator<(const DatasetDatatype& lhs, const DatasetDatatype& rhs) {
    return std::tie(lhs.datasetId, lhs.datatypeId) < std::tie(rhs.datasetId, rhs.datatypeId);
}

bool operator==(const DatasetDatatype& lhs, const DatasetDatatype& rhs) {
    return lhs.datasetId == rhs.datasetId && lhs.datatypeId == rhs.datatypeId;
}

uint qHash(const DatasetDatatype& key, uint seed) {
    return qHash(key.datasetId, seed) ^ qHash(key.datatypeId, seed);
}




// Function to fetch all available data types for a station
QSet<DataType> HydroDownloader::fetchAllDataTypesForStation(const QString& stationId, const QString& apiToken) {
    QSet<DataType> dataTypes;

    // NOAA API endpoint for datatypes
    QString url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/datatypes?stationid=%1&limit=1000").arg(stationId);

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Extract datatype information from results
            for (const QJsonValue& resultValue : resultsArray) {
                QJsonObject resultObj = resultValue.toObject();
                qDebug()<<resultObj;
                DataType dataType;
                dataType.id = resultObj["id"].toString();
                dataType.max_date = resultObj["maxdate"].toString();
                dataType.min_date = resultObj["mindate"].toString();
                dataType.name = resultObj["name"].toString();
                dataTypes.insert(dataType);
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching data types:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return dataTypes;
}

// Overload operators to use DataType in QSet
bool operator<(const DataType& lhs, const DataType& rhs) {
    return lhs.id < rhs.id;
}

bool operator==(const DataType& lhs, const DataType& rhs) {
    return lhs.id == rhs.id;
}

uint qHash(const DataType& key, uint seed) {
    return qHash(key.id, seed);
}


// Function to fetch stations providing hourly precipitation data
QMap<QString, WeatherStationData> HydroDownloader::fetchPrecipitationStations(const QString& FIPS, const QString& apiToken, precip_time_interval interval) {
    QMap<QString, WeatherStationData> stationList;

    // NOAA API endpoint
    QString url;
    if (interval == precip_time_interval::HLY)
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/stations?datasetid=PRECIP_HLY&datatypeid=HPCP&locationid=FIPS:%1&limit=1000").arg(FIPS);
    else if (interval == precip_time_interval::PRECIP_15)
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/stations?datasetid=PRECIP_15&datatypeid=HPCP&locationid=FIPS:%1&limit=1000").arg(FIPS);
    else
        url = QString("https://www.ncei.noaa.gov/cdo-web/api/v2/stations?datasetid=PRECIP_5&datatypeid=HPCP&locationid=FIPS:%1&limit=1000").arg(FIPS);

    // Create a QNetworkAccessManager
    QNetworkAccessManager manager;

    // Create the request with API token
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("token", apiToken.toUtf8());

    // Make the GET request
    QNetworkReply* reply = manager.get(request);

    // Use QEventLoop to wait for the response
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // Process the reply
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray responseData = reply->readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);

        if (!jsonDoc.isNull() && jsonDoc.isObject()) {
            QJsonObject rootObj = jsonDoc.object();
            QJsonArray resultsArray = rootObj["results"].toArray();

            // Iterate over the stations and populate the QVector
            for (const QJsonValue& stationValue : resultsArray) {
                QJsonObject stationObj = stationValue.toObject();
                WeatherStationData metadata;
                qDebug()<<stationObj;
                metadata.id = stationObj["id"].toString();
                qDebug()<<metadata.id;
                metadata.name = stationObj["name"].toString();
                metadata.latitude = stationObj.contains("latitude") ? stationObj["latitude"].toDouble() : 0.0;
                metadata.longitude = stationObj.contains("longitude") ? stationObj["longitude"].toDouble() : 0.0;
                metadata.elevation = stationObj.contains("elevation") ? stationObj["elevation"].toDouble() : 0.0;
                metadata.mindate = stationObj["mindate"].toString();
                metadata.maxdate = stationObj["maxdate"].toString();
                metadata.datacoverage = stationObj.contains("datacoverage") ? stationObj["datacoverage"].toDouble() : 0.0;

                stationList[metadata.name] = metadata;
            }
        } else {
            qDebug() << "Failed to parse JSON response.";
        }
    } else {
        qDebug() << "Error fetching station metadata:" << reply->errorString();
    }

    // Cleanup
    reply->deleteLater();
    return stationList;
}




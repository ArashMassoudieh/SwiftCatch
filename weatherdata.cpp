#include "weatherdata.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include "Utilities/BTC.h"
#include <QDate>
#include <QTime>
#include <QDateTime>


WeatherData::WeatherData()
{

}

bool WeatherData::ReadFromFile(const QString &fileName)
{
    QFile file(fileName);
    QFileInfo fileInfo(fileName);

    if (!fileInfo.exists())
       qDebug() << "File Error", "File '" + fileName + "' does not exist";
    // Check if file exists
    if (!fileInfo.exists() || !file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File Error", "Unable to open the file: " + fileName;
        return false;
    }

    QTextStream in(&file);
    QStringList headers;
    QString line = in.readLine(); // First line should contain the headers

    if (!line.isEmpty()) {

    } else {
        qDebug() << "File Error", "CSV file is empty.";
        return false;
    }
    CTimeSeries<double> precipitation;
    int row = 0;

    double _last_value = 0;
    unsigned int error_count = 0;
    while (!in.atEnd()) {
        line = in.readLine();
        if (!line.isEmpty()) {
            // Create a new instance of WeatherData struct
            DataRecord data;
            QStringList cells = splitCsv(line);

            //for (int i = 0; i < cells.size(); ++i) {
            //    qDebug() << "Column " << i << ": " << cells[i];
            //}

            // Fill the struct members with the data from the CSV row
            data.STATION = cells[0].remove('"');
            data.DATE = cells[1].remove('"');
            data.REPORT_TYPE = cells[2].remove('"');
            data.SOURCE = cells[3].remove('"');
            data.AWND = cells[4].remove('"');
            data.BackupDirection = cells[5].remove('"');
            data.BackupDistance = cells[6].remove('"');
            data.BackupDistanceUnit = cells[7].remove('"');
            data.BackupElements = cells[8].remove('"');
            data.BackupElevation = cells[9].remove('"');
            data.BackupElevationUnit = cells[10].remove('"');
            data.BackupEquipment = cells[11].remove('"');
            data.BackupLatitude = cells[12].remove('"');
            data.BackupLongitude = cells[13].remove('"');
            data.BackupName = cells[14].remove('"');
            data.CDSD = cells[15].remove('"');
            data.CLDD = cells[16].remove('"');
            data.DSNW = cells[17].remove('"');
            data.DYHF = cells[18].remove('"');
            data.DYTS = cells[19].remove('"');
            data.DailyAverageDewPointTemperature = cells[20].remove('"');
            data.DailyAverageDryBulbTemperature = cells[21].remove('"');
            data.DailyAverageRelativeHumidity = cells[22].remove('"');
            data.DailyAverageSeaLevelPressure = cells[23].remove('"');
            data.DailyAverageStationPressure = cells[24].remove('"');
            data.DailyAverageWetBulbTemperature = cells[25].remove('"');
            data.DailyAverageWindSpeed = cells[26].remove('"');
            data.DailyCoolingDegreeDays = cells[27].remove('"');
            data.DailyDepartureFromNormalAverageTemperature = cells[28].remove('"');
            data.DailyHeatingDegreeDays = cells[29].remove('"');
            data.DailyMaximumDryBulbTemperature = cells[30].remove('"');
            data.DailyMinimumDryBulbTemperature = cells[31].remove('"');
            data.DailyPeakWindDirection = cells[32].remove('"');
            data.DailyPeakWindSpeed = cells[33].remove('"');
            data.DailyPrecipitation = cells[34].remove('"');
            data.DailySnowDepth = cells[35].remove('"');
            data.DailySnowfall = cells[36].remove('"');
            data.DailySustainedWindDirection = cells[37].remove('"');
            data.DailySustainedWindSpeed = cells[38].remove('"');
            data.DailyWeather = cells[39].remove('"');
            data.HDSD = cells[40].remove('"');
            data.HTDD = cells[41].remove('"');
            data.HourlyAltimeterSetting = cells[42].remove('"');
            data.HourlyDewPointTemperature = cells[43].remove('"');
            data.HourlyDryBulbTemperature = cells[44].remove('"');
            data.HourlyPrecipitation = cells[45].remove('"');
            data.HourlyPresentWeatherType = cells[46].remove('"');
            data.HourlyPressureChange = cells[47].remove('"');
            data.HourlyPressureTendency = cells[48].remove('"');
            data.HourlyRelativeHumidity = cells[49].remove('"');
            data.HourlySeaLevelPressure = cells[50].remove('"');
            data.HourlySkyConditions = cells[51].remove('"');
            data.HourlyStationPressure = cells[52].remove('"');
            data.HourlyVisibility = cells[53].remove('"');
            data.HourlyWetBulbTemperature = cells[54].remove('"');
            data.HourlyWindDirection = cells[55].remove('"');
            data.HourlyWindGustSpeed = cells[56].remove('"');
            data.HourlyWindSpeed = cells[57].remove('"');
            data.MonthlyAverageRH = cells[58].remove('"');
            data.MonthlyDaysWithGT001Precip = cells[59].remove('"');
            data.MonthlyDaysWithGT010Precip = cells[60].remove('"');
            data.MonthlyDaysWithGT32Temp = cells[61].remove('"');
            data.MonthlyDaysWithGT90Temp = cells[62].remove('"');
            data.MonthlyDaysWithLT0Temp = cells[63].remove('"');
            data.MonthlyDaysWithLT32Temp = cells[64].remove('"');
            data.MonthlyDepartureFromNormalAverageTemperature = cells[65].remove('"');
            data.MonthlyDepartureFromNormalCoolingDegreeDays = cells[66].remove('"');
            data.MonthlyDepartureFromNormalHeatingDegreeDays = cells[67].remove('"');
            data.MonthlyDepartureFromNormalMaximumTemperature = cells[68].remove('"');
            data.MonthlyDepartureFromNormalMinimumTemperature = cells[69].remove('"');
            data.MonthlyDepartureFromNormalPrecipitation = cells[70].remove('"');
            data.MonthlyDewpointTemperature = cells[71].remove('"');
            data.MonthlyGreatestPrecip = cells[72].remove('"');
            data.MonthlyGreatestPrecipDate = cells[73].remove('"');
            data.MonthlyGreatestSnowDepth = cells[74].remove('"');
            data.MonthlyGreatestSnowDepthDate = cells[75].remove('"');
            data.MonthlyGreatestSnowfall = cells[76].remove('"');
            data.MonthlyGreatestSnowfallDate = cells[77].remove('"');
            data.MonthlyMaxSeaLevelPressureValue = cells[78].remove('"');
            data.MonthlyMaxSeaLevelPressureValueDate = cells[79].remove('"');
            data.MonthlyMaxSeaLevelPressureValueTime = cells[80].remove('"');
            data.MonthlyMaximumTemperature = cells[81].remove('"');
            data.MonthlyMeanTemperature = cells[82].remove('"');
            data.MonthlyMinSeaLevelPressureValue = cells[83].remove('"');
            data.MonthlyMinSeaLevelPressureValueDate = cells[84].remove('"');
            data.MonthlyMinSeaLevelPressureValueTime = cells[85].remove('"');
            data.MonthlyMinimumTemperature = cells[86].remove('"');
            data.MonthlySeaLevelPressure = cells[87].remove('"');
            data.MonthlyStationPressure = cells[88].remove('"');
            data.MonthlyTotalLiquidPrecipitation = cells[89].remove('"');
            data.MonthlyTotalSnowfall = cells[90].remove('"');
            data.MonthlyWetBulb = cells[91].remove('"');
            data.NormalsCoolingDegreeDay = cells[92].remove('"');
            data.NormalsHeatingDegreeDay = cells[93].remove('"');
            data.REM = cells[94].remove('"');
            data.ShortDurationEndDate005 = cells[95].remove('"');
            data.ShortDurationEndDate010 = cells[96].remove('"');
            data.ShortDurationEndDate015 = cells[97].remove('"');
            data.ShortDurationEndDate020 = cells[98].remove('"');
            data.ShortDurationEndDate030 = cells[99].remove('"');
            data.ShortDurationEndDate045 = cells[100].remove('"');
            data.ShortDurationEndDate060 = cells[101].remove('"');
            data.ShortDurationEndDate080 = cells[102].remove('"');
            data.ShortDurationEndDate100 = cells[103].remove('"');
            data.ShortDurationEndDate120 = cells[104].remove('"');
            data.ShortDurationEndDate150 = cells[105].remove('"');
            data.ShortDurationEndDate180 = cells[106].remove('"');
            data.ShortDurationPrecipitationValue005 = cells[107].remove('"');
            data.ShortDurationPrecipitationValue010 = cells[108].remove('"');
            data.ShortDurationPrecipitationValue015 = cells[109].remove('"');
            data.ShortDurationPrecipitationValue020 = cells[110].remove('"');
            data.ShortDurationPrecipitationValue030 = cells[111].remove('"');
            data.ShortDurationPrecipitationValue045 = cells[112].remove('"');
            data.ShortDurationPrecipitationValue060 = cells[113].remove('"');
            data.ShortDurationPrecipitationValue080 = cells[114].remove('"');
            data.ShortDurationPrecipitationValue100 = cells[115].remove('"');
            data.ShortDurationPrecipitationValue120 = cells[116].remove('"');
            data.ShortDurationPrecipitationValue150 = cells[117].remove('"');
            data.ShortDurationPrecipitationValue180 = cells[118].remove('"');
            data.Sunrise = cells[119].remove('"');
            data.Sunset = cells[120].remove('"');
            data.WindEquipmentChangeDate = cells[121].remove('"');

            append(data);
            /*
            double value = convertToExcelDateTime(data.DATE.remove('"'), "yyyy-MM-ddThh:mm:ss");

            if (!data.HourlyPrecipitation.isEmpty())
            {
                //qDebug()<<data.HourlyPrecipitation;
            }
            if (data.SOURCE.remove('"')=="7")
            {   if (value != -1)
                    precipitation.append( convertToExcelDateTime(data.DATE.remove('"'), "yyyy-MM-ddThh:mm:ss"),  data.HourlyPrecipitation.remove('"').toDouble()/100.00);
                else
                {   precipitation.append( _last_value + 1.0/24.0,  data.HourlyPrecipitation.remove('"').toDouble()/100.00);
                    error_count++;
                }
            }

            _last_value = value;
            */
            // Insert a new row in the table
            //tableWidget->insertRow(row);
            //for (int column = 0; column < cells.size(); ++column) {
            //    tableWidget->setItem(row, column, new QTableWidgetItem(cells[column]));
            //}
            ++row;
        }
    }

    file.close();
    //precipitation.writefile("/home/hoomanmoradpour/Dropbox/HickeyRun/Local_climate/Precipitation.csv");
    qDebug()<< "A total number of " + QString::number(error_count) + "  date values out of " + QString::number(row) + " rows were invalid";
    return true;
}

double convertToExcelDateTime(const QString &dateTimeString, const QString& format) {

    QDate excelEpoch(1899, 12, 30);  // Excel counts from 1900-01-01 but has an off-by-one issue

    // Convert input string to QDateTime
    QDateTime dt = QDateTime::fromString(dateTimeString, format);

    if (!dt.isValid()) {
        return -1; // Return -1 for invalid date input
    }

    // Compute the number of days
    qint64 days = excelEpoch.daysTo(dt.date());

    // Compute fraction of a day for time component
    double timeFraction = dt.time().msecsSinceStartOfDay() / (24.0 * 60 * 60 * 1000);

    // Return the Excel serial date
    double excelDateTime = days + timeFraction;

    return excelDateTime;
}

QStringList splitCsv(const QString &line) {
    QStringList result;
    int start = 0;
    bool insideQuote = false;

    for (int i = 0; i <= line.length(); ++i) {
        if (i == line.length() || (!insideQuote && line[i] == ',')) {
            // End of a field
            result.append(line.mid(start, i - start).trimmed());
            start = i + 1;
        } else if (line[i] == '"') {
            // Toggle the quote state
            insideQuote = !insideQuote;
        }
    }

    return result;
}

WeatherData WeatherData::filterByColumnValue(const QString &columnName, const QString &targetValue) const {
    WeatherData filteredData;

    for (int i=0; i<count(); i++) {
        QString value;

        // Retrieve value dynamically based on column name
        if (columnName == "STATION") value = operator[](i).STATION;
        else if (columnName == "DATE") value = at(i).DATE;
        else if (columnName == "REPORT_TYPE") value = at(i).REPORT_TYPE;
        else if (columnName == "SOURCE") value = at(i).SOURCE;
        else if (columnName == "AWND") value = at(i).AWND;
        else if (columnName == "BackupDirection") value = at(i).BackupDirection;
        else if (columnName == "BackupDistance") value = at(i).BackupDistance;
        else if (columnName == "BackupDistanceUnit") value = at(i).BackupDistanceUnit;
        else if (columnName == "BackupElements") value = at(i).BackupElements;
        else if (columnName == "BackupElevation") value = at(i).BackupElevation;
        else if (columnName == "BackupElevationUnit") value = at(i).BackupElevationUnit;
        else if (columnName == "BackupEquipment") value = at(i).BackupEquipment;
        else if (columnName == "BackupLatitude") value = at(i).BackupLatitude;
        else if (columnName == "BackupLongitude") value = at(i).BackupLongitude;
        else if (columnName == "BackupName") value = at(i).BackupName;
        else if (columnName == "CDSD") value = at(i).CDSD;
        else if (columnName == "CLDD") value = at(i).CLDD;
        else if (columnName == "DSNW") value = at(i).DSNW;
        else if (columnName == "DYHF") value = at(i).DYHF;
        else if (columnName == "DYTS") value = at(i).DYTS;
        else if (columnName == "DailyAverageDewPointTemperature") value = at(i).DailyAverageDewPointTemperature;
        else if (columnName == "DailyAverageDryBulbTemperature") value = at(i).DailyAverageDryBulbTemperature;
        else if (columnName == "DailyAverageRelativeHumidity") value = at(i).DailyAverageRelativeHumidity;
        else if (columnName == "DailyAverageSeaLevelPressure") value = at(i).DailyAverageSeaLevelPressure;
        else if (columnName == "DailyAverageStationPressure") value = at(i).DailyAverageStationPressure;
        else if (columnName == "DailyAverageWetBulbTemperature") value = at(i).DailyAverageWetBulbTemperature;
        else if (columnName == "DailyAverageWindSpeed") value = at(i).DailyAverageWindSpeed;
        else if (columnName == "DailyCoolingDegreeDays") value = at(i).DailyCoolingDegreeDays;
        else if (columnName == "DailyDepartureFromNormalAverageTemperature") value = at(i).DailyDepartureFromNormalAverageTemperature;
        else if (columnName == "DailyHeatingDegreeDays") value = at(i).DailyHeatingDegreeDays;
        else if (columnName == "DailyMaximumDryBulbTemperature") value = at(i).DailyMaximumDryBulbTemperature;
        else if (columnName == "DailyMinimumDryBulbTemperature") value = at(i).DailyMinimumDryBulbTemperature;
        else if (columnName == "DailyPeakWindDirection") value = at(i).DailyPeakWindDirection;
        else if (columnName == "DailyPeakWindSpeed") value = at(i).DailyPeakWindSpeed;
        else if (columnName == "DailyPrecipitation") value = at(i).DailyPrecipitation;
        else if (columnName == "DailySnowDepth") value = at(i).DailySnowDepth;
        else if (columnName == "DailySnowfall") value = at(i).DailySnowfall;
        else if (columnName == "DailySustainedWindDirection") value = at(i).DailySustainedWindDirection;
        else if (columnName == "DailySustainedWindSpeed") value = at(i).DailySustainedWindSpeed;
        else if (columnName == "DailyWeather") value = at(i).DailyWeather;
        else if (columnName == "HDSD") value = at(i).HDSD;
        else if (columnName == "HTDD") value = at(i).HTDD;
        else if (columnName == "HourlyAltimeterSetting") value = at(i).HourlyAltimeterSetting;
        else if (columnName == "HourlyDewPointTemperature") value = at(i).HourlyDewPointTemperature;
        else if (columnName == "HourlyDryBulbTemperature") value = at(i).HourlyDryBulbTemperature;
        else if (columnName == "HourlyPrecipitation") value = at(i).HourlyPrecipitation;
        else if (columnName == "HourlyPresentWeatherType") value = at(i).HourlyPresentWeatherType;
        else if (columnName == "HourlyPressureChange") value = at(i).HourlyPressureChange;
        else if (columnName == "HourlyPressureTendency") value = at(i).HourlyPressureTendency;
        else if (columnName == "HourlyRelativeHumidity") value = at(i).HourlyRelativeHumidity;
        else if (columnName == "HourlySeaLevelPressure") value = at(i).HourlySeaLevelPressure;
        else if (columnName == "HourlySkyConditions") value = at(i).HourlySkyConditions;
        else if (columnName == "HourlyStationPressure") value = at(i).HourlyStationPressure;
        else if (columnName == "HourlyVisibility") value = at(i).HourlyVisibility;
        else if (columnName == "HourlyWetBulbTemperature") value = at(i).HourlyWetBulbTemperature;
        else if (columnName == "HourlyWindDirection") value = at(i).HourlyWindDirection;
        else if (columnName == "HourlyWindGustSpeed") value = at(i).HourlyWindGustSpeed;
        else if (columnName == "HourlyWindSpeed") value = at(i).HourlyWindSpeed;
        else {
            qDebug() << "Unknown column name: " << columnName;
            return filteredData; // Return empty instance if column is invalid
        }

        // Check if the value matches the target
        if (value == targetValue) {
            filteredData.append(at(i));
        }
    }

    return filteredData;
}

bool WeatherData::writeCSV(const QString &outputFile, const QString &selectedColumn) {
    QFile file(outputFile);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "File Error: Unable to open file for writing - " << outputFile;
        return false;
    }

    QTextStream out(&file);

    // Write CSV Header
    out << "DateTime," << selectedColumn << "\n";

    for (int i=0; i<count(); i++) {
        QString proper_date = at(i).DATE;
        double excelDate = convertToExcelDateTime(proper_date.remove('"'), "yyyy-MM-ddThh:mm:ss");
        QString columnValue;

        // Retrieve selected column as double
        if (selectedColumn == "DailyPrecipitation") columnValue = at(i).DailyPrecipitation;
        else if (selectedColumn == "HourlyPrecipitation") columnValue = at(i).HourlyPrecipitation;
        else if (selectedColumn == "DailyAverageTemperature") columnValue = at(i).DailyAverageDryBulbTemperature;
        else if (selectedColumn == "HourlyWindSpeed") columnValue = at(i).HourlyWindSpeed;
        else if (selectedColumn == "HourlyRelativeHumidity") columnValue = at(i).HourlyRelativeHumidity;
        else {
            qDebug() << "Error: Unknown column name - " << selectedColumn;
            file.close();
            return false;
        }

        double columnDoubleValue = columnValue.remove('"').toDouble();

        if (excelDate != -1) {
            out << QString::number(excelDate,'g',9) << "," << columnDoubleValue << "\n";
        } else {
            qDebug() << "Warning: Invalid date format in record.";
        }
    }

    file.close();
    qDebug() << "CSV file successfully written to: " << outputFile;
    return true;
}

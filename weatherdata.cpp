#include "weatherdata.h"
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <BTC.h>
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
            data.STATION = cells[0];
            data.DATE = cells[1];
            data.REPORT_TYPE = cells[2];
            data.SOURCE = cells[3];
            data.AWND = cells[4];
            data.BackupDirection = cells[5];
            data.BackupDistance = cells[6];
            data.BackupDistanceUnit = cells[7];
            data.BackupElements = cells[8];
            data.BackupElevation = cells[9];
            data.BackupElevationUnit = cells[10];
            data.BackupEquipment = cells[11];
            data.BackupLatitude = cells[12];
            data.BackupLongitude = cells[13];
            data.BackupName = cells[14];
            data.CDSD = cells[15];
            data.CLDD = cells[16];
            data.DSNW = cells[17];
            data.DYHF = cells[18];
            data.DYTS = cells[19];
            data.DailyAverageDewPointTemperature = cells[20];
            data.DailyAverageDryBulbTemperature = cells[21];
            data.DailyAverageRelativeHumidity = cells[22];
            data.DailyAverageSeaLevelPressure = cells[23];
            data.DailyAverageStationPressure = cells[24];
            data.DailyAverageWetBulbTemperature = cells[25];
            data.DailyAverageWindSpeed = cells[26];
            data.DailyCoolingDegreeDays = cells[27];
            data.DailyDepartureFromNormalAverageTemperature = cells[28];
            data.DailyHeatingDegreeDays = cells[29];
            data.DailyMaximumDryBulbTemperature = cells[30];
            data.DailyMinimumDryBulbTemperature = cells[31];
            data.DailyPeakWindDirection = cells[32];
            data.DailyPeakWindSpeed = cells[33];
            data.DailyPrecipitation = cells[34];
            data.DailySnowDepth = cells[35];
            data.DailySnowfall = cells[36];
            data.DailySustainedWindDirection = cells[37];
            data.DailySustainedWindSpeed = cells[38];
            data.DailyWeather = cells[39];
            data.HDSD = cells[40];
            data.HTDD = cells[41];
            data.HourlyAltimeterSetting = cells[42];
            data.HourlyDewPointTemperature = cells[43];
            data.HourlyDryBulbTemperature = cells[44];
            data.HourlyPrecipitation = cells[45];
            data.HourlyPresentWeatherType = cells[46];
            data.HourlyPressureChange = cells[47];
            data.HourlyPressureTendency = cells[48];
            data.HourlyRelativeHumidity = cells[49];
            data.HourlySeaLevelPressure = cells[50];
            data.HourlySkyConditions = cells[51];
            data.HourlyStationPressure = cells[52];
            data.HourlyVisibility = cells[53];
            data.HourlyWetBulbTemperature = cells[54];
            data.HourlyWindDirection = cells[55];
            data.HourlyWindGustSpeed = cells[56];
            data.HourlyWindSpeed = cells[57];
            data.MonthlyAverageRH = cells[58];
            data.MonthlyDaysWithGT001Precip = cells[59];
            data.MonthlyDaysWithGT010Precip = cells[60];
            data.MonthlyDaysWithGT32Temp = cells[61];
            data.MonthlyDaysWithGT90Temp = cells[62];
            data.MonthlyDaysWithLT0Temp = cells[63];
            data.MonthlyDaysWithLT32Temp = cells[64];
            data.MonthlyDepartureFromNormalAverageTemperature = cells[65];
            data.MonthlyDepartureFromNormalCoolingDegreeDays = cells[66];
            data.MonthlyDepartureFromNormalHeatingDegreeDays = cells[67];
            data.MonthlyDepartureFromNormalMaximumTemperature = cells[68];
            data.MonthlyDepartureFromNormalMinimumTemperature = cells[69];
            data.MonthlyDepartureFromNormalPrecipitation = cells[70];
            data.MonthlyDewpointTemperature = cells[71];
            data.MonthlyGreatestPrecip = cells[72];
            data.MonthlyGreatestPrecipDate = cells[73];
            data.MonthlyGreatestSnowDepth = cells[74];
            data.MonthlyGreatestSnowDepthDate = cells[75];
            data.MonthlyGreatestSnowfall = cells[76];
            data.MonthlyGreatestSnowfallDate = cells[77];
            data.MonthlyMaxSeaLevelPressureValue = cells[78];
            data.MonthlyMaxSeaLevelPressureValueDate = cells[79];
            data.MonthlyMaxSeaLevelPressureValueTime = cells[80];
            data.MonthlyMaximumTemperature = cells[81];
            data.MonthlyMeanTemperature = cells[82];
            data.MonthlyMinSeaLevelPressureValue = cells[83];
            data.MonthlyMinSeaLevelPressureValueDate = cells[84];
            data.MonthlyMinSeaLevelPressureValueTime = cells[85];
            data.MonthlyMinimumTemperature = cells[86];
            data.MonthlySeaLevelPressure = cells[87];
            data.MonthlyStationPressure = cells[88];
            data.MonthlyTotalLiquidPrecipitation = cells[89];
            data.MonthlyTotalSnowfall = cells[90];
            data.MonthlyWetBulb = cells[91];
            data.NormalsCoolingDegreeDay = cells[92];
            data.NormalsHeatingDegreeDay = cells[93];
            data.REM = cells[94];
            data.ShortDurationEndDate005 = cells[95];
            data.ShortDurationEndDate010 = cells[96];
            data.ShortDurationEndDate015 = cells[97];
            data.ShortDurationEndDate020 = cells[98];
            data.ShortDurationEndDate030 = cells[99];
            data.ShortDurationEndDate045 = cells[100];
            data.ShortDurationEndDate060 = cells[101];
            data.ShortDurationEndDate080 = cells[102];
            data.ShortDurationEndDate100 = cells[103];
            data.ShortDurationEndDate120 = cells[104];
            data.ShortDurationEndDate150 = cells[105];
            data.ShortDurationEndDate180 = cells[106];
            data.ShortDurationPrecipitationValue005 = cells[107];
            data.ShortDurationPrecipitationValue010 = cells[108];
            data.ShortDurationPrecipitationValue015 = cells[109];
            data.ShortDurationPrecipitationValue020 = cells[110];
            data.ShortDurationPrecipitationValue030 = cells[111];
            data.ShortDurationPrecipitationValue045 = cells[112];
            data.ShortDurationPrecipitationValue060 = cells[113];
            data.ShortDurationPrecipitationValue080 = cells[114];
            data.ShortDurationPrecipitationValue100 = cells[115];
            data.ShortDurationPrecipitationValue120 = cells[116];
            data.ShortDurationPrecipitationValue150 = cells[117];
            data.ShortDurationPrecipitationValue180 = cells[118];
            data.Sunrise = cells[119];
            data.Sunset = cells[120];
            data.WindEquipmentChangeDate = cells[121];


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
    // Excel's "zero date" is 1900-01-01 (but Excel wrongly treats 1900 as a leap year)
    QDateTime excelZeroDate(QDate(1900, 1, 1), QTime(0, 0));

    // Parse the input dateTime string
    QDateTime inputDateTime = QDateTime::fromString(dateTimeString, format);

    // Ensure the input dateTime is valid
    if (!inputDateTime.isValid()) {
        qDebug() << "Invalid date/time format.";
        return -1; // Invalid input
    }

    QDateTime inputDateTimeWithoutTime(inputDateTime.date(), QTime(0, 0));

    // Get the number of days between the input date and Excel's "zero date"
    qint64 daysSinceExcelZero = excelZeroDate.daysTo(inputDateTimeWithoutTime);

    // Get the fraction of the day (time portion) from the input date/time
    double timeFraction = inputDateTime.time().msecsSinceStartOfDay() / 86400000.0; // 86400000 ms in a day

    // Combine the date (days) and time (fraction of a day)
    double excelDateTime = daysSinceExcelZero + timeFraction;

    // Excel incorrectly treats 1900 as a leap year, so we need to add 1 for dates after 1900-02-28
    if (inputDateTime.date() >= QDate(1900, 2, 29)) {
        excelDateTime += 1.0;
    }

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

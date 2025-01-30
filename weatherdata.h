#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QString>
#include <QVector>

struct DataRecord {
    QString STATION;
    QString DATE;
    QString REPORT_TYPE;
    QString SOURCE;
    QString AWND;
    QString BackupDirection;
    QString BackupDistance;
    QString BackupDistanceUnit;
    QString BackupElements;
    QString BackupElevation;
    QString BackupElevationUnit;
    QString BackupEquipment;
    QString BackupLatitude;
    QString BackupLongitude;
    QString BackupName;
    QString CDSD;
    QString CLDD;
    QString DSNW;
    QString DYHF;
    QString DYTS;
    QString DailyAverageDewPointTemperature;
    QString DailyAverageDryBulbTemperature;
    QString DailyAverageRelativeHumidity;
    QString DailyAverageSeaLevelPressure;
    QString DailyAverageStationPressure;
    QString DailyAverageWetBulbTemperature;
    QString DailyAverageWindSpeed;
    QString DailyCoolingDegreeDays;
    QString DailyDepartureFromNormalAverageTemperature;
    QString DailyHeatingDegreeDays;
    QString DailyMaximumDryBulbTemperature;
    QString DailyMinimumDryBulbTemperature;
    QString DailyPeakWindDirection;
    QString DailyPeakWindSpeed;
    QString DailyPrecipitation;
    QString DailySnowDepth;
    QString DailySnowfall;
    QString DailySustainedWindDirection;
    QString DailySustainedWindSpeed;
    QString DailyWeather;
    QString HDSD;
    QString HTDD;
    QString HourlyAltimeterSetting;
    QString HourlyDewPointTemperature;
    QString HourlyDryBulbTemperature;
    QString HourlyPrecipitation;
    QString HourlyPresentWeatherType;
    QString HourlyPressureChange;
    QString HourlyPressureTendency;
    QString HourlyRelativeHumidity;
    QString HourlySeaLevelPressure;
    QString HourlySkyConditions;
    QString HourlyStationPressure;
    QString HourlyVisibility;
    QString HourlyWetBulbTemperature;
    QString HourlyWindDirection;
    QString HourlyWindGustSpeed;
    QString HourlyWindSpeed;
    QString MonthlyAverageRH;
    QString MonthlyDaysWithGT001Precip;
    QString MonthlyDaysWithGT010Precip;
    QString MonthlyDaysWithGT32Temp;
    QString MonthlyDaysWithGT90Temp;
    QString MonthlyDaysWithLT0Temp;
    QString MonthlyDaysWithLT32Temp;
    QString MonthlyDepartureFromNormalAverageTemperature;
    QString MonthlyDepartureFromNormalCoolingDegreeDays;
    QString MonthlyDepartureFromNormalHeatingDegreeDays;
    QString MonthlyDepartureFromNormalMaximumTemperature;
    QString MonthlyDepartureFromNormalMinimumTemperature;
    QString MonthlyDepartureFromNormalPrecipitation;
    QString MonthlyDewpointTemperature;
    QString MonthlyGreatestPrecip;
    QString MonthlyGreatestPrecipDate;
    QString MonthlyGreatestSnowDepth;
    QString MonthlyGreatestSnowDepthDate;
    QString MonthlyGreatestSnowfall;
    QString MonthlyGreatestSnowfallDate;
    QString MonthlyMaxSeaLevelPressureValue;
    QString MonthlyMaxSeaLevelPressureValueDate;
    QString MonthlyMaxSeaLevelPressureValueTime;
    QString MonthlyMaximumTemperature;
    QString MonthlyMeanTemperature;
    QString MonthlyMinSeaLevelPressureValue;
    QString MonthlyMinSeaLevelPressureValueDate;
    QString MonthlyMinSeaLevelPressureValueTime;
    QString MonthlyMinimumTemperature;
    QString MonthlySeaLevelPressure;
    QString MonthlyStationPressure;
    QString MonthlyTotalLiquidPrecipitation;
    QString MonthlyTotalSnowfall;
    QString MonthlyWetBulb;
    QString NormalsCoolingDegreeDay;
    QString NormalsHeatingDegreeDay;
    QString REM;
    QString ShortDurationEndDate005;
    QString ShortDurationEndDate010;
    QString ShortDurationEndDate015;
    QString ShortDurationEndDate020;
    QString ShortDurationEndDate030;
    QString ShortDurationEndDate045;
    QString ShortDurationEndDate060;
    QString ShortDurationEndDate080;
    QString ShortDurationEndDate100;
    QString ShortDurationEndDate120;
    QString ShortDurationEndDate150;
    QString ShortDurationEndDate180;
    QString ShortDurationPrecipitationValue005;
    QString ShortDurationPrecipitationValue010;
    QString ShortDurationPrecipitationValue015;
    QString ShortDurationPrecipitationValue020;
    QString ShortDurationPrecipitationValue030;
    QString ShortDurationPrecipitationValue045;
    QString ShortDurationPrecipitationValue060;
    QString ShortDurationPrecipitationValue080;
    QString ShortDurationPrecipitationValue100;
    QString ShortDurationPrecipitationValue120;
    QString ShortDurationPrecipitationValue150;
    QString ShortDurationPrecipitationValue180;
    QString Sunrise;
    QString Sunset;
    QString WindEquipmentChangeDate;
};

class WeatherData: public QVector<DataRecord>
{
public:
    WeatherData();
    bool ReadFromFile(const QString &fileName);
    WeatherData filterByColumnValue(const QString &columnName, const QString &targetValue) const;
    bool writeCSV(const QString &outputFile, const QString &selectedColumn);

};

QStringList splitCsv(const QString &line);
double convertToExcelDateTime(const QString &dateTimeString, const QString& format);
#endif // WEATHERDATA_H

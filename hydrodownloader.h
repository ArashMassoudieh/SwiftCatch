#ifndef HYDRODOWNLOADER_H
#define HYDRODOWNLOADER_H

#include <QString>
#include <QDateTime>


struct station_info
{
    QString agency_cd;
    QString site_no;
    QString station_nm;
    QString site_tp_cd;
    double dec_lat_va;
    double dec_long_va;
    QString coord_acy_cd;
    QString ddec_coord_datum_cd;
    QString alt_va;
    QString alt_acy_va;
    QString alt_datum_cd;
    QString huc_cd;
};

struct FlowData {
    QDateTime dateTime;
    double flowRate;
};

struct HydroStationData {
    QString parameterName;  // Name of the parameter (e.g., "Streamflow")
    QString parameterCode;  // Parameter code (e.g., "00060")
    QVector<QPair<QDateTime, double>> values;  // Timestamps and their corresponding values
};

struct WeatherStationData {
    QString id;         // Station ID
    QString name;       // Station Name
    double latitude;    // Latitude
    double longitude;   // Longitude
    double elevation;   // Elevation in meters
    QString mindate;    // Earliest data available
    QString maxdate;    // Latest data available
    double datacoverage; // Data coverage percentage};
};


struct State_Info
{
    QString Name;
    QString Code;
    QString FIPS;
};

struct PrecipitationData {
    QDateTime dateTime;  // Timestamp of the measurement
    double precipitation; // Precipitation value in mm
};


struct DatasetDatatype {
    QString datasetId;  // Dataset ID (e.g., "GHCND")
    QString datatypeId; // Datatype ID (e.g., "PRCP")
};

struct DataType {
    QString id;         // Data type ID (e.g., PRCP)
    QString name;       // Description (e.g., Precipitation)
    QString max_date;   // MaxDate
    QString min_date;   // MinDate
};

enum class precip_time_interval {PRECIP_15, HLY, PRECIP_5};
class HydroDownloader
{
public:
    HydroDownloader();
    QMap<QString, station_info>  fetchAllHydroStations(const QString &state);
    QVector<FlowData> fetchFlowData(const QString& stationId, const QString& startDate, const QString& endDate);
    QVector<HydroStationData> fetchAllStationData(const QString& stationId, const QString& startDate = "", const QString& endDate = "");
    QMap<QString,WeatherStationData> fetchNOAAStations(const QString& stateCode, const QString& apiToken);
    QVector<PrecipitationData> fetchPrecipitationData(const QString& stationId, const QString& startDate, const QString& endDate, const QString& apiToken, precip_time_interval interval = precip_time_interval::HLY);
    QSet<DatasetDatatype> fetchDatasetAndDatatype(const QString& stationId, const QString& apiToken);
    QSet<DataType> fetchAllDataTypesForStation(const QString& stationId, const QString& apiToken);
    QMap<QString, WeatherStationData> fetchPrecipitationStations(const QString& FIPS, const QString& apiToken, precip_time_interval interval = precip_time_interval::HLY);



};

bool operator<(const DatasetDatatype& lhs, const DatasetDatatype& rhs);
bool operator==(const DatasetDatatype& lhs, const DatasetDatatype& rhs);
uint qHash(const DatasetDatatype& key, uint seed = 0);
// Overload operators to use DataType in QSet
bool operator<(const DataType& lhs, const DataType& rhs);
bool operator==(const DataType& lhs, const DataType& rhs);
uint qHash(const DataType& key, uint seed = 0);
double qDateTimeToExcel(const QDateTime& dateTime);
QDateTime excelToQDateTime(double excelDate);

#endif // HYDRODOWNLOADER_H

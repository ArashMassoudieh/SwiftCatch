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

struct StationData {
    QString parameterName;  // Name of the parameter (e.g., "Streamflow")
    QString parameterCode;  // Parameter code (e.g., "00060")
    QVector<QPair<QDateTime, double>> values;  // Timestamps and their corresponding values
};

class HydroDownloader
{
public:
    HydroDownloader();
    QMap<QString, station_info>  fetchAllStations(const QString &state);
    QVector<FlowData> fetchFlowData(const QString& stationId, const QString& startDate, const QString& endDate);
    QVector<StationData> fetchAllStationData(const QString& stationId, const QString& startDate = "", const QString& endDate = "");
};

double qDateTimeToExcel(const QDateTime& dateTime);
QDateTime excelToQDateTime(double excelDate);

#endif // HYDRODOWNLOADER_H

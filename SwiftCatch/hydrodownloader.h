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

class HydroDownloader
{
public:
    HydroDownloader();
    QVector<station_info>  fetchAllStations(const QString &state);
};

#endif // HYDRODOWNLOADER_H

#ifndef HYDRODOWNLOADERDLG_H
#define HYDRODOWNLOADERDLG_H

#include <QDialog>
#include <QNetworkReply>
#include "hydrodownloader.h"
#include <QMap>
#include "BTC.h"

namespace Ui {
class HydroDownloaderDlg;
}

class HydroDownloaderDlg : public QDialog
{
    Q_OBJECT

public:
    explicit HydroDownloaderDlg(QWidget *parent = nullptr);
    ~HydroDownloaderDlg();
    void fetchStateCodes(const QString& url);
    void showGraph(const CTimeSeries<double> &data);

private:
    Ui::HydroDownloaderDlg *ui;
    //QStringList stateCodes;
    QNetworkReply *reply;
    QNetworkAccessManager *manager;
    QMap<QString, station_info> stations;
    CTimeSeries<double> UniformizedTimeSeries;
public slots:
    void on_State_Changed();
    void on_States_Downloaded();
    void on_Station_Selected();
    void on_DataRetrieveRequested();
    void on_Date_Changed();
    void on_ExporttoCSV();

signals:
    void downloadingstatesfinished();
};

#endif // HYDRODOWNLOADERDLG_H

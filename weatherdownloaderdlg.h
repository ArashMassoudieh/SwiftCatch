#ifndef WEATHERDOWNLOADERDLG_H
#define WEATHERDOWNLOADERDLG_H
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "hydrodownloader.h"

#include <QDialog>

namespace Ui {
class WeatherDownloaderDlg;
}

class WeatherDownloaderDlg : public QDialog
{
    Q_OBJECT

public:
    explicit WeatherDownloaderDlg(QWidget *parent = nullptr);
    ~WeatherDownloaderDlg();

private:
    Ui::WeatherDownloaderDlg *ui;
    void fetchStateCodes(const QString& url);
    QNetworkReply *reply;
    QNetworkAccessManager *manager;
    QMap<QString,WeatherStationData> stations;
    QMap<QString, State_Info> StatesInformation;

public slots:
    void on_States_Downloaded();
    void on_State_Changed();
    void on_Station_Selected();
    void on_Retreive_Data();
    void on_Date_Changed();
};

#endif // WEATHERDOWNLOADERDLG_H

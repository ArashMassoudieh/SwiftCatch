#ifndef HYDRODOWNLOADERDLG_H
#define HYDRODOWNLOADERDLG_H

#include <QDialog>
#include <QNetworkReply>

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

private:
    Ui::HydroDownloaderDlg *ui;
    QStringList stateCodes;
    QNetworkReply *reply;
    QNetworkAccessManager *manager;

public slots:
    void on_State_Changed();
    void on_States_Downloaded();

signals:
    void downloadingstatesfinished();
};

#endif // HYDRODOWNLOADERDLG_H

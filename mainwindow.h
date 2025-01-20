#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class HydroDownloaderDlg;
class WeatherDownloaderDlg;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    HydroDownloaderDlg *hydrodownloaderdlg = nullptr;
    WeatherDownloaderDlg *weatherdownloaderdlg = nullptr;


public slots:
    void on_Download_flow_data();
    void on_Download_Weather_data();
    void on_Download_GeoTIFF();
};
#endif // MAINWINDOW_H

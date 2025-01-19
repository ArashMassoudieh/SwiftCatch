#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class HydroDownloaderDlg;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    HydroDownloaderDlg *hydrodownloaderdlg = nullptr;

public slots:
    void on_Upload_flow_data();
};
#endif // MAINWINDOW_H

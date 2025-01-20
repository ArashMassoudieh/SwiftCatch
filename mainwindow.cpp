#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hydrodownloaderdlg.h"
#include "weatherdownloaderdlg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionUpload_flow_data, SIGNAL(triggered()), this, SLOT(on_Download_flow_data()) );
    connect(ui->actionDownload_Weather_Data, SIGNAL(triggered()), this, SLOT(on_Download_Weather_data()) );
}

MainWindow::~MainWindow()
{
    delete hydrodownloaderdlg;
    delete ui;

}

void MainWindow::on_Download_flow_data()
{
    hydrodownloaderdlg = new HydroDownloaderDlg(this);
    hydrodownloaderdlg->show();
}

void MainWindow::on_Download_Weather_data()
{
    weatherdownloaderdlg = new WeatherDownloaderDlg(this);
    weatherdownloaderdlg->show();
}

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "hydrodownloaderdlg.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionUpload_flow_data, SIGNAL(triggered()), this, SLOT(on_Upload_flow_data()) );
}

MainWindow::~MainWindow()
{
    delete hydrodownloaderdlg;
    delete ui;

}

void MainWindow::on_Upload_flow_data()
{
    hydrodownloaderdlg = new HydroDownloaderDlg(this);
    hydrodownloaderdlg->show();
}

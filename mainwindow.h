#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QFileInfo>

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
    void on_Load_GeoTIFF();
    void on_Uniformized();
    void on_ReadWeatherData();
    void on_Select_Area();
    void on_Load_Transportation_Layer();
    void on_Find_Closest_Sewers();
};

enum class DialogMode { Open, Save };

inline QString getFileNameWithExtension(QWidget* parent,
                                        const QString& title,
                                        const QString& dir,
                                        const QString& filter,
                                        DialogMode mode,
                                        const QString& defaultExt = "tif")
{
    QString selectedFilter;
    QString fileName;

    if (mode == DialogMode::Save) {
        fileName = QFileDialog::getSaveFileName(parent, title, dir, filter, &selectedFilter);
    } else {
        fileName = QFileDialog::getOpenFileName(parent, title, dir, filter, &selectedFilter);
    }

    if (!fileName.isEmpty() && QFileInfo(fileName).suffix().isEmpty()) {
        if (selectedFilter.contains("*.tiff"))
            fileName += ".tiff";
        else if (selectedFilter.contains("*.tif"))
            fileName += ".tif";
        else
            fileName += "." + defaultExt;
    }

    if (mode == DialogMode::Open) {
        // Ensure the file exists when opening
        if (!fileName.isEmpty() && !QFile::exists(fileName)) {
            return "";
        }
    }

    return fileName;
}

inline QString changeExtension(const QString& fileName, const QString& newExt)
{
    QFileInfo fi(fileName);
    QString base = fi.completeBaseName();  // file name without extension(s)
    QString dir  = fi.path();              // directory path

    QString ext = newExt;
    if (!ext.startsWith(".")) {
        ext = "." + ext;
    }

    if (dir == "." || dir.isEmpty()) {
        return base + ext;
    } else {
        return dir + "/" + base + ext;
    }
}


#endif // MAINWINDOW_H

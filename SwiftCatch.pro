QT       += core gui widgets network charts quick quickwidgets positioning location

CONFIG += c++17

CONFIG += qtquickcompiler

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += /usr/include/gdal

SOURCES += \
    GeoDataModel.cpp \
    GeoDataSetInterface.cpp \
    MapDialog.cpp \
    OpenGLGeoWidget.cpp \
    PointGeoDataSet.cpp \
    TableViewer.cpp \
    Utilities/QuickSort.cpp \
    Utilities/Utilities.cpp \
    geodatadownloader.cpp \
    geomertymapviewer.cpp \
    geometrybase.cpp \
    geometrymapdialog.cpp \
    geometryviewerwidget.cpp \
    geotiffhandler.cpp \
    hydrodownloader.cpp \
    hydrodownloaderdlg.cpp \
    junction.cpp \
    junctionset.cpp \
    main.cpp \
    mainwindow.cpp \
    mapwidget.cpp \
    modelcreator.cpp \
    node.cpp \
    path.cpp \
    polyline.cpp \
    polylinegeodataset.cpp \
    polylineset.cpp \
    streamnetwork.cpp \
    weatherdata.cpp \
    weatherdownloaderdlg.cpp

HEADERS += \
    GeoDataModel.h \
    GeoDataSetInterface.h \
    MapDialog.h \
    OpenGLGeoWidget.h \
    PointGeoDataSet.h \
    TableViewer.h \
    Utilities/BTC.h \
    Utilities/BTC.hpp \
    Utilities/BTCSet.h \
    Utilities/BTCSet.hpp \
    Utilities/QuickSort.h \
    Utilities/Utilities.h \
    geodatadownloader.h \
    geomertymapviewer.h \
    geometrybase.h \
    geometrymapdialog.h \
    geometryviewerwidget.h \
    geotiffhandler.h \
    hydrodownloader.h \
    hydrodownloaderdlg.h \
    junction.h \
    junctionset.h \
    mainwindow.h \
    mapwidget.h \
    modelcreator.h \
    node.h \
    path.h \
    polyline.h \
    polylinegeodataset.h \
    polylineset.h \
    streamnetwork.h \
    weatherdata.h \
    weatherdownloaderdlg.h

FORMS += \
    hydrodownloaderdlg.ui \
    mainwindow.ui \
    weatherdownloaderdlg.ui

TRANSLATIONS += \
    SwiftCatch_en_US.ts

LIBS += -lgdal

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    MapView.qrc

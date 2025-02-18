QT       += core gui widgets network charts


CONFIG += c++17

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
    hydrodownloader.cpp \
    hydrodownloaderdlg.cpp \
    main.cpp \
    mainwindow.cpp \
    polylinegeodataset.cpp \
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
    hydrodownloader.h \
    hydrodownloaderdlg.h \
    mainwindow.h \
    polylinegeodataset.h \
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

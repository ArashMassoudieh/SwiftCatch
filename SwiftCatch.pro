QT       += core gui widgets network charts


CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../Utilities
INCLUDEPATH += /usr/include/gdal

SOURCES += \
    ../Utilities/Utilities.cpp \
    geodatadownloader.cpp \
    hydrodownloader.cpp \
    hydrodownloaderdlg.cpp \
    main.cpp \
    mainwindow.cpp \
    weatherdata.cpp \
    weatherdownloaderdlg.cpp

HEADERS += \
    geodatadownloader.h \
    hydrodownloader.h \
    hydrodownloaderdlg.h \
    mainwindow.h \
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

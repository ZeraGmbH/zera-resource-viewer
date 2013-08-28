include(resource-viewer.user.pri)

QT += core gui network

SOURCES += \
    main.cpp \
    resourceviewer.cpp \
    scpiclient.cpp \
    settingsdialog.cpp

HEADERS += \
    resourceviewer.h \
    scpiclient.h \
    settingsdialog.h

FORMS += \
    resource-viewer.ui \
    settingsdialog.ui

INCLUDEPATH += $${ZERANETCLIENT_INCLUDEPATH}
INCLUDEPATH += $${ZERA_PROTOBUF_INCLUDEPATH}
INCLUDEPATH += $${SCPI_INCLUDEPATH}

LIBS += -lprotobuf
LIBS += $${ZERANETCLIENT_LIBPATH} -lzeranetclient
LIBS += $${ZERA_PROTOBUF_LIBPATH} -lzera-resourcemanager-protobuf
LIBS += $${SCPI_LIBPATH} -lSCPI

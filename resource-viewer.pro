include(../include/project-paths.pri)

QT += core gui network widgets

VERSION = 0.9.0

SOURCES += \
    main.cpp \
    resourceviewer.cpp \
    scpiclient.cpp \
    settingsdialog.cpp \
    loghelper.cpp \
    rmprotobufwrapper.cpp

HEADERS += \
    resourceviewer.h \
    scpiclient.h \
    settingsdialog.h \
    loghelper.h \
    rmprotobufwrapper.h

FORMS += \
    resource-viewer.ui \
    settingsdialog.ui

INCLUDEPATH += $${PROTONET_INCLUDEDIR}
INCLUDEPATH += $${RESOURCE_PROTOBUF_INCLUDEDIR}
INCLUDEPATH += $${SCPI_INCLUDEDIR}

LIBS += -lprotobuf
LIBS += $${PROTONET_LIBDIR} -lproto-net-qt
LIBS += $${RESOURCE_PROTOBUF_LIBDIR} -lzera-resourcemanager-protobuf
LIBS += $${SCPI_LIBDIR} -lSCPI

target.path = /usr/bin
INSTALLS += target

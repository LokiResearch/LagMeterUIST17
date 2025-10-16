#
# Authors: Géry Casiez
#
# Copyright © Inria

TEMPLATE  = app
CONFIG   -= app_bundle
QT -= core gui

# hidapi libpointing
macx {
    # Prepare for Homebrew packages (macx = OS X only, not iOS)
    INCLUDEPATH += /usr/local/include
    LIBS += -L/usr/local/lib

    # Prepare for macport
    INCLUDEPATH += /opt/local/include
    LIBS += -L/opt/local/lib

    LIBS += -lpointing
    LIBS += -lhidapi
} else:unix {
        LIBS += -lhidapi-libusb
        LIBS += -lpointing -L/home/dmarchal/projects/2016/LagMeter/libpointing/libpointing/build/pointing
        INCLUDEPATH += /home/dmarchal/projects/2016/LagMeter/libpointing/libpointing
} else:windows {
    INCLUDEPATH += "..\external\windows\include"
    INCLUDEPATH += "..\external\windows\include\libpointing"

    LIBS += -L"..\external\windows\lib\x64"
    LIBS += -lhidapi -lpointing
}


HEADERS += ../TurboHID/TurboHID.h
SOURCES += main.cpp \
           ../TurboHID/TurboHID.cpp
INCLUDEPATH += ../TurboHID/

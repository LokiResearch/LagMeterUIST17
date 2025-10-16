# LagMeter.pro --
#
# Authors: Nicolas Roussel, Géry Casiez
#
# Copyright © Inria

TEMPLATE = app

CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += ../TurboHID/
HEADERS   += ../TurboHID/TurboHID.h
SOURCES   += ../TurboHID/TurboHID.cpp

#HEADERS   += LagMeter.h
SOURCES   += main.cpp

# remove possible other optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O
QMAKE_CXXFLAGS_RELEASE -= -O1
QMAKE_CXXFLAGS_RELEASE -= -O2 
QMAKE_CXXFLAGS_RELEASE *= -O3

windows {

  # FIXME: Do something...

} else:mac {

  QMAKE_CXXFLAGS += -Wno-deprecated-declarations

  # Look for pkg-config in Fink, Macports and Homebrew (the last one we find wins)
  exists(/sw/bin/pkg-config) {
      QMAKE_PKG_CONFIG = /sw/bin/pkg-config
      INCLUDEPATH += /sw/include            
  }
  exists(/opt/local/bin/pkg-config) {
	  QMAKE_PKG_CONFIG = /opt/local/bin/pkg-CONFIG
      INCLUDEPATH += /opt/local/include      
  }
  exists(/usr/local/bin/pkg-config) {
      QMAKE_PKG_CONFIG = /usr/local/bin/pkg-config
      INCLUDEPATH += /usr/local/include
  }

  message("Looking for dependencies using $$QMAKE_PKG_CONFIG")

  system($$QMAKE_PKG_CONFIG --exists hidapi) {
    QMAKE_CXXFLAGS += $$system("$$QMAKE_PKG_CONFIG --cflags hidapi")
    LIBS += $$system("$$QMAKE_PKG_CONFIG --libs hidapi")
    DEFINES += LIBLAG_HAS_HIDAPI
    CONFIG += hidapi
    message("HIDAPI support enabled")	
  }

  LIBS += -framework IOKit -framework GLUT -framework OpenGL -framework Cocoa -lhidapi

} else:linux-*  {

  # Look for pkg-config in the usual places
  exists(/usr/bin/pkg-config) { QMAKE_PKG_CONFIG = /usr/bin/pkg-config }
  exists(/usr/local/bin/pkg-config) { QMAKE_PKG_CONFIG = /usr/local/bin/pkg-config }
  message("Looking for dependencies using $$QMAKE_PKG_CONFIG")

  # Use hidapi-libusb, not hidapi-raw (which does not work with some devices)
  # See https://github.com/signal11/hidapi
  system($$QMAKE_PKG_CONFIG --exists hidapi-libusb) {
    QMAKE_CXXFLAGS += $$system("$$QMAKE_PKG_CONFIG --cflags hidapi-libusb")
    LIBS += $$system("$$QMAKE_PKG_CONFIG --libs hidapi-libusb")
    DEFINES += LIBLAG_HAS_HIDAPI
    CONFIG += hidapi
    message("HIDAPI support enabled")	
  }

}

!ios:mac|linux-* {
  POINTING = $$(POINTING_HOME) # Try this environment variable first
  exists($$POINTING/pointing/pointing.pri) {
    include($$POINTING/pointing/pointing.pri)
    DEFINES += LIBLAG_HAS_LIBPOINTING
    CONFIG += libpointing
    message("Libpointing support enabled (found in in $POINTING_HOME=$$POINTING)")
  } else {
    # The Homebrew (macOS) and Debian packages are installed in /usr/local
    exists(/usr/local/include/pointing/pointing.h) {
      LIBS += -L/usr/local/lib
      LIBS += -lpointing
      DEFINES += LIBLAG_HAS_LIBPOINTING
      CONFIG += libpointing
      message("Libpointing support enabled (found in /usr/local)")
    }
  }
} else {
  warning("Libpointing not supported on this platform")
  DEFINES += NO_LIBPOINTING
  HEADERS += utils/ConfigDict.h   utils/TimeStamp.h
  SOURCES += utils/ConfigDict.cpp utils/TimeStamp.cpp
}

android {

  QT += core

  DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

  config.files += Android.cfg
  config.path = /assets
  INSTALLS += config

  ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

}

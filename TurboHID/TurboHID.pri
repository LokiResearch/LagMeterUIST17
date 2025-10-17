windows {

  INCLUDEPATH += "external\windows"
  INCLUDEPATH += "external\windows\libpointing"

  LIBS += "..\cpp\external\windows\libpointing\libpointing.a"
  LIBS += "..\cpp\external\windows\hidapi\hidapi.lib"

} else:mac {

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
  exists(/opt/homebrew/bin/pkg-config) {
      QMAKE_PKG_CONFIG = /opt/homebrew/bin/pkg-config
      INCLUDEPATH += /opt/homebrew/include
  }

  message("Looking for dependencies using $$QMAKE_PKG_CONFIG")

  system($$QMAKE_PKG_CONFIG --exists hidapi) {
    QMAKE_CXXFLAGS += $$system("$$QMAKE_PKG_CONFIG --cflags hidapi")
    LIBS += $$system("$$QMAKE_PKG_CONFIG --libs hidapi")
    DEFINES += LIBLAG_HAS_HIDAPI
    CONFIG += hidapi
    message("HIDAPI support enabled")	
  }

  # Gery: ca ne fonctionne pas sur ma machine si je n'ajoute pas la ligne ci-dessous
  # Nicolas: Peux-tu faire "brew info hidapi" dans un terminal ?
  LIBS += -lhidapi -framework ApplicationServices

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
} else:!windows {
  warning("Libpointing not supported on this platform")
}

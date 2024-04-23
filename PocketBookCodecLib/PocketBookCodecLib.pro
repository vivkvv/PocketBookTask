QT -= gui

TEMPLATE = lib
CONFIG += staticlib

CONFIG += c++11

BIN_DIR = $$PWD/../PocketBookCodecLibBins

DESTDIR = $$BIN_DIR/$$CONFIG

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    pocket_book_codec_lib.cpp

HEADERS += \
    pocket_book_codec_lib.h

CONFIG(debug, debug|release): {
    DESTDIR = $$BIN_DIR/debug
}

CONFIG(release, debug|release): {
    DESTDIR = $$BIN_DIR/release
}

OBJECTS_DIR = $$DESTDIR

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

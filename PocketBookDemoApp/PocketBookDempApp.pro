TEMPLATE = app

QT += qml quick
CONFIG += c++11

DESTDIR = ../

POCKETBOOK_PLUGIN_DIR = $$PWD/../PocketBookDemoPlugin


LibBinsDir = PocketBookCodecLibBins

CONFIG(debug, debug|release): {
    LibBinsDir = $$LibBinsDir/debug
}

CONFIG(release, debug|release): {
    LibBinsDir = $$LibBinsDir/release
}


POCKETBOOK_CODEC_HEADERS_DIR = $$PWD/../PocketBookCodecLib
POCKETBOOK_CODEC_LIBS_DIR = $$PWD/../$$LibBinsDir

INCLUDEPATH += $$POCKETBOOK_PLUGIN_DIR
INCLUDEPATH += $$POCKETBOOK_CODEC_HEADERS_DIR

LIBS += -L$$PWD/../$$LibBinsDir -lPocketBookCodecLib

SOURCES += main.cpp \
    $$POCKETBOOK_PLUGIN_DIR/file_list_model.cpp \
    $$POCKETBOOK_PLUGIN_DIR/file_list_filter_proxy_model.cpp \
    pocket_qml_processor.cpp \
    workers.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    $$POCKETBOOK_PLUGIN_DIR/file_list_model.h \
    $$POCKETBOOK_PLUGIN_DIR/file_list_filter_proxy_model.h \
    pocket_qml_processor.h \
    workers.h


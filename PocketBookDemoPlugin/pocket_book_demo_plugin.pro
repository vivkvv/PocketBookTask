TEMPLATE = lib
TARGET = PocketBookDemoPlugin
QT += qml quick
CONFIG += qt plugin c++11

BIN_DIR = $$PWD/../PluginBins

DESTDIR = $$BIN_DIR

OBJECTS_DIR = $$DESTDIR

TARGET = $$qtLibraryTarget($$TARGET)
uri = PocketBookTestDemoPlugin__

# Input
SOURCES += \
    file_list_model.cpp \
    pocket_book_demo_plugin.cpp

HEADERS += \
    file_list_model.h \
    pocket_book_demo_plugin.h

DISTFILES = qmldir

!equals(_PRO_FILE_PWD_, $$OUT_PWD) {
    copy_qmldir.target = $$OUT_PWD/qmldir
    copy_qmldir.depends = $$_PRO_FILE_PWD_/qmldir
    copy_qmldir.commands = $(COPY_FILE) \"$$replace(copy_qmldir.depends, /, $$QMAKE_DIR_SEP)\" \"$$replace(copy_qmldir.target, /, $$QMAKE_DIR_SEP)\"
    QMAKE_EXTRA_TARGETS += copy_qmldir
    PRE_TARGETDEPS += $$copy_qmldir.target
}

qmldir.files=qmldir
unix {
    installPath = $$[QT_INSTALL_QML]/$$replace(uri, \\., /)
    qmldir.path = $$installPath
    target.path = $$installPath
    INSTALLS += target qmldir
}

# Copy the qmldir file to the same folder as the plugin binary
cpqmldir.files = qmldir
cpqmldir.path = $$DESTDIR
COPIES += cpqmldir

RESOURCES += \
    qml.qrc

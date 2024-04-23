#ifndef POCKET_BOOK_DEMO_PLUGIN_H
#define POCKET_BOOK_DEMO_PLUGIN_H

#include <QQmlExtensionPlugin>

class PocketBookDemoPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri);
};

#endif // POCKET_BOOK_DEMO_PLUGIN_H

#include "pocket_book_demo_plugin.h"
#include "file_list_model.h"
#include "file_list_filter_proxy_model.h"

#include <qqml.h>

void PocketBookDemoPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<FileListModel>(uri, 1, 0, "FileListModel");
    qmlRegisterType<FileListFilterProxyModel>(uri, 1, 0, "FileListFilterProxyModel");
}


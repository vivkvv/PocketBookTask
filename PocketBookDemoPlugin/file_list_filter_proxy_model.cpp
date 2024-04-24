#include "file_list_filter_proxy_model.h"

FileListFilterProxyModel::FileListFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent){
}

Q_INVOKABLE void FileListFilterProxyModel::setFilterByExtension(
    const QString& extensions) {

    QStringList list = extensions.split(';');

    QStringList regexPatterns;
    for(const QString& ext: list){
        QString cleanExt = ext.trimmed();
        if(cleanExt.startsWith("*.")){
            cleanExt.remove(0, 2);
        }
        regexPatterns.append(QRegularExpression::escape(cleanExt).replace("\\*", ".*"));
    }

    QString regexStr = ".*\\.(?:" + regexPatterns.join("|") + ")(?!\\S)";
    setFilterRegularExpression(QRegularExpression(
        regexStr,
        QRegularExpression::CaseInsensitiveOption));
    setFilterKeyColumn(0);
}

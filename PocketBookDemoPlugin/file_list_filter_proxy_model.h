#ifndef FILELISTFILTERPROXYMODEL_H
#define FILELISTFILTERPROXYMODEL_H

#include <QObject>
#include <QSortFilterProxyModel>

class FileListFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    FileListFilterProxyModel(QObject* parent = nullptr);

    Q_INVOKABLE void setFilterByExtension(const QString& extensions);
};

#endif // FILELISTFILTERPROXYMODEL_H

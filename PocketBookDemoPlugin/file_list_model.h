#ifndef FILELISTMODEL_H
#define FILELISTMODEL_H

#include <QAbstractListModel>
#include <QMetaType>
#include <QQmlApplicationEngine>

enum class CodingState {
    csNone = 0,
    csCoding = 1,
    csDecoding = 2
};

enum class OperationResultState {
    csNone = -1,
    orsCantLoadInputImage = -2,
    orsCantOpenBarchAsFileStream = -3,
    orsBarchIsNotExists = -4,
    orsBarchHeaderTooSmall = -5,
    orsBarchRowIndexTooSmall = -6,
    orsCantLoadBarchRowIndex = -7,
    orsGeneralFileStreamError = -8
};

struct FileItem {
    QString name;
    qint64 size;
    CodingState codingState;
    OperationResultState resultState = OperationResultState::csNone;
};

class FileListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit FileListModel(const QQmlApplicationEngine& engine, QObject *parent = nullptr);

    Q_INVOKABLE void setFilesList(const QList<FileItem>& fileDetails);
    void addFile(const FileItem& fileDetails);

    void updateFileState(int idx, CodingState codingState);
    void updateResultState(int idx, int resultState);

    const FileItem* const getFileItem(int idx) const;

private:

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    QList<FileItem> m_filesList;
    const QQmlApplicationEngine& m_engine;
};

Q_DECLARE_METATYPE(FileItem)

#endif // FILELISTMODEL_H

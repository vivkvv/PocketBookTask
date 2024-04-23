#include "file_list_model.h"

#include <QQmlComponent>
#include <QQuickWindow>

FileListModel::FileListModel(const QQmlApplicationEngine& engine, QObject *parent)
    : QAbstractListModel(parent)
    , m_engine(engine) {
}

Q_INVOKABLE void FileListModel::setFilesList(const QList<FileItem>& fileDetails) {
    beginResetModel();
    m_filesList = fileDetails;
    endResetModel();
}

void FileListModel::addFile(const FileItem& fileDetails) {
    auto it = std::find_if(m_filesList.begin(), m_filesList.end(), [&fileDetails](const FileItem& fi){
        return fi.name == fileDetails.name;
    });

    if(it != m_filesList.end()){
        return;
    }

    beginResetModel();
    m_filesList.append(fileDetails);
    endResetModel();
}

const FileItem* const FileListModel::getFileItem(int idx) const {
    if(idx < 0 || idx >= m_filesList.size()){
        return nullptr;
    }

    return &m_filesList[idx];
}

void FileListModel::updateResultState(int idx, int resultState) {
    if(idx < 0 || idx >= m_filesList.size()){
        return;
    }

    m_filesList[idx].resultState = static_cast<OperationResultState>(resultState);

    QModelIndex modelIndex = createIndex(idx, 0);
    emit dataChanged(modelIndex, modelIndex);
}

void FileListModel::updateFileState(int idx, CodingState codingState){
    if(idx < 0 || idx >= m_filesList.size()){
        return;
    }

    m_filesList[idx].codingState = codingState;

    QModelIndex modelIndex = createIndex(idx, 0);
    emit dataChanged(modelIndex, modelIndex);
}

QHash<int, QByteArray> FileListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::UserRole] = "state";
    roles[Qt::UserRole + 1] = "resultCode";
    return roles;
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()){
        return 0;
    }

    return static_cast<int>(m_filesList.size());
}

QVariant FileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_filesList.size()) {
        return QVariant();
    }

    const FileItem& item = m_filesList[index.row()];


    switch(role) {
        case Qt::DisplayRole:
            return QString("%1 (%2 bytes)").arg(item.name).arg(item.size);
        case Qt::UserRole:
            return QVariant::fromValue(static_cast<int>(item.codingState));
        case Qt::UserRole + 1:
            return static_cast<int>(item.resultState);
    }

    return QVariant();
}

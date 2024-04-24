#include "file_list_model.h"

#include <QQmlComponent>
#include <QQuickWindow>
#include <QLocale>

FileListModel::FileListModel(const QQmlApplicationEngine& engine, QObject *parent)
    : QAbstractListModel(parent)
    , mEngine(engine) {
}

Q_INVOKABLE void FileListModel::setFilesList(const QList<FileItem>& fileDetails) {
    beginResetModel();
    mFilesList = fileDetails;
    endResetModel();
}

void FileListModel::addFile(const FileItem& fileDetails) {
    auto it = std::find_if(mFilesList.begin(), mFilesList.end(), [&fileDetails](const FileItem& fi){
        return fi.name == fileDetails.name;
    });

    if(it != mFilesList.end()){
        return;
    }

    beginResetModel();
    mFilesList.append(fileDetails);
    endResetModel();
}

int FileListModel::getFileIdx(const QString& fileName) const {
    int idx = -1;
    for(int i = 0; i < mFilesList.size(); ++i){
        if(mFilesList[i].name == fileName){
            idx = i;
            break;
        };
    };

    return idx;
}

void FileListModel::updateResultState(const QString& fileName, int resultState) {

    int idx = getFileIdx(fileName);

    if(idx < 0){
        return;
    }

    mFilesList[idx].resultState = static_cast<OperationResultState>(resultState);

    QModelIndex modelIndex = createIndex(idx, 0);
    emit dataChanged(modelIndex, modelIndex);
}

void FileListModel::updateFileState(const QString& fileName, CodingState codingState){

    int idx = getFileIdx(fileName);

    if(idx < 0){
        return;
    }

    mFilesList[idx].codingState = codingState;

    QModelIndex modelIndex = createIndex(idx, 0);
    emit dataChanged(modelIndex, modelIndex);
}

QHash<int, QByteArray> FileListModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "display";
    roles[Qt::UserRole] = "size";
    roles[Qt::UserRole + 1] = "state";
    roles[Qt::UserRole + 2] = "resultCode";
    return roles;
}

int FileListModel::rowCount(const QModelIndex& parent) const
{
    if(parent.isValid()){
        return 0;
    }

    return static_cast<int>(mFilesList.size());
}

QVariant FileListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= mFilesList.size()) {
        return QVariant();
    }

    const FileItem& item = mFilesList[index.row()];


    switch(role) {
        case Qt::DisplayRole:
            return item.name;
        case Qt::UserRole:
            return QLocale().toString(static_cast<double>(item.size), 'f', 0);
        case Qt::UserRole + 1:
            return QVariant::fromValue(static_cast<int>(item.codingState));
        case Qt::UserRole + 2:
            return static_cast<int>(item.resultState);
    }

    return QVariant();
}

#include "pocket_qml_processor.h"
#include <QQmlComponent>
#include <QQuickWindow>

#include <cstring>
#include <fstream>
#include <iostream>

#include <QThread>
#include "pocket_book_codec_lib.h"


PocketQmlProcessor::PocketQmlProcessor(FileListModel& model,
                                       QQmlApplicationEngine& engine,
                                       QObject *parent)
    : QObject{parent}
    , m_fileListModel(model)
    , m_engine(engine) {
    QStringList args = QCoreApplication::arguments();
    m_directory = QDir::currentPath();

    if (args.count() > 1 && QDir(args[1]).exists()){
        m_directory  = args[1];
    }
}

Q_INVOKABLE void PocketQmlProcessor::initializeFiles() {
    QDir dir(m_directory);
    QStringList filters{"*.bmp", "*.png", "*.barch"};
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    QList<FileItem> filesList;
    for(const auto& file: dir.entryInfoList()){
        FileItem item;
        item.name = file.fileName();
        item.size = file.size();
        item.codingState = CodingState::csNone;
        filesList.append(item);
    }

    m_fileListModel.setFilesList(filesList);
}

void PocketQmlProcessor::handleResult(int idx, int result) {

    const FileItem* item = m_fileListModel.getFileItem(idx);

    if(item == nullptr) {
        return;
    }

    m_fileListModel.updateFileState(idx, CodingState::csNone);
    m_fileListModel.updateResultState(idx, result);

    QMutexLocker locker(&m_mutex);
    if(workers.contains(idx)){
        WorkerStruct& workerStruct = workers[idx];
        QString newFileName = QDir(m_directory).filePath(workerStruct.fileName);
        QFileInfo fileInfo(newFileName);
        if(fileInfo.exists()){
            FileItem fileDetails{workerStruct.fileName, fileInfo.size(),
                                 CodingState::csNone};
            m_fileListModel.addFile(fileDetails);
        }
    }
}

void PocketQmlProcessor::bmpSelect(int idx, const QString& inputFileName) {
    QImage img;
    bool loaded = img.load(inputFileName);
    if(!loaded){
        m_fileListModel.updateResultState(
            idx,
            static_cast<int>(OperationResultState::orsCantLoadInputImage));
        return;
    }

    m_fileListModel.updateFileState(idx, CodingState::csCoding);

    QImage converted = img.convertToFormat(QImage::Format_Grayscale8);

    std::unique_ptr<RawImageData> rawData = std::make_unique<RawImageData>();

    rawData->width = converted.width();
    rawData->height = converted.height();
    rawData->data = std::make_unique<unsigned char[]>(rawData->width * rawData->height);

    for(int row = 0; row < converted.height(); ++row){
        memcpy(rawData->data.get() + row * rawData->width, converted.scanLine(row), rawData->width);
    }

    QFileInfo fi(inputFileName);
    QString shortFileName = fi.fileName() + "packed.barch";
    QString outputFileName = QDir(m_directory).filePath(shortFileName);
    EncodeWorker* encodeWorker = new EncodeWorker(idx, outputFileName, std::move(rawData));

    QThread* thread = new QThread();

    encodeWorker->moveToThread(thread);

    connect(this, &PocketQmlProcessor::cancelProcessing, encodeWorker, &EncodeWorker::cancelProcessing);
    connect(encodeWorker, &EncodeWorker::resultReady, this, &PocketQmlProcessor::handleResult);
    connect(encodeWorker, &EncodeWorker::resultReady, thread, &QThread::quit);
    connect(thread, &QThread::finished, encodeWorker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, idx, thread]() {
        QMutexLocker locker(&m_mutex);
        workers.remove(idx);
        thread->deleteLater();
    });

    connect(thread, &QThread::started, encodeWorker, &EncodeWorker::process);

    {
        QMutexLocker locker(&m_mutex);
        workers.insert(idx, WorkerStruct{encodeWorker, shortFileName});
    }

    thread->start();

    return;
}

OperationResultState PocketQmlProcessor::loadEncodedData(
            const QString& inputFileName,
            std::unique_ptr<EncodedData>& encodedData) const {

    QFileInfo fileInfo(inputFileName);
    if(!fileInfo.exists()){
        return OperationResultState::orsBarchIsNotExists;
    }

    qint64 fileSize = fileInfo.size();
    const qint64 minimalSize = sizeof(encodedData->formatIdentifier) +
                               sizeof(encodedData->width) +
                               sizeof(encodedData->height);
    if(fileSize < minimalSize){
        return OperationResultState::orsBarchHeaderTooSmall;
    }

    std::ifstream file(inputFileName.toStdString(), std::ios::in | std::ios::binary);
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    if(!file.is_open()){
        return OperationResultState::orsCantOpenBarchAsFileStream;
    }

    // header
    file.read(reinterpret_cast<char*>(&encodedData->formatIdentifier), sizeof(encodedData->formatIdentifier));
    file.read(reinterpret_cast<char*>(&encodedData->width), sizeof(encodedData->width));
    file.read(reinterpret_cast<char*>(&encodedData->height), sizeof(encodedData->height));

    // check if there are height * sizeof(height)
    qint64 rowIndexSize = encodedData->height * sizeof(EncodedData::dimension_type);
    if(fileSize < minimalSize + rowIndexSize){
        return OperationResultState::orsBarchRowIndexTooSmall;
    }

    encodedData->rowsIndex = std::make_unique<EncodedData::dimension_type[]>(encodedData->height);
    if(!encodedData->rowsIndex){
        return OperationResultState::orsCantLoadBarchRowIndex;
    }

    file.read(reinterpret_cast<char*>(encodedData->rowsIndex.get()),
              encodedData->height * sizeof(EncodedData::dimension_type));

    encodedData->dataSize = fileSize - (minimalSize + rowIndexSize);
    encodedData->data = std::make_unique<unsigned char[]>(encodedData->dataSize);
    file.read(reinterpret_cast<char*>(encodedData->data.get()), encodedData->dataSize);

    return OperationResultState::csNone;
}

void PocketQmlProcessor::barchSelect(int idx, const QString& inputFileName) {

    std::unique_ptr<EncodedData> encodedData = std::make_unique<EncodedData>();

    OperationResultState loadResult = loadEncodedData(inputFileName, encodedData);

    if(loadResult != OperationResultState::csNone){
        m_fileListModel.updateResultState(
            idx,
            static_cast<int>(loadResult));
        return;
    }

    m_fileListModel.updateFileState(idx, CodingState::csDecoding);

    QFileInfo fi(inputFileName);
    QString shortFileName = fi.fileName() + "unpacked.bmp";
    QString outputFileName = QDir(m_directory).filePath(shortFileName);
    auto decodeWorker = new DecodeWorker(idx, outputFileName, std::move(encodedData));

    QThread* thread = new QThread();

    decodeWorker->moveToThread(thread);

    connect(decodeWorker, &DecodeWorker::resultReady, this, &PocketQmlProcessor::handleResult);
    connect(decodeWorker, &DecodeWorker::resultReady, thread, &QThread::quit);
    connect(thread, &QThread::finished, decodeWorker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, idx, thread]() {
        QMutexLocker locker(&m_mutex);
        workers.remove(idx);
        thread->deleteLater();
    });

    connect(thread, &QThread::started, decodeWorker, &DecodeWorker::process);

    {
        QMutexLocker locker(&m_mutex);
        workers.insert(idx, WorkerStruct{decodeWorker, shortFileName});
    }

    thread->start();       

    return;
}

void PocketQmlProcessor::unknownSelect(const QString& inputFileName) const {
    QQmlComponent component(&m_engine, QUrl(QStringLiteral("qrc:/ErrorDialog.qml")));

    if (component.isError()) {
        qWarning() << "Error loading dialog:" << component.errorString();
        return;
    }

    QObject *dialogObject = component.create();
    if (!dialogObject) {
        qWarning() << "Failed to create the dialog object.";
        return;
    }

    QQuickWindow *dialogWindow = qobject_cast<QQuickWindow*>(dialogObject);
    if (!dialogWindow) {
        qWarning() << "Failed to cast the dialog object to QQuickWindow.";
        delete dialogObject;
        return;
    }

    dialogObject->setProperty("errorMessage", QString("Uknown file %1").arg(inputFileName));
    dialogObject->setProperty("visible", true);
    dialogWindow->setModality(Qt::ApplicationModal);
    dialogWindow->show();
}

Q_INVOKABLE void PocketQmlProcessor::cancelAction(int idx) {
    QMutexLocker locker(&m_mutex);
    if(workers.contains(idx)){
        WorkerStruct& workerStruct = workers[idx];
        if(workerStruct.worker) {
            workerStruct.worker->cancelProcessing();
        }
    }
}

void Q_INVOKABLE PocketQmlProcessor::onItemSelected(int idx) {

    const FileItem* item = m_fileListModel.getFileItem(idx);

    if(item == nullptr) {
        return;
    }

    if(item->codingState != CodingState::csNone) {
        return;
    }

    QString inputFileName  = QDir(m_directory).filePath(item->name);

    try {
        if(item->name.endsWith(".bmp")){
            bmpSelect(idx, inputFileName);
            return;
        }

        if(item->name.endsWith(".barch")){
            barchSelect(idx, inputFileName);
            return;
        }
    }
    catch(...) {
        m_fileListModel.updateFileState(idx, CodingState::csNone);
        m_fileListModel.updateResultState(
            idx,
            static_cast<int>(OperationResultState::orsGeneralFileStreamError));
    }

    unknownSelect(item->name);
}

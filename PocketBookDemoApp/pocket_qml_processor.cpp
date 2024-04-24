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
}

Q_INVOKABLE void PocketQmlProcessor::initializeFiles() {

    QStringList args = QCoreApplication::arguments();
    QString directory = QDir::currentPath();

    if (args.count() > 1 && QDir(args[1]).exists()){
        directory  = args[1];
    }

    QDir dir(directory);
    dir.setFilter(QDir::Files | QDir::NoSymLinks);

    QList<FileItem> filesList;
    for(const auto& file: dir.entryInfoList()){
        FileItem item;
        item.name = QDir(directory).filePath(file.fileName());
        item.size = file.size();
        item.codingState = CodingState::csNone;
        filesList.append(item);
    }

    m_fileListModel.setFilesList(filesList);
}

void PocketQmlProcessor::handleResult(
        const QString& inputFileName,
        int result) {

    m_fileListModel.updateFileState(inputFileName, CodingState::csNone);
    m_fileListModel.updateResultState(inputFileName, result);

    QMutexLocker locker(&m_mutex);
    if(workers.contains(inputFileName)){
        WorkerStruct& workerStruct = workers[inputFileName];
        QFileInfo fileInfo(workerStruct.outputFileName);
        if(fileInfo.exists()){
            FileItem fileDetails{workerStruct.outputFileName, fileInfo.size(),
                                 CodingState::csNone};
            m_fileListModel.addFile(fileDetails);
        }
    }
}

void PocketQmlProcessor::bmpSelect(const QString& inputFileName) {
    QImage img;
    bool loaded = img.load(inputFileName);
    if(!loaded){
        m_fileListModel.updateResultState(
            inputFileName,
            static_cast<int>(OperationResultState::orsCantLoadInputImage));
        return;
    }

    m_fileListModel.updateFileState(inputFileName, CodingState::csCoding);

    QImage converted = img.convertToFormat(QImage::Format_Grayscale8);

    std::unique_ptr<RawImageData> rawData = std::make_unique<RawImageData>();
    rawData->width = converted.width();
    rawData->height = converted.height();
    /*
    rawData->width = 7;
    rawData->height = 6;
    */
    rawData->data = std::make_unique<unsigned char[]>(rawData->width * rawData->height);

    for(int row = 0; row < converted.height(); ++row){
        memcpy(rawData->data.get() + row * rawData->width, converted.scanLine(row), rawData->width);
    }
    /*
    std::vector<unsigned char> testData = {
        255, 255, 255, 255, 137, 24 , 120,
        255, 255, 255, 255, 255, 13 , 7  ,
        255, 255, 255, 255, 255, 255, 3  ,
        0  , 0  , 0  , 0  , 0  , 7  , 8  ,
        0  , 0  , 0  , 4  , 2  , 1  , 3  ,
        0  , 1  , 2  , 3  , 4  , 5  , 6

    };

    memcpy(rawData->data.get(), testData.data(), rawData->width * rawData->height * sizeof(unsigned char));
    */

    QString outputFileName = inputFileName + "packed.barch";
    EncodeWorker* encodeWorker = new EncodeWorker(inputFileName, outputFileName, std::move(rawData));

    QThread* thread = new QThread();

    encodeWorker->moveToThread(thread);

    connect(this, &PocketQmlProcessor::cancelProcessing, encodeWorker, &EncodeWorker::cancelProcessing);
    connect(encodeWorker, &EncodeWorker::resultReady, this, &PocketQmlProcessor::handleResult);
    connect(encodeWorker, &EncodeWorker::resultReady, thread, &QThread::quit);
    connect(thread, &QThread::finished, encodeWorker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, inputFileName, thread]() {
        QMutexLocker locker(&m_mutex);
        workers.remove(inputFileName);
        thread->deleteLater();
    });

    connect(thread, &QThread::started, encodeWorker, &EncodeWorker::process);

    {
        QMutexLocker locker(&m_mutex);
        workers.insert(inputFileName, WorkerStruct{encodeWorker, outputFileName});
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

    encodedData->dataSize = fileSize - minimalSize;
    encodedData->data = std::make_unique<unsigned char[]>(encodedData->dataSize);
    file.read(reinterpret_cast<char*>(encodedData->data.get()), encodedData->dataSize);

    return OperationResultState::csNone;
}

void PocketQmlProcessor::barchSelect(const QString& inputFileName) {

    std::unique_ptr<EncodedData> encodedData = std::make_unique<EncodedData>();

    OperationResultState loadResult = loadEncodedData(inputFileName, encodedData);

    if(loadResult != OperationResultState::csNone){
        m_fileListModel.updateResultState(
            inputFileName,
            static_cast<int>(loadResult));
        return;
    }

    m_fileListModel.updateFileState(inputFileName, CodingState::csDecoding);

    QString outputFileName = inputFileName + "unpacked.bmp";
    auto decodeWorker = new DecodeWorker(inputFileName, outputFileName, std::move(encodedData));

    QThread* thread = new QThread();

    decodeWorker->moveToThread(thread);

    connect(decodeWorker, &DecodeWorker::resultReady, this, &PocketQmlProcessor::handleResult);
    connect(decodeWorker, &DecodeWorker::resultReady, thread, &QThread::quit);
    connect(thread, &QThread::finished, decodeWorker, &QObject::deleteLater);
    connect(thread, &QThread::finished, this, [this, inputFileName, thread]() {
        QMutexLocker locker(&m_mutex);
        workers.remove(inputFileName);
        thread->deleteLater();
    });

    connect(thread, &QThread::started, decodeWorker, &DecodeWorker::process);

    {
        QMutexLocker locker(&m_mutex);
        workers.insert(inputFileName, WorkerStruct{decodeWorker, outputFileName});
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

Q_INVOKABLE void PocketQmlProcessor::cancelAction(const QString& fileName) {
    QMutexLocker locker(&m_mutex);
    if(workers.contains(fileName)){
        WorkerStruct& workerStruct = workers[fileName];
        if(workerStruct.worker) {
            workerStruct.worker->cancelProcessing();
        }
    }
}

void Q_INVOKABLE PocketQmlProcessor::onItemSelected(const QString& fileName) {

    try {
        if(fileName.endsWith(".bmp")){
            bmpSelect(fileName);
            return;
        }

        if(fileName.endsWith(".barch")){
            barchSelect(fileName);
            return;
        }
    }
    catch(...) {
        m_fileListModel.updateFileState(fileName, CodingState::csNone);
        m_fileListModel.updateResultState(
            fileName,
            static_cast<int>(OperationResultState::orsGeneralFileStreamError));
    }

    unknownSelect(fileName);
}

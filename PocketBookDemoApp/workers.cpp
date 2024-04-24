#include "workers.h"

Worker::Worker(const QString& m_inputFileName,
               const QString& outputFileName,
               QObject* parent)
    : QObject(parent)
    , mInputFileName(m_inputFileName)
    , mOutputFileName(outputFileName) {
}

Worker::~Worker() {
}

EncodeWorker::EncodeWorker(const QString& m_inputFileName,
                           const QString& outputFileName,
                           std::unique_ptr<RawImageData> rawData,
                           QObject* parent)
    : Worker(m_inputFileName, outputFileName, parent)
    , mRawData(std::move(rawData)) {}

void EncodeWorker::process() {
    PocketBookCodecLibErrorCode result = mDecoder.encode(
        std::move(mRawData),
        mOutputFileName.toStdString());
    emit resultReady(mInputFileName, static_cast<int>(result));
    deleteLater();
}

DecodeWorker::DecodeWorker(const QString& m_inputFileName,
                           const QString& outputFileName,
                           std::unique_ptr<EncodedData> encodedData,
                           QObject* parent)
    : Worker(m_inputFileName, outputFileName, parent)
    , mEncodedData(std::move(encodedData))  {}

void DecodeWorker::process() {
    PocketBookCodecLibErrorCode result = mDecoder.decode(
        std::move(mEncodedData),
        mOutputFileName.toStdString());
    emit resultReady(mInputFileName, static_cast<int>(result));
    deleteLater();
}

#include "workers.h"

Worker::Worker(int idx, const QString& outputFileName, QObject* parent)
    : QObject(parent)
    , m_idx(idx)
    , m_outputFileName(outputFileName) {
}

Worker::~Worker() {
}

const QString& Worker::getOutputFileName() const {
    return m_outputFileName;
}

EncodeWorker::EncodeWorker(int idx, const QString& outputFileName,
                           std::unique_ptr<RawImageData> rawData,
                           QObject* parent)
    : Worker(idx, outputFileName, parent)
    , m_rawData(std::move(rawData)) {}

void EncodeWorker::process() {
    PocketBookCodecLibErrorCode result = m_decoder.encode(
        std::move(m_rawData),
        m_outputFileName.toStdString());
    emit resultReady(m_idx, static_cast<int>(result));
    deleteLater();
}

DecodeWorker::DecodeWorker(int idx, const QString& outputFileName,
                           std::unique_ptr<EncodedData> encodedData,
                           QObject* parent)
    : Worker(idx, outputFileName, parent)
    , m_encodedData(std::move(encodedData))  {}

void DecodeWorker::process() {
    PocketBookCodecLibErrorCode result = m_decoder.decode(
        std::move(m_encodedData),
        m_outputFileName.toStdString());
    emit resultReady(m_idx, static_cast<int>(result));
    deleteLater();
}

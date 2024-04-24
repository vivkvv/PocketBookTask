#ifndef WORKERS_H
#define WORKERS_H

#include <pocket_book_codec_lib.h>
#include <QObject>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker(const QString& m_inputFileName, const QString& outputFileName,
           QObject* parent = nullptr);
    virtual ~Worker();

signals:
    void resultReady(const QString& inputFileName,
                     int result);

public slots:
    virtual void process() = 0;
    virtual void cancelProcessing() { mDecoder.cancel(); };

protected:
    PocketDecoder mDecoder;
    const QString mInputFileName;
    const QString mOutputFileName;
};


class EncodeWorker : public Worker {
    Q_OBJECT

public:
    EncodeWorker(const QString& m_inputFileName,
                 const QString& outputFileName,
                 std::unique_ptr<RawImageData> rawData,
                 QObject* parent = nullptr);

public slots:
    void process();

protected:
    std::unique_ptr<RawImageData> mRawData;
};

class DecodeWorker : public Worker {
    Q_OBJECT

public:
    DecodeWorker (const QString& m_inputFileName,
                 const QString& outputFileName,
                 std::unique_ptr<EncodedData> encodedData,
                 QObject* parent = nullptr);

public slots:
    void process();

protected:
    std::unique_ptr<EncodedData> mEncodedData;
};

#endif // WORKERS_H

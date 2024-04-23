#ifndef ENCODEWORKER_H
#define ENCODEWORKER_H

#include <pocket_book_codec_lib.h>
#include <QObject>

class Worker : public QObject {
    Q_OBJECT

public:
    Worker(int idx, const QString& outputFileName, QObject* parent = nullptr);
    virtual ~Worker();

    const QString& getOutputFileName() const;

signals:
    void resultReady(int idx, int result);

public slots:
    virtual void process() = 0;
    virtual void cancelProcessing() { m_decoder.cancel(); };

protected:
    PocketDecoder m_decoder;
    int m_idx;
    const QString m_outputFileName;
};


class EncodeWorker : public Worker {
    Q_OBJECT

public:
    EncodeWorker(int idx, const QString& outputFileName,
                 std::unique_ptr<RawImageData> rawData,
                 QObject* parent = nullptr);

public slots:
    void process();

protected:
    std::unique_ptr<RawImageData> m_rawData;
};

class DecodeWorker : public Worker {
    Q_OBJECT

public:
    DecodeWorker (int idx, const QString& outputFileName,
                 std::unique_ptr<EncodedData> encodedData,
                 QObject* parent = nullptr);

public slots:
    void process();

protected:
    std::unique_ptr<EncodedData> m_encodedData;
};


#endif // ENCODEWORKER_H

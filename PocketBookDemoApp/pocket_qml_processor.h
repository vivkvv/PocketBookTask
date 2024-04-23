#ifndef POCKET_QML_PROCESSOR_H
#define POCKET_QML_PROCESSOR_H

#include <QCoreApplication>
#include <QObject>
#include <QDir>
#include <QMutex>

#include "file_list_model.h"
#include "workers.h"
#include <pocket_book_codec_lib.h>

class PocketQmlProcessor final: public QObject
{
    Q_OBJECT
public:
    explicit PocketQmlProcessor(FileListModel& model, QQmlApplicationEngine& engine, QObject *parent = nullptr);

    Q_INVOKABLE void initializeFiles();

    Q_INVOKABLE void onItemSelected(int idx);

    Q_INVOKABLE void cancelAction(int idx);

public slots:
    void handleResult(int idx, int result);

signals:
    void cancelProcessing();

private:
    struct WorkerStruct {
        Worker* worker;
        QString fileName;
    };

    FileListModel& m_fileListModel;
    QString m_directory;
    QQmlApplicationEngine& m_engine;

    mutable QMutex m_mutex;
    QMap<int, WorkerStruct> workers;

    void bmpSelect(int idx, const QString& inputFileName);
    void barchSelect(int idx, const QString& inputFileName);
    void unknownSelect(const QString& inputFileName) const;

    OperationResultState loadEncodedData(
        const QString& inputFileName,
        std::unique_ptr<EncodedData>& encodedData) const;

};

#endif // POCKET_QML_PROCESSOR_H

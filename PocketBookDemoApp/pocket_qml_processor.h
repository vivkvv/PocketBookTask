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

    Q_INVOKABLE void onItemSelected(const QString& fileName);

    Q_INVOKABLE void cancelAction(const QString& fileName);

public slots:
    void handleResult(const QString& inputFileName,
                      int result);

signals:
    void cancelProcessing();

private:
    struct WorkerStruct {
        Worker* worker;
        QString outputFileName;
    };

    FileListModel& mFileListModel;
    QQmlApplicationEngine& mEngine;

    mutable QMutex mMutex;
    QMap<QString, WorkerStruct> mWorkers;

    void bmpSelect(const QString& inputFileName);
    void barchSelect(const QString& inputFileName);
    void unknownSelect(const QString& inputFileName) const;

    OperationResultState loadEncodedData(
        const QString& inputFileName,
        std::unique_ptr<EncodedData>& encodedData) const;
};

#endif // POCKET_QML_PROCESSOR_H

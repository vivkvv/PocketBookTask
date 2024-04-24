#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickView>
#include <QQmlContext>
#include <file_list_filter_proxy_model.h>

#include "pocket_qml_processor.h"
#include "file_list_model.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    qmlRegisterType<PocketQmlProcessor>("com.vovkvv.pocketprocessing", 1, 0, "PocketQmlProcessor");
    qmlRegisterType<FileListFilterProxyModel>("com.vovkvv.pocketprocessing", 1, 0, "FileListFilterProxyModel");
    qmlRegisterUncreatableType<CodingState>("com.vovkvv.types", 1, 0, "CodingStates", "Enum is not creatable");
    qmlRegisterUncreatableType<OperationResultState>("com.vovkvv.types", 1, 0, "OperationResultState", "Enum is not creatable");

    FileListModel model(engine);

    FileListFilterProxyModel filterModel;
    filterModel.setSourceModel(&model);

    PocketQmlProcessor processor(model, engine);

    engine.rootContext()->setContextProperty("processor", &processor);
    engine.rootContext()->setContextProperty("filterModel", &filterModel);

    engine.load(QUrl("qrc:/main.qml"));
    if (engine.rootObjects().isEmpty()){
        return -1;
    }

    return app.exec();
}

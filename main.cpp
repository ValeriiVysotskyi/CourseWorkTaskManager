#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "authorizationController.h"
#include "dbManager.h"
#include "dbSync.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    DbManager dbManager;
    AuthorizationController authController;
    engine.rootContext()->setContextProperty("dbManager", &dbManager);
    engine.rootContext()->setContextProperty("authController", &authController);

    QString serverPath = "http://127.0.0.1:5000/";
    int syncTime = 30000; //це у мс тобто 30 секунд

    DbSync dbSyncManager(serverPath, &dbManager);
    dbSyncManager.startPeriodicSync(syncTime);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("CourseWork", "Main");

    return app.exec();
}

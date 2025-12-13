#ifndef DBSYNC_H
#define DBSYNC_H

#include "dbManager.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QTimer>

class DbSync : public QObject
{
    Q_OBJECT

public:
    explicit DbSync(const QString &serverUrl, DbManager *localDb, QObject *parent = nullptr);

    void startPeriodicSync(const int &interval);

private:
    QTimer m_syncTimer;
    QString m_serverUrl;
    DbManager *m_localDb;
    QNetworkAccessManager manager;

    void startSync();
    void getSyncFromServer();
    void sendSyncToServer();
    void doSubmitSyncRequest(const QString &uuid, const QString &tableName, const int &lastOperation);
    void doRequest(const QString &url, const QNetworkAccessManager::Operation &method,
                   const QJsonObject &taskInfo={}, const int &getTableNum=0);

    void getUnsyncedTasksFinished(const QJsonArray &unsyncedServerTasks);
    void getUnsyncedDepartmentsFinished(const QJsonArray &unsyncedServerDepartments);
    void getUnsyncedTasksStatusesFinished(const QJsonArray &unsyncedServerTasksStatuses);
    void getUnsyncedUsersFinished(const QJsonArray &unsyncedServerUsers);
};

#endif // DBSYNC_H

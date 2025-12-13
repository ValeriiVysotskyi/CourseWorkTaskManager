#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>
#include <QUuid>
#include <QSqlRecord>
#include <QDateTime>
#include <QObject>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariant>

class DbManager:public QObject {
    Q_OBJECT

public:
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager();

    Q_INVOKABLE void addTask(const QMap<QString, QVariant> &taskInfo);
    Q_INVOKABLE void redactTask(const QJsonObject &taskInfo);
    Q_INVOKABLE void deleteTask(const QString &uuid);

    Q_INVOKABLE QMap<QString, QVariant> getAuthorizationInfo(const QString &username);
    Q_INVOKABLE int getDepartmentNumber(const QString &departmentName);
    Q_INVOKABLE void completeTask(const QString &uuid);
    Q_INVOKABLE QJsonObject getTaskInfo(const QString &uuid);
    Q_INVOKABLE QJsonArray getAllTasksList(const int &page);
    Q_INVOKABLE QJsonArray getUserTasksList(const QString &username, const int &page);
    Q_INVOKABLE QJsonArray getDepartmentTasksList(const int &departmentId, const int &page);

    void successfullSyncDeleteTask(const QString &uuid);
    void successfullSyncTask(const QString &uuid);
    bool syncTaskStatuses(const QJsonObject &notSyncedRecord);
    bool syncDepartments(const QJsonObject &notSyncedRecord);
    bool syncUsers(const QJsonObject &notSyncedRecord);
    bool syncTasks(const QJsonObject &notSyncedRecord);

    QString getSyncTime(const QString &tableName, const QString &uuid);
    QJsonArray getNotSyncedData(const QString &tableName);

signals:
    void tasksViewChanged();

private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H

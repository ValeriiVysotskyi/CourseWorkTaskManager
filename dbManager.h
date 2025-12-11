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

    void syncTaskStatuses(const QJsonObject &notSyncedRecord);
    void syncDepartments(const QJsonObject &notSyncedRecord);
    void syncUsers(const QJsonObject &notSyncedRecord);
    void syncTasks(const QJsonObject &notSyncedRecord);

    QString getSyncTime(const QJsonObject &notSyncedRecord);
    QJsonArray getNotSyncedData(const QString &tableName);

signals:
    void tasksViewChanged();

private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H

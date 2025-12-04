#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMap>
#include <QUuid>
#include <QSqlRecord>
#include <QDateTime>

class DbManager {
public:
    DbManager();
    ~DbManager();


    void addUser(const QMap<QString, QVariant> &userInfo);

    void addTask(const QMap<QString, QVariant> &taskInfo);
    void redactTask(const QMap<QString, QVariant> &taskInfo);
    void deleteTask(const QString &uuid);

    void syncTaskStatuses(const QMap<QString, QVariant> &notSyncedRecord);
    void syncDepartments(const QMap<QString, QVariant> &notSyncedRecord);
    void syncUsers(const QMap<QString, QVariant> &notSyncedRecord);
    void syncTasks(const QMap<QString, QVariant> &notSyncedRecord);

    QString getSyncTime(const QMap<QString, QVariant> &notSyncedRecord);
    QMap<QString, QVariant> getAuthorizationInfo(const QString &username);
    QList<QMap<QString, QVariant>> getAllTasksList(const int &page);
    QList<QMap<QString, QVariant>> getUserTasksList(const QString &username, const int &page);
    QList<QMap<QString, QVariant>> getDepartmentTasksList(const int &departmentId, const int &page);
    QList<QMap<QString, QVariant>> getNotSyncedData(const QString &tableName);

private:
    QSqlDatabase db;
};

#endif // DBMANAGER_H

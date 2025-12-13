#include "dbSync.h"

DbSync::DbSync(const QString &serverUrl, DbManager *localDb, QObject *parent)
    : QObject(parent), m_serverUrl(serverUrl), m_localDb(localDb) {

    connect(&m_syncTimer, &QTimer::timeout,this, &DbSync::startSync);
}

void DbSync::startPeriodicSync(const int &interval) {
    if (!m_syncTimer.isActive()){
        m_syncTimer.start(interval);
    }
}

void DbSync::startSync(){
    qDebug() << "[startSync] Розпочато синхронізацію завдань на сервер";
    sendSyncToServer();

    qDebug() << "[startSync] Розпочато синхронізацію записів із сервера на локальну БД";
    getSyncFromServer();
}

void DbSync::sendSyncToServer(){
    QJsonArray unsyncedTasks = m_localDb->getNotSyncedData("tasks");

    for(int i = 0; i < unsyncedTasks.size(); i++){
        QJsonObject taskInfo = unsyncedTasks[i].toObject();
        QString url;
        switch(taskInfo.value("last_operation").toInt()){
        case 0:
            url = m_serverUrl + "/api/tasks/delete/" + taskInfo.value("uuid").toString() + "/" + taskInfo.value("update_time").toString();
            doRequest(url, QNetworkAccessManager::DeleteOperation);
            break;
        case 1:
            url = m_serverUrl + "/api/tasks";
            doRequest(url, QNetworkAccessManager::PostOperation, taskInfo);
            break;
        case 2:
            url = m_serverUrl + "/api/tasks";
            doRequest(url, QNetworkAccessManager::PutOperation, taskInfo);
            break;
        }
    }
}

void DbSync::getSyncFromServer(){
    QString url;

    url = m_serverUrl + "/api/departments/not_synced_records";
    doRequest(url, QNetworkAccessManager::GetOperation, {}, 1);

    url = m_serverUrl + "/api/tasks_statuses/not_synced_records";
    doRequest(url, QNetworkAccessManager::GetOperation, {}, 2);

    url = m_serverUrl + "/api/users/not_synced_records";
    doRequest(url, QNetworkAccessManager::GetOperation, {}, 3);

    url = m_serverUrl + "/api/tasks/not_synced_records";
    doRequest(url, QNetworkAccessManager::GetOperation, {}, 4);
}

void DbSync::doRequest(const QString &url, const QNetworkAccessManager::Operation &method, const QJsonObject &taskInfo, const int &getTableNum){
    QNetworkRequest request(url);

    switch (method){
        case QNetworkAccessManager::PostOperation:{
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QJsonDocument doc(taskInfo);
            QByteArray data = doc.toJson();
            QNetworkReply* reply = manager.post(request, data);
            connect(reply, &QNetworkReply::finished, this, [this, reply]{
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "[API doRequest PostOperation] помилка при запиті:" << reply->errorString();
                    reply->deleteLater();
                    return;
                }

                const QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonObject obj = doc.object();
                QString uuid = obj.value("uuid").toString();
                qDebug() << "[API doRequest PostOperation] успішна синхронізація на сервері UUID:" << uuid;
                m_localDb->successfullSyncTask(uuid);
                reply->deleteLater();
            });
            break;
        }

        case QNetworkAccessManager::PutOperation:{
            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
            QJsonDocument doc(taskInfo);
            QByteArray data = doc.toJson();
            QNetworkReply* reply = manager.put(request, data);
            connect(reply, &QNetworkReply::finished, this, [this, reply](){
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "[API doRequest PutOperation] помилка при запиті:" << reply->errorString();
                    reply->deleteLater();
                    return;
                }

                const QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonObject obj = doc.object();
                QString uuid = obj.value("uuid").toString();
                qDebug() << "[API doRequest PutOperation] успішна синхронізація на сервері UUID:" << uuid;
                m_localDb->successfullSyncTask(uuid);
                reply->deleteLater();
            });
            break;
        }

        case QNetworkAccessManager::DeleteOperation:{
            QNetworkReply* reply = manager.deleteResource(request);
            connect(reply, &QNetworkReply::finished, this, [this, reply](){
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "[API doRequest DeleteOperation] помилка при запиті:" << reply->errorString();
                    reply->deleteLater();
                    return;
                }

                const QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonObject obj = doc.object();
                QString uuid = obj.value("uuid").toString();
                qDebug() << "[API doRequest DeleteOperation] успішна синхронізація на сервері UUID:" << uuid;
                m_localDb->successfullSyncDeleteTask(uuid);
                reply->deleteLater();
            });
            break;
        }

        case QNetworkAccessManager::GetOperation:{
            QNetworkReply* reply = manager.get(request);
            connect(reply, &QNetworkReply::finished, this, [this, reply, getTableNum](){
                if (reply->error() != QNetworkReply::NoError) {
                    qDebug() << "[API doRequest GetOperation] помилка при запиті:" << reply->errorString();
                    reply->deleteLater();
                    return;
                }

                const QByteArray responseData = reply->readAll();
                QJsonDocument doc = QJsonDocument::fromJson(responseData);
                QJsonArray unsyncedServerData = doc.array();
                qDebug() << "[API doRequest GetOperation] дані для синхронізації отримано";
                qDebug() << "[API doRequest GetOperation] кількість записів:" << unsyncedServerData.size();

                switch (getTableNum) {
                case 1:
                    getUnsyncedDepartmentsFinished(unsyncedServerData);
                    break;

                case 2:
                    getUnsyncedTasksStatusesFinished(unsyncedServerData);
                    break;

                case 3:
                    getUnsyncedUsersFinished(unsyncedServerData);
                    break;

                case 4:
                    getUnsyncedTasksFinished(unsyncedServerData);
                    break;

                default:
                    qDebug() << "[API doRequest GetOperation] номер таблиці невідомий";
                    break;
                }
                reply->deleteLater();
            });
            break;
        }

        default:
            qDebug() << "[API doRequest] Дана API операція не підтримується або її не існує:" << method;
            break;
    }
}

void DbSync::getUnsyncedDepartmentsFinished(const QJsonArray &unsyncedServerDepartments){
    for(int i = 0; i < unsyncedServerDepartments.size(); i++){
        QJsonObject departmentInfo = unsyncedServerDepartments[i].toObject();

        if (departmentInfo.value("last_operation").toInt() == 1){
            if (m_localDb->syncDepartments(departmentInfo)){
                doSubmitSyncRequest(departmentInfo.value("uuid").toString(), "departments", departmentInfo.value("last_operation").toInt());
            }
        }
        else {
            QString localDbSyncTime = m_localDb->getSyncTime("departments", departmentInfo.value("uuid").toString());
            if (departmentInfo.value("update_time").toString() >= localDbSyncTime){
                if (m_localDb->syncDepartments(departmentInfo)){
                    doSubmitSyncRequest(departmentInfo.value("uuid").toString(), "departments", departmentInfo.value("last_operation").toInt());
                }
            }
        }
    }
}

void DbSync::getUnsyncedTasksFinished(const QJsonArray &unsyncedServerTasks){
    for(int i = 0; i < unsyncedServerTasks.size(); i++){
        QJsonObject taskInfo = unsyncedServerTasks[i].toObject();

        if (taskInfo.value("last_operation").toInt() == 1){
            if (m_localDb->syncTasks(taskInfo)){
                doSubmitSyncRequest(taskInfo.value("uuid").toString(), "tasks", taskInfo.value("last_operation").toInt());
            }
        }
        else {
            QString localDbSyncTime = m_localDb->getSyncTime("tasks", taskInfo.value("uuid").toString());
            if (taskInfo.value("update_time").toString() >= localDbSyncTime){
                if (m_localDb->syncTasks(taskInfo)){
                    doSubmitSyncRequest(taskInfo.value("uuid").toString(), "tasks", taskInfo.value("last_operation").toInt());
                }
            }
        }
    }
}

void DbSync::getUnsyncedUsersFinished(const QJsonArray &unsyncedServerUsers){
    for(int i = 0; i < unsyncedServerUsers.size(); i++){
        QJsonObject userInfo = unsyncedServerUsers[i].toObject();

        if (userInfo.value("last_operation").toInt() == 1){
            if (m_localDb->syncUsers(userInfo)){
                doSubmitSyncRequest(userInfo.value("uuid").toString(), "users", userInfo.value("last_operation").toInt());
            }
        }
        else {
            QString localDbSyncTime = m_localDb->getSyncTime("users", userInfo.value("uuid").toString());
            if (userInfo.value("update_time").toString() >= localDbSyncTime){
                if (m_localDb->syncUsers(userInfo)){
                    doSubmitSyncRequest(userInfo.value("uuid").toString(), "users", userInfo.value("last_operation").toInt());
                }
            }
        }
    }
}

void DbSync::getUnsyncedTasksStatusesFinished(const QJsonArray &unsyncedServerTasksStatuses){
    for(int i = 0; i < unsyncedServerTasksStatuses.size(); i++){
        QJsonObject taskStatusInfo = unsyncedServerTasksStatuses[i].toObject();

        if (taskStatusInfo.value("last_operation").toInt() == 1){
            if (m_localDb->syncTaskStatuses(taskStatusInfo)){
                doSubmitSyncRequest(taskStatusInfo.value("uuid").toString(), "tasks_statuses", taskStatusInfo.value("last_operation").toInt());
            }
        }
        else {
            QString localDbSyncTime = m_localDb->getSyncTime("tasks_statuses", taskStatusInfo.value("uuid").toString());
            if (taskStatusInfo.value("update_time").toString() >= localDbSyncTime){
                if (m_localDb->syncTaskStatuses(taskStatusInfo)){
                    doSubmitSyncRequest(taskStatusInfo.value("uuid").toString(), "tasks_statuses", taskStatusInfo.value("last_operation").toInt());
                }
            }
        }
    }
}

void DbSync::doSubmitSyncRequest(const QString &uuid, const QString &tableName, const int &lastOperation){
    QString url;
    if (lastOperation){
        url = m_serverUrl + "/api/successfullSync/" + tableName + "/" + uuid;
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply* reply = manager.put(request, QByteArray());

        connect(reply, &QNetworkReply::finished, this, [reply]{
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "[API doSubmitSyncRequest PutOperation] помилка при запиті:" << reply->errorString();
                reply->deleteLater();
                return;
            }
            qDebug() << "[API doSubmitSyncRequest PutOperation] успішна зміна статусу синхронізації запису";
            reply->deleteLater();
        });
    }
    else{
        url = m_serverUrl + "/api/successfullSyncDelete/" + tableName + "/" + uuid;
        QNetworkRequest request(url);
        QNetworkReply* reply = manager.deleteResource(request);

        connect(reply, &QNetworkReply::finished, this, [reply]{
            if (reply->error() != QNetworkReply::NoError) {
                qDebug() << "[API doSubmitSyncRequest PutOperation] помилка при запиті:" << reply->errorString();
                reply->deleteLater();
                return;
            }
            qDebug() << "[API doSubmitSyncRequest PutOperation] успішне видалення запису на сервері після локального видалення";
            reply->deleteLater();
        });
    }
}

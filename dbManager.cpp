#include "dbManager.h"
#include "passwordHashing.h"

DbManager::DbManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("taskManagerDb.db");

    if (!db.open()) {
        qDebug() << "Помилка відкриття БД:" << db.lastError().text();
    }
    else {
        qDebug() << "БД завантаженно та готово до роботи";
        // QSqlQuery query;
        // query.exec("PRAGMA foreign_keys = ON");
    }
}

DbManager::~DbManager(){
    db.close();
}

void DbManager::addUser(const QMap<QString, QVariant> &userInfo) {
    QMap<QString, QString> passwordInfo = hashPassword(userInfo["password"].toString());
    QSqlQuery query;

    query.prepare("INSERT INTO users (uuid, last_operation, username, password_hash, "
                  "password_salt, name, surname, fk_department_id) VALUES"
                  "(?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(QUuid::createUuid().toString(QUuid::WithoutBraces));
    query.addBindValue(1);
    query.addBindValue(userInfo["username"]);
    query.addBindValue(passwordInfo["password_hash"]);
    query.addBindValue(passwordInfo["salt"]);
    query.addBindValue(userInfo["name"]);
    query.addBindValue(userInfo["surname"]);
    query.addBindValue(userInfo["fk_department_id"]);

    if(!query.exec()) {
        qDebug() << "Помилка створення користувача:" << query.lastError().text();
    }
    else {
        qDebug() << "Створення користувача " << userInfo["username"].toString() << " виконано успішно";
    }
}

void DbManager::addTask(const QMap<QString, QVariant> &taskInfo){
    QSqlQuery query;
    QString taskUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare("INSERT INTO tasks (uuid, last_operation, assigner, assignee, "
                  "due_date, fk_task_status_id, tittle, description, fk_department_id) VALUES("
                  "?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(taskUuid);
    query.addBindValue(1);
    query.addBindValue(taskInfo["assigner"]);
    query.addBindValue(taskInfo["assignee"]);
    query.addBindValue(taskInfo["due_date"]);
    query.addBindValue(taskInfo["fk_task_status_id"]);
    query.addBindValue(taskInfo["tittle"]);
    query.addBindValue(taskInfo["description"]);
    query.addBindValue(taskInfo["fk_department_id"]);

    if(!query.exec()) {
        qDebug() << "Помилка створення задачі:" << query.lastError().text();
    }
    else {
        qDebug() << "Задачу " << taskUuid << " успішно створено";
    }
}

void DbManager::redactTask(const QMap<QString, QVariant> &taskInfo){
    QSqlQuery query;
    query.prepare("UPDATE tasks SET sync_status=?, update_time=?, last_operation=?, assigner=?, "
                  "assignee=?, due_date=?, fk_task_status_id=?, tittle=?, "
                  "description=?, fk_department_id=? WHERE uuid =?");

    query.addBindValue(false);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(2);
    query.addBindValue(taskInfo["assigner"]);
    query.addBindValue(taskInfo["assignee"]);
    query.addBindValue(taskInfo["due_date"]);
    query.addBindValue(taskInfo["fk_task_status_id"]);
    query.addBindValue(taskInfo["tittle"]);
    query.addBindValue(taskInfo["description"]);
    query.addBindValue(taskInfo["fk_department_id"]);
    query.addBindValue(taskInfo["uuid"]);

    if(!query.exec()) {
        qDebug() << "Помилка редагування задачі. Task uuid:" << taskInfo["uuid"] << query.lastError().text();
    }
    else {
        qDebug() << "Задачу " << taskInfo["uuid"] << " успішно відредаговано";
    }
}

void DbManager::deleteTask(const QString &uuid){
    QSqlQuery query;

    query.prepare("UPDATE tasks SET last_operation=?, update_time=?, sync_status=? WHERE uuid = ?");
    query.addBindValue(0);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(false);
    query.addBindValue(uuid);

    if(!query.exec()) {
        qDebug() << "Помилка зміни статусу задачі на видалення. task uuid:" << uuid << query.lastError().text();
    }
    else {
        qDebug() << "Статус задачі " << uuid << " успішно змінено на видалений";
    }
};

void DbManager::syncTaskStatuses(const QMap<QString, QVariant> &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord["last_operation"].toInt()) {
    case 0:
        query.prepare("DELETE FROM tasks_statuses WHERE uuid = ?");
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()){
            qDebug() << "Помилка видалення типу статусу задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "Тип статусу задачі " << notSyncedRecord["name"].toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO tasks_statuses VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord["uuid"]);
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["task_status_id"]);
        query.addBindValue(notSyncedRecord["name"]);

        if(!query.exec()) {
            qDebug() << "Помилка додавання нового статусу для задач:" << query.lastError().text();
        }

        else {
            qDebug() << "Додано новий тип статусу для задач:" << notSyncedRecord["name"].toString();
        }
        break;

    case 2:
        query.prepare("UPDATE tasks_statuses SET "
                      "sync_status=?, update_time=?, "
                      "last_operation = ?, name=? WHERE uuid = ?");

        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["name"]);
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()) {
            qDebug() << "Помилка оновлення типу статусу задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "Інформацію про тип статусу задачі оновлено uuid:" << notSyncedRecord["uuid"].toString();
        }
        break;

    default:
        qDebug() << "Невідома операція";
        break;
    }
}

void DbManager::syncDepartments(const QMap<QString, QVariant> &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord["last_operation"].toInt()) {
    case 0:
        query.prepare("DELETE FROM departments WHERE uuid = ?");
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()){
            qDebug() << "Помилка видалення департаменту:" << query.lastError().text();
        }
        else {
            qDebug() << "Департамент " << notSyncedRecord["name"].toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO departments VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord["uuid"]);
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["department_id"]);
        query.addBindValue(notSyncedRecord["name"]);
        query.addBindValue(notSyncedRecord["manager"]);

        if(!query.exec()) {
            qDebug() << "Помилка додавання департаменту:" << query.lastError().text();
        }
        else {
            qDebug() << "Додано новий департамент:" << notSyncedRecord["name"].toString();
        }
        break;

    case 2:
        query.prepare("UPDATE departments SET "
                      "sync_status=?, update_time=?, last_operation=?,"
                      "manager=?, name=? WHERE uuid=?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["manager"]);
        query.addBindValue(notSyncedRecord["name"]);
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()) {
            qDebug() << "Помилка оновлення типу департамента:" << query.lastError().text();
        }
        else {
            qDebug() << "Інформацію про департамент оновлено. uuid:" << notSyncedRecord["uuid"].toString();
        }
        break;

    default:
        qDebug() << "Невідома операція";
        break;
    }
}

void DbManager::syncUsers(const QMap<QString, QVariant> &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord["last_operation"].toInt()) {
    case 0:
        query.prepare("DELETE FROM users WHERE uuid = ?");
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()){
            qDebug() << "Помилка видалення користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "Користувача " << notSyncedRecord["username"].toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO users VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord["uuid"]);
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["username"]);
        query.addBindValue(notSyncedRecord["password_hash"]);
        query.addBindValue(notSyncedRecord["name"]);
        query.addBindValue(notSyncedRecord["surname"]);
        query.addBindValue(notSyncedRecord["fk_department_id"]);
        query.addBindValue(notSyncedRecord["password_salt"]);

        if(!query.exec()) {
            qDebug() << "Помилка додавання користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "Додано нового користувача:" << notSyncedRecord["username"].toString();
        }
        break;

    case 2:
        query.prepare("UPDATE users SET "
                      "sync_status=?, update_time=?, last_operation=?, "
                      "password_hash=?, name=?, surname=?, "
                      "fk_department_id=?, password_salt=? WHERE uuid = ?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["password_hash"]);
        query.addBindValue(notSyncedRecord["name"]);
        query.addBindValue(notSyncedRecord["surname"]);
        query.addBindValue(notSyncedRecord["fk_department_id"]);
        query.addBindValue(notSyncedRecord["password_salt"]);
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()) {
            qDebug() << "Помилка оновлення інформації про користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "Інформацію про користувача " << notSyncedRecord["username"].toString() << " оновлено";
        }
        break;

    default:
        qDebug() << "Невідома операція";
        break;
    }
}

void DbManager::syncTasks(const QMap<QString, QVariant> &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord["last_operation"].toInt()) {
    case 0:
        query.prepare("DELETE FROM tasks WHERE uuid = ?");
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()){
            qDebug() << "Помилка видалення задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "Задачу " << notSyncedRecord["uuid"].toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO tasks VALUES (?, ?, ?, ?, ?, ?, "
                      "?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord["uuid"]);
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["assigner"]);
        query.addBindValue(notSyncedRecord["assignee"]);
        query.addBindValue(notSyncedRecord["create_date"]);
        query.addBindValue(notSyncedRecord["due_date"]);
        query.addBindValue(notSyncedRecord["fk_task_status_id"]);
        query.addBindValue(notSyncedRecord["tittle"]);
        query.addBindValue(notSyncedRecord["description"]);
        query.addBindValue(notSyncedRecord["fk_department_id"]);

        if(!query.exec()) {
            qDebug() << "Помилка додавання задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "Додано нову задачу:" << notSyncedRecord["uuid"].toString();
        }
        break;

    case 2:
        query.prepare("UPDATE tasks SET "
                      "sync_status=?, update_time=?, last_operation=?, "
                      "assignee=?, due_date=?, fk_task_status_id=?, "
                      "fk_department_id=?, tittle=?, description=? WHERE uuid = ?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord["update_time"]);
        query.addBindValue(notSyncedRecord["last_operation"]);
        query.addBindValue(notSyncedRecord["assignee"]);
        query.addBindValue(notSyncedRecord["due_date"]);
        query.addBindValue(notSyncedRecord["fk_task_status_id"]);
        query.addBindValue(notSyncedRecord["fk_department_id"]);
        query.addBindValue(notSyncedRecord["tittle"]);
        query.addBindValue(notSyncedRecord["description"]);
        query.addBindValue(notSyncedRecord["uuid"]);

        if(!query.exec()) {
            qDebug() << "Помилка оновлення даних у задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "Дані задачі " << notSyncedRecord["uuid"].toString() << " оновлено";
        }
        break;

    default:
        qDebug() << "Невідома операція";
        break;
    }
}

QString DbManager::getSyncTime(const QMap<QString, QVariant> &notSyncedRecord) {
    QSqlQuery query;
    QString syncTimeResult;
    QString sqlSyncDateReq = QString("SELECT update_time FROM %1 WHERE uuid = ?").arg(notSyncedRecord["table_name"].toString());

    query.prepare(sqlSyncDateReq);
    query.addBindValue(notSyncedRecord["uuid"].toString());

    if (!query.exec()) {
        qDebug() << "[getSyncTime] Помилка виконання запиту:" << query.lastError().text();
    }
    else if (query.next()) {
        syncTimeResult = query.value("update_time").toString();
        qDebug() << "[getSyncTime] Успішно отримано час:" << syncTimeResult;
    }
    else {
        qDebug() << "[getSyncTime] Запис з UUID:" << notSyncedRecord["uuid"].toString()
                 << " не знайдено в таблиці" << notSyncedRecord["table_name"].toString();
    }

    return syncTimeResult;
}

QMap<QString, QVariant> DbManager::getAuthorizationInfo(const QString &username){
    QMap<QString, QVariant> result;
    QSqlQuery query;

    query.prepare("SELECT password_hash, password_salt FROM users WHERE username=? AND last_operation<>0");
    query.addBindValue(username);

    if (!query.exec()) {
        qDebug() << "[getAuthorizationInfo] видав помилку:" << query.lastError().text();
    } else if (query.next()){
        qDebug() << "[getAuthorizationInfo] відпрацював успішно";
        result["password_hash"] = query.value("password_hash");
        result["password_salt"] = query.value("password_salt");
    }
    else{
        qDebug() << "[getAuthorizationInfo] отримав пустий запит";
    }

    return result;
}

QList<QMap<QString, QVariant>> DbManager::getAllTasksList(const int &page){
    QList<QMap<QString, QVariant>> result;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "tittle, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id = 0 AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getAllTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            QMap<QString, QVariant> taskInfo;
            taskInfo["uuid"] = query.value("uuid");
            taskInfo["assigner"] = query.value("assigner");
            taskInfo["assignee"] = query.value("assignee");
            taskInfo["create_date"] = query.value("create_date");
            taskInfo["due_date"] = query.value("due_date");
            taskInfo["fk_task_status_id"] = query.value("fk_task_status_id");
            taskInfo["tittle"] = query.value("tittle");
            taskInfo["description"] = query.value("description");
            taskInfo["fk_department_id"] = query.value("fk_department_id");
            result.push_back(taskInfo);
        }

        qDebug() << "[getAllTasksList] відпрацював успішно";
    }

    return result;
}

QList<QMap<QString, QVariant>> DbManager::getUserTasksList(const QString &username, const int &page){
    QList<QMap<QString, QVariant>> result;
    QMap<QString, QVariant> taskInfo;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "tittle, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id=0 AND assignee=? AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue(username);
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getUserTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            taskInfo["uuid"] = query.value("uuid");
            taskInfo["assigner"] = query.value("assigner");
            taskInfo["assignee"] = query.value("assignee");
            taskInfo["create_date"] = query.value("create_date");
            taskInfo["due_date"] = query.value("due_date");
            taskInfo["fk_task_status_id"] = query.value("fk_task_status_id");
            taskInfo["tittle"] = query.value("tittle");
            taskInfo["description"] = query.value("description");
            taskInfo["fk_department_id"] = query.value("fk_department_id");
            result.push_back(taskInfo);
        }

        qDebug() << "[getUserTasksList] відпрацював успішно";
    }

    return result;
}

QList<QMap<QString, QVariant>> DbManager::getDepartmentTasksList(const int &departmentId, const int &page){
    QList<QMap<QString, QVariant>> result;
    QMap<QString, QVariant> taskInfo;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "tittle, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id = 0 AND fk_department_id=? AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue(departmentId);
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getDepartmentTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            taskInfo["uuid"] = query.value("uuid");
            taskInfo["assigner"] = query.value("assigner");
            taskInfo["assignee"] = query.value("assignee");
            taskInfo["create_date"] = query.value("create_date");
            taskInfo["due_date"] = query.value("due_date");
            taskInfo["fk_task_status_id"] = query.value("fk_task_status_id");
            taskInfo["tittle"] = query.value("tittle");
            taskInfo["description"] = query.value("description");
            taskInfo["fk_department_id"] = query.value("fk_department_id");
            result.push_back(taskInfo);
        }

        qDebug() << "[getDepartmentTasksList] відпрацював успішно";
    }

    return result;
}

QList<QMap<QString, QVariant>> DbManager::getNotSyncedData(const QString &tableName) {
    QList<QMap<QString, QVariant>> result;
    QString sqlNotSyncedReq = QString("SELECT * FROM %1 WHERE sync_status = false").arg(tableName);
    QSqlQuery query;

    if (!query.exec(sqlNotSyncedReq)) {
        qDebug() << "[getNotSyncedData] видав помилку:" << query.lastError().text();
    }
    else {
        QSqlRecord record = query.record();

        while (query.next()) {
            QMap<QString, QVariant> notSyncedRecord;

            for (int i = 0; i < record.count(); i++) {
                notSyncedRecord[record.fieldName(i)] = query.value(i);
            }
            result.append(notSyncedRecord);
        }

         qDebug() << "[getNotSyncedData] відпрацював успішно";
    }

    return result;
}

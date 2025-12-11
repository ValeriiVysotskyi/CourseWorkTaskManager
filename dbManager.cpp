#include "dbManager.h"
#include "AuthorizationController.h"

DbManager::DbManager(QObject *parent): QObject(parent) {
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
    if (db.isOpen()) db.close();
}

void DbManager::addTask(const QMap<QString, QVariant> &taskInfo){
    QSqlQuery query;
    QString taskUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);

    query.prepare("INSERT INTO tasks (uuid, last_operation, assigner, assignee, "
                  "due_date, fk_task_status_id, title, description, fk_department_id) VALUES("
                  "?, ?, ?, ?, ?, ?, ?, ?, ?)");

    query.addBindValue(taskUuid);
    query.addBindValue(1);
    query.addBindValue(taskInfo["assigner"]);
    query.addBindValue(taskInfo["assignee"]);
    query.addBindValue(taskInfo["due_date"]);
    query.addBindValue(0);
    query.addBindValue(taskInfo["title"]);
    query.addBindValue(taskInfo["description"]);
    query.addBindValue(taskInfo["fk_department_id"]);

    if(!query.exec()) {
        qDebug() << "[addTask] Помилка створення задачі:" << query.lastError().text();
    }
    else {
        qDebug() << "[addTask] Задачу " << taskUuid << " успішно створено";
    }
}

QJsonObject DbManager::getTaskInfo(const QString &uuid){
    QSqlQuery query;
    query.prepare("SELECT assigner, assignee, due_date, "
                  "title, description, department_id, name FROM tasks "
                  "INNER JOIN departments ON fk_department_id = department_id "
                  "WHERE tasks.uuid =?");
    query.addBindValue(uuid);
    query.exec();
    if (query.next()) {
        QJsonObject result;
        result["assigner"] = query.value("assigner").toString();
        result["assignee"] = query.value("assignee").toString();
        result["due_date"] = query.value("due_date").toString();
        result["title"] = query.value("title").toString();
        result["description"] = query.value("description").toString();
        result["department_id"] = query.value("department_id").toString();
        result["department_name"] = query.value("name").toString();

        qDebug() << "[getTaskInfo] успішно отримали інформацію завдання. uuid:" << uuid;
        return result;
    }
    else {
        qDebug() << "[getTaskInfo] завдання "<< uuid << " не існує";
        return {};
    }
}

void DbManager::completeTask(const QString &uuid){
    QSqlQuery query;
    query.prepare("UPDATE tasks SET sync_status=?, update_time=?, fk_task_status_id=? WHERE uuid =?");
    query.addBindValue(false);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(1);
    query.addBindValue(uuid);

    if(!query.exec()) {
        qDebug() << "[completeTask] Помилка завершення задачі. Task uuid:" << uuid << query.lastError().text();
    }
    else {
        qDebug() << "[completeTask] Задачу " << uuid << " завершено";
        emit tasksViewChanged();
    }
}

void DbManager::redactTask(const QJsonObject &taskInfo){
    QSqlQuery query;
    query.prepare("UPDATE tasks SET sync_status=?, update_time=?, last_operation=?, assigner=?, "
                  "assignee=?, due_date=?, title=?, "
                  "description=?, fk_department_id=? WHERE uuid =?");

    query.addBindValue(false);
    query.addBindValue(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd hh:mm:ss"));
    query.addBindValue(2);
    query.addBindValue(taskInfo.value("assigner").toVariant());
    query.addBindValue(taskInfo.value("assignee").toVariant());
    query.addBindValue(taskInfo.value("due_date").toVariant());
    query.addBindValue(taskInfo.value("title").toVariant());
    query.addBindValue(taskInfo.value("description").toVariant());
    query.addBindValue(taskInfo.value("fk_department_id").toVariant());
    query.addBindValue(taskInfo.value("uuid").toVariant());

    if(!query.exec()) {
        qDebug() << "[redactTask] Помилка редагування задачі. Task uuid:" << taskInfo.value("uuid") << query.lastError().text();
    }
    else {
        qDebug() << "[redactTask] Задачу " << taskInfo.value("uuid") << " успішно відредаговано";
        emit tasksViewChanged();
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
        qDebug() << "[deleteTask] Помилка зміни статусу задачі на видалення. task uuid:" << uuid << query.lastError().text();
    }
    else {
        qDebug() << "[deleteTask] Статус задачі " << uuid << " успішно змінено на видалений";
    }
}

void DbManager::syncTaskStatuses(const QJsonObject &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord.value("last_operation").toInt()) {
    case 0:
        query.prepare("DELETE FROM tasks_statuses WHERE uuid = ?");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()){
            qDebug() << "[syncTaskStatuses] Помилка видалення типу статусу задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncTaskStatuses] Тип статусу задачі " << notSyncedRecord.value("name").toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO tasks_statuses VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("task_status_id").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncTaskStatuses] Помилка додавання нового статусу для задач:" << query.lastError().text();
        }

        else {
            qDebug() << "[syncTaskStatuses] Додано новий тип статусу для задач:" << notSyncedRecord.value("name").toString();
        }
        break;

    case 2:
        query.prepare("UPDATE tasks_statuses SET "
                      "sync_status=?, update_time=?, task_status_id=?, "
                      "last_operation = ?, name=? WHERE uuid = ?");

        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("task_status_id").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncTaskStatuses] Помилка оновлення типу статусу задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncTaskStatuses] Інформацію про тип статусу задачі оновлено uuid:" << notSyncedRecord["uuid"].toString();
        }
        break;

    default:
        qDebug() << "[syncTaskStatuses] Невідома операція";
        break;
    }
}

void DbManager::syncDepartments(const QJsonObject &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord.value("last_operation").toInt()) {
    case 0:
        query.prepare("DELETE FROM departments WHERE uuid = ?");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()){
            qDebug() << "[syncDepartments] Помилка видалення департаменту:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncDepartments] Департамент " << notSyncedRecord.value("name").toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO departments VALUES (?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("department_id").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());
        query.addBindValue(notSyncedRecord.value("manager").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncDepartments] Помилка додавання департаменту:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncDepartments] Додано новий департамент:" << notSyncedRecord.value("name").toString();
        }
        break;

    case 2:
        query.prepare("UPDATE departments SET "
                      "sync_status=?, update_time=?, last_operation=?,"
                      "department_id=?, manager=?, name=? WHERE uuid=?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("department_id").toVariant());
        query.addBindValue(notSyncedRecord.value("manager").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncDepartments] Помилка оновлення типу департамента:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncDepartments] Інформацію про департамент оновлено. uuid:" << notSyncedRecord["uuid"].toString();
        }
        break;

    default:
        qDebug() << "[syncDepartments] Невідома операція";
        break;
    }
}

void DbManager::syncUsers(const QJsonObject &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord.value("last_operation").toInt()) {
    case 0:
        query.prepare("DELETE FROM users WHERE uuid = ?");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()){
            qDebug() << "[syncUsers] Помилка видалення користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncUsers] Користувача " << notSyncedRecord.value("username").toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO users VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("username").toVariant());
        query.addBindValue(notSyncedRecord.value("password_hash").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());
        query.addBindValue(notSyncedRecord.value("surname").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_department_id").toVariant());
        query.addBindValue(notSyncedRecord.value("password_salt").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncUsers] Помилка додавання користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncUsers] Додано нового користувача:" << notSyncedRecord.value("username").toString();
        }
        break;

    case 2:
        query.prepare("UPDATE users SET "
                      "sync_status=?, update_time=?, last_operation=?, "
                      "username=?, password_hash=?, name=?, surname=?, "
                      "fk_department_id=?, password_salt=? WHERE uuid = ?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("username").toVariant());
        query.addBindValue(notSyncedRecord.value("password_hash").toVariant());
        query.addBindValue(notSyncedRecord.value("name").toVariant());
        query.addBindValue(notSyncedRecord.value("surname").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_department_id").toVariant());
        query.addBindValue(notSyncedRecord.value("password_salt").toVariant());
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncUsers] Помилка оновлення інформації про користувача:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncUsers] Інформацію про користувача " << notSyncedRecord.value("username").toString() << " оновлено";
        }
        break;

    default:
        qDebug() << "[syncUsers] Невідома операція";
        break;
    }
}

void DbManager::syncTasks(const QJsonObject &notSyncedRecord){
    QSqlQuery query;

    switch (notSyncedRecord.value("last_operation").toInt()) {
    case 0:
        query.prepare("DELETE FROM tasks WHERE uuid = ?");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()){
            qDebug() << "[syncTasks] Помилка видалення задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncTasks] Задачу " << notSyncedRecord.value("uuid").toString() << " було успішно видалено";
        }
        break;

    case 1:
        query.prepare("INSERT INTO tasks VALUES (?, ?, ?, ?, ?, ?, "
                      "?, ?, ?, ?, ?, ?)");
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("assigner").toVariant());
        query.addBindValue(notSyncedRecord.value("assignee").toVariant());
        query.addBindValue(notSyncedRecord.value("create_date").toVariant());
        query.addBindValue(notSyncedRecord.value("due_date").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_task_status_id").toVariant());
        query.addBindValue(notSyncedRecord.value("title").toVariant());
        query.addBindValue(notSyncedRecord.value("description").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_department_id").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncTasks] Помилка додавання задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncTasks] Додано нову задачу:" << notSyncedRecord.value("uuid").toString();
        }
        break;

    case 2:
        query.prepare("UPDATE tasks SET "
                      "sync_status=?, update_time=?, last_operation=?, "
                      "assigner=?, assignee=?, create_date=?, due_date=?, fk_task_status_id=?, "
                      "fk_department_id=?, title=?, description=? WHERE uuid = ?");
        query.addBindValue(1);
        query.addBindValue(notSyncedRecord.value("update_time").toVariant());
        query.addBindValue(notSyncedRecord.value("last_operation").toVariant());
        query.addBindValue(notSyncedRecord.value("assigner").toVariant());
        query.addBindValue(notSyncedRecord.value("assignee").toVariant());
        query.addBindValue(notSyncedRecord.value("create_date").toVariant());
        query.addBindValue(notSyncedRecord.value("due_date").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_task_status_id").toVariant());
        query.addBindValue(notSyncedRecord.value("fk_department_id").toVariant());
        query.addBindValue(notSyncedRecord.value("title").toVariant());
        query.addBindValue(notSyncedRecord.value("description").toVariant());
        query.addBindValue(notSyncedRecord.value("uuid").toVariant());

        if(!query.exec()) {
            qDebug() << "[syncTasks] Помилка оновлення даних у задачі:" << query.lastError().text();
        }
        else {
            qDebug() << "[syncTasks] Дані задачі " << notSyncedRecord.value("uuid").toString() << " оновлено";
        }
        break;

    default:
        qDebug() << "[syncTasks] Невідома операція";
        break;
    }
}

QString DbManager::getSyncTime(const QJsonObject &notSyncedRecord) {
    QSqlQuery query;
    QString syncTimeResult;
    QString sqlSyncDateReq = QString("SELECT update_time FROM %1 WHERE uuid = ?").arg(notSyncedRecord["table_name"].toString());

    query.prepare(sqlSyncDateReq);
    query.addBindValue(notSyncedRecord.value("uuid").toString());

    if (!query.exec()) {
        qDebug() << "[getSyncTime] Помилка виконання запиту:" << query.lastError().text();
    }
    else if (query.next()) {
        syncTimeResult = query.value("update_time").toString();
        qDebug() << "[getSyncTime] Успішно отримано час:" << syncTimeResult;
    }
    else {
        qDebug() << "[getSyncTime] Запис з UUID:" << notSyncedRecord.value("uuid").toString()
                 << " не знайдено в таблиці" << notSyncedRecord.value("table_name").toString();
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

QJsonArray DbManager::getAllTasksList(const int &page){
    QJsonArray result;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "title, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id = 0 AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getAllTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            QJsonObject taskInfo;
            taskInfo["uuid"] = QJsonValue::fromVariant(query.value("uuid"));
            taskInfo["assigner"] = QJsonValue::fromVariant(query.value("assigner"));
            taskInfo["assignee"] = QJsonValue::fromVariant(query.value("assignee"));
            taskInfo["create_date"] = QJsonValue::fromVariant(query.value("create_date"));
            taskInfo["due_date"] = QJsonValue::fromVariant(query.value("due_date"));
            taskInfo["fk_task_status_id"] = QJsonValue::fromVariant(query.value("fk_task_status_id"));
            taskInfo["title"] = QJsonValue::fromVariant(query.value("title"));
            taskInfo["description"] = QJsonValue::fromVariant(query.value("description"));
            taskInfo["fk_department_id"] = QJsonValue::fromVariant(query.value("fk_department_id"));
            result.push_back(taskInfo);
        }

        qDebug() << "[getAllTasksList] відпрацював успішно";
    }

    return result;
}

QJsonArray DbManager::getUserTasksList(const QString &username, const int &page){
    QJsonArray result;
    QJsonObject taskInfo;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "title, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id=0 AND assignee=? AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue(username);
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getUserTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            taskInfo["uuid"] = QJsonValue::fromVariant(query.value("uuid"));
            taskInfo["assigner"] = QJsonValue::fromVariant(query.value("assigner"));
            taskInfo["assignee"] = QJsonValue::fromVariant(query.value("assignee"));
            taskInfo["create_date"] = QJsonValue::fromVariant(query.value("create_date"));
            taskInfo["due_date"] = QJsonValue::fromVariant(query.value("due_date"));
            taskInfo["fk_task_status_id"] = QJsonValue::fromVariant(query.value("fk_task_status_id"));
            taskInfo["title"] = QJsonValue::fromVariant(query.value("title"));
            taskInfo["description"] = QJsonValue::fromVariant(query.value("description"));
            taskInfo["fk_department_id"] = QJsonValue::fromVariant(query.value("fk_department_id"));
            result.push_back(taskInfo);
        }

        qDebug() << "[getUserTasksList] відпрацював успішно";
    }

    return result;
}

QJsonArray DbManager::getDepartmentTasksList(const int &departmentId, const int &page){
    QJsonArray result;
    QJsonObject taskInfo;
    QSqlQuery query;

    query.prepare("SELECT uuid, assigner, assignee, "
                  "create_date, due_date, fk_task_status_id, "
                  "title, description, fk_department_id "
                  "FROM tasks WHERE fk_task_status_id = 0 AND fk_department_id=? AND last_operation<>0 "
                  "ORDER BY due_date ASC "
                  "LIMIT 10 OFFSET ?");
    query.addBindValue(departmentId);
    query.addBindValue((page-1)*10);

    if (!query.exec()) {
        qDebug() << "[getDepartmentTasksList] видав помилку:" << query.lastError().text();
    } else {
        while (query.next()){
            taskInfo["uuid"] = QJsonValue::fromVariant(query.value("uuid"));
            taskInfo["assigner"] = QJsonValue::fromVariant(query.value("assigner"));
            taskInfo["assignee"] = QJsonValue::fromVariant(query.value("assignee"));
            taskInfo["create_date"] = QJsonValue::fromVariant(query.value("create_date"));
            taskInfo["due_date"] = QJsonValue::fromVariant(query.value("due_date"));
            taskInfo["fk_task_status_id"] = QJsonValue::fromVariant(query.value("fk_task_status_id"));
            taskInfo["title"] = QJsonValue::fromVariant(query.value("title"));
            taskInfo["description"] = QJsonValue::fromVariant(query.value("description"));
            taskInfo["fk_department_id"] = QJsonValue::fromVariant(query.value("fk_department_id"));
            result.push_back(taskInfo);
        }

        qDebug() << "[getDepartmentTasksList] відпрацював успішно";
    }

    return result;
}

QJsonArray DbManager::getNotSyncedData(const QString &tableName) {
    QJsonArray result;
    QString sqlNotSyncedReq = QString("SELECT * FROM %1 WHERE sync_status = false").arg(tableName);
    QSqlQuery query;

    if (!query.exec(sqlNotSyncedReq)) {
        qDebug() << "[getNotSyncedData] видав помилку:" << query.lastError().text();
    }
    else {
        QSqlRecord record = query.record();

        while (query.next()) {
            QJsonObject notSyncedRecord;

            for (int i = 0; i < record.count(); i++) {
                notSyncedRecord[record.fieldName(i)] = QJsonValue::fromVariant(query.value(i));
            }
            result.push_back(notSyncedRecord);
        }

         qDebug() << "[getNotSyncedData] відпрацював успішно";
    }

    return result;
}

int DbManager::getDepartmentNumber(const QString &departmentName){
    QSqlQuery query;
    query.prepare("SELECT department_id FROM departments "
                  "WHERE name=?");

    query.addBindValue(departmentName);
    query.exec();
    if (query.next()) {
        qDebug() << "[getDepartmentNumber] успішно отримали номер департаменту";
        return query.value("department_id").toInt();
    }
    else {
        qDebug() << "[getDepartmentNumber] департаменту не існує";
        return 0;
    }
}

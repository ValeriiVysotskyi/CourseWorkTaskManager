#ifndef PASSWORDHASHING_H
#define PASSWORDHASHING_H

#include <QUuid>
#include <QCryptographicHash>
#include <QMap>

QMap<QString, QString> hashPassword(const QString &inputPassword){
    QMap<QString, QString> result;
    QString salt = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString passwordForHash = inputPassword + salt;

    result["password_hash"] = QCryptographicHash::hash((passwordForHash).toUtf8(), QCryptographicHash::Sha256).toHex();;
    result["salt"] = salt;
    return result;
}
#endif // PASSWORDHASHING_H

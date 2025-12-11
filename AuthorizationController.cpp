#include "authorizationController.h"


AuthorizationController::AuthorizationController(QObject *parent)
    : QObject(parent){}

QString AuthorizationController::hashPassword(const QString &inputPassword, const QString &salt){
    QString result;
    QString passwordForHash = inputPassword + salt;
    result = QCryptographicHash::hash((passwordForHash).toUtf8(), QCryptographicHash::Sha256).toHex();

    return result;
}

void AuthorizationController::checkAuthorization(const QString &username, const QString &password,
    const QMap<QString, QVariant> &dbUserPasswdInfo){
    QString inputHashedPasswd = hashPassword(password, dbUserPasswdInfo["password_salt"].toString());
    if (inputHashedPasswd == dbUserPasswdInfo["password_hash"].toString()){
        m_currentUser = username;
        emit loginSuccessfull();
    }
    else{
        emit loginFailed();
    }
}

QString AuthorizationController::currentUser() const{
    return m_currentUser;
}

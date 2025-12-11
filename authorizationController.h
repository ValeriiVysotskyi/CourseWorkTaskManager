#ifndef AUTHORIZATIONCONTROLLER_H
#define AUTHORIZATIONCONTROLLER_H

#include <QUuid>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QString>
#include <QObject>


class AuthorizationController:public QObject{
    Q_OBJECT

public:
    explicit AuthorizationController(QObject *parent = nullptr);

    Q_INVOKABLE void checkAuthorization(const QString &usernameInput, const QString &passwordInput,
        const QMap<QString, QVariant> &dbUserPasswdInfo);
    Q_INVOKABLE QString currentUser() const;

    QString hashPassword(const QString &inputPassword, const QString &salt);


signals:
    void loginSuccessfull();
    void loginFailed();

private:
    QString m_currentUser;
};

#endif // AUTHORIZATIONCONTROLLER_H

#pragma once

#include "AccountController.h"
#include <QObject>
#include <QHash>
#include <QString>
#include <Qontrol>
#include <silent.h>

class AccountWidget;

class AppController : public qontrol::Controller {
    Q_OBJECT

public:
    AppController();
    static void init();
    static auto get() -> AppController *;
    auto accounts() -> int;

signals:
    void accountList(QList<QString>);
    void accountCreated(const QString &name);

public slots:
    void initState();
    void addAccount(const QString &name);
    void removeAccount(const QString &account);
    void listAccounts();
    void onCreateAccount();
    void createAccount(const QString &name, const QString &mnemonic,
                      Network network, const QString &blindbit_url);
    void onAccountCreated(const QString &name);
    void openAccount(const QString &name);

private:
    QHash<QString, AccountController *> m_accounts;
};

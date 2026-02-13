#pragma once

#include "AccountController.h"
#include <QHash>
#include <QObject>
#include <QString>
#include <Qontrol>
#include <silent.h>

class AccountWidget;

class AppController : public qontrol::Controller {
    Q_OBJECT

public:
    AppController();
    static auto init() -> void;
    static auto get() -> AppController *;
    auto accounts() -> int;
    [[nodiscard]] auto isAccountOpen(const QString &name) const -> bool;

signals:
    void accountList(QList<QString>);
    void accountCreated(const QString &name);

public slots:
    auto initState() -> void;
    auto addAccount(const QString &name) -> void;
    auto removeAccount(const QString &account) -> void;
    auto listAccounts() -> void;
    auto onCreateAccount() -> void;
    auto createAccount(const QString &name, const QString &mnemonic, Network network,
                       const QString &blindbit_url) -> void;
    auto onAccountCreated(const QString &name) -> void;
    auto openAccount(const QString &name) -> void;
    auto deleteAccount(const QString &name) -> void;
    auto onDeleteConfirmed(const QString &account) -> void;

private:
    QHash<QString, AccountController *> m_accounts;
};

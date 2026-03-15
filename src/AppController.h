#pragma once

#include "AccountController.h"
#include <QHash>
#include <QObject>
#include <QString>
#include <Qontrol>
#include <optional>
#include <silent.h>

class AccountWidget;

struct RegtestDefaultsInfo {
    QString blindbit_url;
    QString p2p_node;
    QString electrum_url;
};

class AppController : public qontrol::Controller {
    Q_OBJECT

public:
    AppController();
    static auto init() -> void;
    static auto get() -> AppController *;
    auto accounts() -> int;
    [[nodiscard]] auto isAccountOpen(const QString &name) const -> bool;
    [[nodiscard]] auto regtestDefaults() const -> std::optional<RegtestDefaultsInfo>;

signals:
    void accountList(QList<QString>);
    void accountCreated(const QString &name);
    void regtestDefaultsReady(const QString &blindbit, const QString &p2p, const QString &electrum);

public slots:
    auto initState() -> void;
    auto addAccount(const QString &name) -> void;
    auto removeAccount(const QString &account) -> void;
    auto listAccounts() -> void;
    auto onCreateAccount() -> void;
    auto createAccount(const QString &name, const QString &mnemonic, Network network,
                       const QString &blindbit_url, const QString &p2p_node,
                       const QString &electrum_url) -> void;
    auto onAccountCreated(const QString &name) -> void;
    auto openAccount(const QString &name) -> void;
    auto deleteAccount(const QString &name) -> void;
    auto onDeleteConfirmed(const QString &account) -> void;
    auto onRegtestDefaultsReady(const QString &blindbit, const QString &p2p,
                                const QString &electrum) -> void;

private:
    QHash<QString, AccountController *> m_accounts;
    std::optional<RegtestDefaultsInfo> m_regtest_defaults;
};

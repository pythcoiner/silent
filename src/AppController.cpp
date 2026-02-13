#include "AppController.h"
#include "AccountWidget.h"
#include "MainWindow.h"
#include "screens/modals/ConfirmDelete.h"
#include "screens/modals/CreateAccount.h"
#include <common.h>
#include <qlogging.h>

AppController::AppController() = default;

auto AppController::init() -> void {
    if (Controller::isInit()) {
        qFatal() << "Controller have already been initialized!";
    }
    Controller::init(new AppController);
}

auto AppController::get() -> AppController * {
    auto *ctrl = Controller::get();
    auto *controller = dynamic_cast<AppController *>(ctrl);
    return controller;
}

auto AppController::initState() -> void {
    connect(this, &AppController::accountCreated, this, &AppController::onAccountCreated,
            qontrol::UNIQUE);
    listAccounts();
}

auto AppController::addAccount(const QString &name) -> void {
    auto *window = AppController::window();
    auto *win = dynamic_cast<MainWindow *>(window);
    if (win == nullptr) {
        qCritical() << "Failed to cast window to MainWindow";
        return;
    }
    if (!win->accountExists(name)) {
        auto *acc = new AccountWidget(name);
        win->insertAccount(acc, name);
        m_accounts.insert(name, acc->controller());
    } else {
        qCritical() << "Account " << name << " already exists!";
    }
}

auto AppController::removeAccount(const QString &account) -> void {
    auto *win = dynamic_cast<MainWindow *>(window());
    if (win == nullptr) {
        qCritical() << "Failed to cast window to MainWindow";
        return;
    }
    win->removeAccount(account);
    m_accounts.remove(account);
    listAccounts();
}

auto AppController::listAccounts() -> void {
    auto raccounts = list_configs();
    auto accounts = QList<QString>();
    for (auto &acc : raccounts) {
        auto a = QString::fromStdString(std::string(acc.c_str()));
        accounts.append(a);
    }
    emit accountList(accounts);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto AppController::onCreateAccount() -> void {
    auto *modal = new CreateAccount();
    AppController::execModal(modal);
}

auto AppController::createAccount(const QString &name, const QString &mnemonic, Network network,
                                  const QString &blindbit_url) -> void {
    // Create config with default dust limit of 546 sats
    auto config =
        new_config(rust::String(name.toStdString()), network, rust::String(mnemonic.toStdString()),
                   rust::String(blindbit_url.toStdString()), 546);

    // Save config to file
    config->to_file();

    emit accountCreated(name);
}

auto AppController::onAccountCreated(const QString &name) -> void {
    addAccount(name);
    listAccounts();
}

auto AppController::openAccount(const QString &name) -> void {
    addAccount(name);
    listAccounts();
}

auto AppController::deleteAccount(const QString &name) -> void {
    auto *modal = new ConfirmDelete(name);
    connect(modal, &ConfirmDelete::confirmed, this, &AppController::onDeleteConfirmed);
    AppController::execModal(modal);
}

auto AppController::onDeleteConfirmed(const QString &account) -> void {
    // Stop and remove account instance if running
    if (m_accounts.contains(account)) {
        m_accounts.value(account)->stop();
        auto *win = dynamic_cast<MainWindow *>(window());
        if (win != nullptr) {
            win->removeAccount(account);
        }
        m_accounts.remove(account);
    }
    if (!delete_config(rust::String(account.toStdString()))) {
        qCritical() << "Failed to delete account:" << account;
    }
    listAccounts();
}

auto AppController::accounts() -> int {
    return m_accounts.size();
}

auto AppController::isAccountOpen(const QString &name) const -> bool {
    return m_accounts.contains(name);
}

#include "AppController.h"
#include "MainWindow.h"
#include "AccountWidget.h"
#include "screens/modals/CreateAccount.h"
#include <qlogging.h>

AppController::AppController() = default;

void AppController::init() {
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

void AppController::initState() {
    // No logger initialization needed for silent (unlike qoinstr)
    listAccounts();
    connect(this, &AppController::accountCreated, this,
            &AppController::onAccountCreated, qontrol::UNIQUE);
}

void AppController::addAccount(const QString &name) {
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

void AppController::removeAccount(const QString &account) {
    auto *win = dynamic_cast<MainWindow *>(window());
    if (win == nullptr) {
        qCritical() << "Failed to cast window to MainWindow";
        return;
    }
    win->removeAccount(account);
    m_accounts.remove(account);
}

void AppController::listAccounts() {
    auto raccounts = list_configs();
    auto accounts = QList<QString>();
    for (auto &acc : raccounts) {
        accounts.append(QString::fromStdString(std::string(acc.c_str())));
    }
    emit accountList(accounts);
}

void AppController::onCreateAccount() {
    auto *modal = new CreateAccount();
    AppController::execModal(modal);
}

void AppController::createAccount(const QString &name, const QString &mnemonic,
                                  Network network, const QString &blindbit_url) {
    // Create config with default dust limit of 546 sats
    auto config = new_config(
        rust::String(name.toStdString()),
        network,
        rust::String(mnemonic.toStdString()),
        rust::String(blindbit_url.toStdString()),
        546
    );

    // Save config to file
    config->to_file();

    emit accountCreated(name);
}

void AppController::onAccountCreated(const QString &name) {
    qDebug() << "AppController::onAccountCreated()";
    addAccount(name);
}

auto AppController::accounts() -> int {
    return m_accounts.size();
}

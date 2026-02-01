#include "AccountController.h"
#include "AccountWidget.h"
#include "AppController.h"
#include <qlogging.h>
#include <qtimer.h>
#include <string>
#include <utility>

AccountController::AccountController(const QString &account, AccountWidget *widget) {
    m_widget = widget;
    init(account);
}

void AccountController::init(const QString &account) {
    if (m_init)
        return;

    // Initialize the account
    auto acc = new_account(rust::String(account.toStdString()));
    m_account = std::make_optional(std::move(acc));

    // Start the scanner
    if (m_account.has_value()) {
        m_account.value()->start_scanner();
    }

    // Initialize the timer that polls notifications every 100ms
    m_notif_timer = new QTimer(this);
    connect(m_notif_timer, &QTimer::timeout, this, &AccountController::poll);
    m_notif_timer->start(100);

    // Initialize the timer that polls coins every 1000ms
    m_coins_timer = new QTimer(this);
    connect(m_coins_timer, &QTimer::timeout, this, &AccountController::pollCoins);
    m_coins_timer->start(1000);

    m_init = true;
}

void AccountController::loadPanel(const QString &name) {
    auto *panel = m_panels.value(name);
    if (m_widget != nullptr && panel != nullptr) {
        m_widget->loadPanel(panel);
    }
}

void AccountController::insertPanel(qontrol::Panel *panel) {
    m_panels.insert(panel->name(), panel);
}

auto AccountController::screen(const QString &screen) -> std::optional<qontrol::Screen *> {
    auto *panel = m_panels.value(screen);
    if (panel != nullptr) {
        return std::optional(panel->screen());
    }
    return std::nullopt;
}

void AccountController::poll() {
    pollNotifications();
}

void AccountController::pollCoins() {
    if (m_account.has_value()) {
        auto coin_state = m_account.value()->spendable_coins();
        emit updateCoins(coin_state);

        auto balance = m_account.value()->balance();
        emit updateBalance(balance);
    }
}

void AccountController::pollNotifications() {
    if (!m_account.has_value())
        return;

    auto poll = m_account.value()->try_recv();
    while (poll->is_some()) {
        auto notif = poll->get_notification();
        auto flag = notif.flag;

        if (flag == NotificationFlag::ScanProgress) {
            // Parse payload for height and tip
            // Payload format expected: "height,tip" or similar
            // For now, emit with placeholder values
            emit scanProgress(0, 0);
        } else if (flag == NotificationFlag::NewOutput) {
            // New output received, update coins
            pollCoins();
        } else if (flag == NotificationFlag::OutputSpent) {
            // Output spent, update coins
            pollCoins();
        } else if (flag == NotificationFlag::ScanError) {
            auto error = poll->get_error();
            emit scanError(error);
        } else if (flag == NotificationFlag::ScanStarted) {
            qDebug() << "AccountController: Scan started";
        } else if (flag == NotificationFlag::ScanCompleted) {
            qDebug() << "AccountController: Scan completed";
            pollCoins();
        } else if (flag == NotificationFlag::Stopped) {
            qDebug() << "AccountController: Scanner stopped";
        } else {
            qDebug() << "AccountController: Unknown notification type";
        }

        poll = m_account.value()->try_recv();
    }
}

void AccountController::loadPanels() {
    // Phase 6: Create placeholder panels
    // Phase 7 will implement the actual screen classes
    // For now, we'll just create empty panels to make the side menu work

    // TODO: Implement actual screen classes in Phase 7
    // auto *cScreen = new screen::Coins(this);
    // auto *coins = new qontrol::Panel(cScreen, "coins");
    // this->insertPanel(coins);

    // Similar for Send, Receive, Settings screens
}

void AccountController::coinsClicked() {
    this->loadPanel("coins");
}

void AccountController::sendClicked() {
    this->loadPanel("send");
}

void AccountController::receiveClicked() {
    this->loadPanel("receive");
}

void AccountController::settingsClicked() {
    this->loadPanel("settings");
}

auto AccountController::simulateTx(TransactionTemplate tx) -> TransactionSimulation {
    if (m_account.has_value()) {
        return m_account.value()->simulate_transaction(std::move(tx));
    }
    qDebug() << "AccountController::simulateTx() m_account is None!";
    // Return empty/invalid simulation
    TransactionSimulation sim;
    sim.is_valid = false;
    sim.error = rust::String("Account not initialized");
    return sim;
}

void AccountController::stop() {
    if (m_notif_timer != nullptr) {
        m_notif_timer->stop();
    }
    if (m_coins_timer != nullptr) {
        m_coins_timer->stop();
    }
    if (m_account.has_value()) {
        m_account.value()->stop_scanner();
    }
}

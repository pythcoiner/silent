#include "AccountController.h"
#include "AccountWidget.h"
#include "AppController.h"
#include "screens/Coins.h"
#include "screens/Receive.h"
#include "screens/Send.h"
#include "screens/Settings.h"
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
            // Payload format: "height,tip" (e.g., "100,200")
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            auto parts = payload.split(",");
            if (parts.size() == 2) {
                bool ok1 = false, ok2 = false;
                uint32_t height = parts[0].toUInt(&ok1);
                uint32_t tip = parts[1].toUInt(&ok2);
                if (ok1 && ok2) {
                    emit scanProgress(height, tip);
                } else {
                    qWarning() << "Failed to parse scan progress payload:" << payload;
                    emit scanProgress(0, 0);
                }
            } else {
                qWarning() << "Invalid scan progress payload format:" << payload;
                emit scanProgress(0, 0);
            }
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
    // Create Coins screen panel
    auto *coinsScreen = new screen::Coins(this);
    auto *coinsPanel = new qontrol::Panel(coinsScreen, "coins");
    this->insertPanel(coinsPanel);

    // Create Receive screen panel
    auto *receiveScreen = new screen::Receive(this);
    auto *receivePanel = new qontrol::Panel(receiveScreen, "receive");
    this->insertPanel(receivePanel);

    // Create Send screen panel
    auto *sendScreen = new screen::Send(this);
    auto *sendPanel = new qontrol::Panel(sendScreen, "send");
    this->insertPanel(sendPanel);

    // Create Settings screen panel
    auto *settingsScreen = new screen::Settings(this);
    auto *settingsPanel = new qontrol::Panel(settingsScreen, "settings");
    this->insertPanel(settingsPanel);
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

auto AccountController::getCoins() -> rust::Vec<RustCoin> {
    if (m_account.has_value()) {
        return m_account.value()->coins();
    }
    return rust::Vec<RustCoin>();
}

auto AccountController::getSpAddress() -> rust::String {
    if (m_account.has_value()) {
        return m_account.value()->sp_address();
    }
    return rust::String("");
}

auto AccountController::coins() -> qontrol::Screen * {
    auto scr = screen("coins");
    if (scr.has_value()) {
        return scr.value();
    }
    return nullptr;
}

void AccountController::updateCoinLabel(const QString &outpoint, const QString &label) {
    if (m_account.has_value()) {
        try {
            m_account.value()->update_coin_label(
                rust::String(outpoint.toStdString()),
                rust::String(label.toStdString())
            );
            // Refresh coins to show updated label
            pollCoins();
        } catch (const std::exception &e) {
            qWarning() << "Failed to update coin label:" << e.what();
        }
    }
}

auto AccountController::getAccount() -> std::optional<rust::Box<Account>> & {
    return m_account;
}

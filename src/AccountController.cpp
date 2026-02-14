#include "AccountController.h"
#include "AccountWidget.h"
#include "AppController.h"
#include "screens/Coins.h"
#include "screens/Receive.h"
#include "screens/Send.h"
#include "screens/Settings.h"
#include "silent.h"
#include <qlogging.h>
#include <qtimer.h>
#include <string>
#include <utility>

AccountController::AccountController(const QString &account, AccountWidget *widget) {
    m_widget = widget;
    init(account);
}

auto AccountController::init(const QString &account) -> void {
    if (m_init)
        return;

    // Initialize the account
    auto acc = new_account(rust::String(account.toStdString()));
    if (!acc->is_ok()) {
        qCritical() << "Failed to create account:" << account
                     << QString::fromStdString(std::string(acc->get_error().c_str()));
        return;
    }
    m_account = std::make_optional(std::move(acc));

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

auto AccountController::loadPanel(const QString &name) -> void {
    auto *panel = m_panels.value(name);
    if (m_widget != nullptr && panel != nullptr) {
        m_widget->loadPanel(panel);
    }
}

auto AccountController::insertPanel(qontrol::Panel *panel) -> void {
    m_panels.insert(panel->name(), panel);
}

auto AccountController::screen(const QString &screen) -> std::optional<qontrol::Screen *> {
    auto *panel = m_panels.value(screen);
    if (panel != nullptr) {
        return std::optional(panel->screen());
    }
    return std::nullopt;
}

auto AccountController::poll() -> void {
    pollNotifications();
}

auto AccountController::pollCoins() -> void {
    if (m_account.has_value()) {
        auto coinState = m_account.value()->spendable_coins();
        emit updateCoins(coinState);

        auto balance = m_account.value()->balance();
        emit updateBalance(balance);
    }
}

auto AccountController::pollNotifications() -> void {
    if (!m_account.has_value())
        return;

    auto poll = m_account.value()->try_recv();
    while (poll->is_some()) {
        auto notif = poll->get_notification();
        auto flag = notif.flag;

        switch (flag) {
        case NotificationFlag::ScanProgress: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            auto parts = payload.split(",");
            if (parts.size() == 2) {
                bool ok1 = false;
                bool ok2 = false;
                uint32_t height = parts[0].toUInt(&ok1);
                uint32_t tip = parts[1].toUInt(&ok2);
                if (ok1 && ok2) {
                    qDebug() << "AccountController: Scan progress" << height << "/" << tip;
                    emit scanProgress(height, tip);
                } else {
                    qWarning() << "Failed to parse scan progress payload:" << payload;
                    emit scanProgress(0, 0);
                }
            } else {
                qWarning() << "Invalid scan progress payload format:" << payload;
                emit scanProgress(0, 0);
            }
            break;
        }
        case NotificationFlag::NewOutput:
        case NotificationFlag::OutputSpent:
            pollCoins();
            break;
        case NotificationFlag::FailStartScanning: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            qWarning() << "AccountController: Failed to start scanning:" << payload;
            emit scanError(notif.payload);
            break;
        }
        case NotificationFlag::FailScan: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            qWarning() << "AccountController: Scan failed:" << payload;
            emit scanError(notif.payload);
            break;
        }
        case NotificationFlag::StartingScan:
            qDebug() << "AccountController: Starting scan";
            m_scanner_running = true;
            emit scannerStateChanged(true);
            break;
        case NotificationFlag::ScanStarted: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            qDebug() << "AccountController: Scan started, range:" << payload;
            m_scanner_running = true;
            emit scannerStateChanged(true);
            break;
        }
        case NotificationFlag::StoppingScan:
            qDebug() << "AccountController: Stopping scan";
            break;
        case NotificationFlag::ScanCompleted:
            qDebug() << "AccountController: Scan completed";
            pollCoins();
            break;
        case NotificationFlag::ScanStopped:
            qDebug() << "AccountController: Scanner stopped";
            m_scanner_running = false;
            emit scannerStateChanged(false);
            break;
        case NotificationFlag::WaitingForBlocks: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            bool ok = false;
            uint32_t tipHeight = payload.toUInt(&ok);
            if (ok) {
                qDebug() << "AccountController: Waiting for blocks at tip" << tipHeight;
                emit waitingForBlocks(tipHeight);
            }
            break;
        }
        case NotificationFlag::NewBlocksDetected: {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            auto parts = payload.split(",");
            if (parts.size() == 2) {
                qDebug() << "AccountController: New blocks" << parts[0] << "->" << parts[1];
            }
            break;
        }
        default:
            qDebug() << "AccountController: Unknown notification type";
            break;
        }

        poll = m_account.value()->try_recv();
    }
}

auto AccountController::loadPanels() -> void {
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

auto AccountController::coinsClicked() -> void {
    this->loadPanel("coins");
}

auto AccountController::sendClicked() -> void {
    this->loadPanel("send");
}

auto AccountController::receiveClicked() -> void {
    this->loadPanel("receive");
}

auto AccountController::settingsClicked() -> void {
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

auto AccountController::startScanner() -> void {
    if (m_account.has_value()) {
        m_account.value()->start_scanner();
    }
}

auto AccountController::stop() -> void {
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

auto AccountController::updateCoinLabel(const QString &outpoint, const QString &label) -> void {
    if (m_account.has_value()) {
        if (!m_account.value()->update_coin_label(rust::String(outpoint.toStdString()),
                                                  rust::String(label.toStdString()))) {
            qWarning() << "Failed to update coin label for:" << outpoint;
        }
        pollCoins();
    }
}

auto AccountController::getAccount() -> std::optional<rust::Box<Account>> & {
    return m_account;
}

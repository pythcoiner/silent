#include "AccountController.h"
#include "AccountWidget.h"
#include "AppController.h"
#include "screens/Coins.h"
#include "screens/Receive.h"
#include "screens/Send.h"
#include "screens/Settings.h"
#include <Qontrol>
#include <common.h>
#include <qlogging.h>
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
    m_electrum_expected_count = static_cast<int>(m_account.value()->sub_account_count());

    // Take the notification receiver and spawn a blocking listener thread
    connect(this, &AccountController::notificationReceived, this,
            &AccountController::handleNotification, qontrol::UNIQUE);
    auto receiver = m_account.value()->take_receiver();
    m_notif_thread = QThread::create([this, recv = std::move(receiver)]() {
        while (true) {
            auto poll = recv->recv();
            if (!poll->is_some()) {
                break;
            }
            auto notif = poll->get_notification();
            emit notificationReceived(notif);
        }
    });
    connect(m_notif_thread, &QThread::finished, m_notif_thread, &QThread::deleteLater);
    m_notif_thread->start();

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

auto AccountController::pollCoins() -> void {
    if (m_account.has_value()) {
        auto coinState = m_account.value()->spendable_coins();
        emit updateCoins(coinState);

        auto balance = m_account.value()->balance();
        emit updateBalance(balance);
    }
}

auto AccountController::etaSecs() const -> uint64_t {
    return m_estimator->estimate_secs();
}

auto AccountController::handleNotification(Notification notif) -> void {
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
                m_estimator->update(height, tip);
                emit scanProgress(height, tip);
            } else {
                emit scanProgress(0, 0);
            }
        } else {
            emit scanProgress(0, 0);
        }
        break;
    }
    case NotificationFlag::NewOutput:
    case NotificationFlag::OutputSpent:
        pollCoins();
        break;
    case NotificationFlag::FailStartScanning:
        emit scanError(notif.payload);
        break;
    case NotificationFlag::FailScan:
        emit scanError(notif.payload);
        break;
    case NotificationFlag::StartingScan:
        m_scanner_running = true;
        m_estimator->reset();
        emit scannerStateChanged(true);
        break;
    case NotificationFlag::ScanStarted:
        m_scanner_running = true;
        emit scannerStateChanged(true);
        break;
    case NotificationFlag::StoppingScan:
        break;
    case NotificationFlag::ScanCompleted:
        break;
    case NotificationFlag::ScanStopped:
        m_scanner_running = false;
        m_estimator->reset();
        emit scannerStateChanged(false);
        break;
    case NotificationFlag::WaitingForBlocks: {
        auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
        bool ok = false;
        uint32_t tipHeight = payload.toUInt(&ok);
        if (ok) {
            emit waitingForBlocks(tipHeight);
        }
        break;
    }
    case NotificationFlag::NewBlocksDetected:
        break;
    case NotificationFlag::CoinUpdate:
    case NotificationFlag::AddressTipChanged:
        pollCoins();
        break;
    case NotificationFlag::ElectrumStarted:
        break;
    case NotificationFlag::ElectrumConnected: {
        m_electrum_connected_count++;
        if (m_electrum_connected_count >= m_electrum_expected_count) {
            auto payload = QString::fromStdString(std::string(notif.payload.c_str()));
            emit electrumConnected(payload);
        }
        break;
    }
    case NotificationFlag::ElectrumError:
        emit scanError(notif.payload);
        break;
    case NotificationFlag::ElectrumStopped:
        m_electrum_connected_count--;
        if (m_electrum_connected_count <= 0) {
            m_electrum_connected_count = 0;
            emit electrumDisconnected();
        }
        break;
    default:
        break;
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
    if (m_account.has_value()) {
        m_account.value()->stop_scanner();
    }
    // Thread exits when the channel disconnects (sender dropped by stop_scanner)
    if (m_notif_thread != nullptr && m_notif_thread->isRunning()) {
        m_notif_thread->wait(5000);
    }
}

auto AccountController::getCoins() -> rust::Vec<RustCoin> {
    if (m_account.has_value()) {
        auto all = m_account.value()->coins();
        rust::Vec<RustCoin> spendable;
        for (auto &coin : all) {
            if (!coin.spent) {
                spendable.push_back(std::move(coin));
            }
        }
        return spendable;
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

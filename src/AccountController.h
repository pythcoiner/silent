#pragma once

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QThread>
#include <Qontrol>
#include <optional>
#include <silent.h>

class AccountWidget;

class AccountController : public QObject {
    Q_OBJECT

public:
    AccountController(const QString &account, AccountWidget *widget);
    auto init(const QString &account) -> void;
    auto screen(const QString &screen) -> std::optional<qontrol::Screen *>;
    auto loadPanels() -> void;
    auto getCoins() -> rust::Vec<RustCoin>;
    auto getSpAddress() -> rust::String;
    auto coins() -> qontrol::Screen *;
    auto getAccount() -> std::optional<rust::Box<Account>> &;

signals:
    void updateCoins(CoinState coins);
    void updateBalance(uint64_t balance);
    void newAddress(rust::String addr);
    void scanProgress(uint32_t height, uint32_t tip);
    void waitingForBlocks(uint32_t tip_height);
    void scanError(rust::String error);
    void scannerStateChanged(bool running);
    void notificationReceived(Notification notif);

public slots:
    auto loadPanel(const QString &name) -> void;
    auto insertPanel(qontrol::Panel *panel) -> void;
    auto pollCoins() -> void;
    auto handleNotification(Notification notif) -> void;
    auto simulateTx(TransactionTemplate tx) -> TransactionSimulation;
    auto updateCoinLabel(const QString &outpoint, const QString &label) -> void;
    auto startScanner() -> void;
    auto stop() -> void;

    // Screen button actions
    auto coinsClicked() -> void;
    auto sendClicked() -> void;
    auto receiveClicked() -> void;
    auto settingsClicked() -> void;

    [[nodiscard]] auto isScannerRunning() const -> bool {
        return m_scanner_running;
    }

    [[nodiscard]] auto etaSecs() const -> uint64_t;

private:
    QPointer<qontrol::Panel> m_current_panel;
    QHash<QString, qontrol::Panel *> m_panels;
    AccountWidget *m_widget;
    std::optional<rust::Box<Account>> m_account = std::nullopt;
    QThread *m_notif_thread = nullptr;
    rust::Box<SyncEstimator> m_estimator = new_sync_estimator();
    bool m_init = false;
    bool m_scanner_running = false;
};

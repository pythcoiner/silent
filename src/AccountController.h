#pragma once

#include "AccountWidget.h"
#include <QObject>
#include <QTimer>
#include <QHash>
#include <QString>
#include <QPointer>
#include <Qontrol>
#include <optional>
#include <templar.h>

class AccountWidget;

class AccountController : public QObject {
    Q_OBJECT

public:
    AccountController(const QString &account, AccountWidget *widget);
    void init(const QString &account);
    auto screen(const QString &screen) -> std::optional<qontrol::Screen *>;
    void loadPanels();

signals:
    void updateCoins(CoinState coins);
    void updateBalance(uint64_t balance);
    void newAddress(rust::String addr);
    void scanProgress(uint32_t height, uint32_t tip);
    void scanError(rust::String error);

public slots:
    void loadPanel(const QString &name);
    void insertPanel(qontrol::Panel *panel);
    void poll();
    void pollCoins();
    void pollNotifications();
    auto simulateTx(TransactionTemplate tx) -> TransactionSimulation;
    void stop();

    // Screen button actions
    void coinsClicked();
    void sendClicked();
    void receiveClicked();
    void settingsClicked();

private:
    QPointer<qontrol::Panel> m_current_panel;
    QHash<QString, qontrol::Panel *> m_panels;
    AccountWidget *m_widget;
    std::optional<rust::Box<Account>> m_account = std::nullopt;
    QTimer *m_notif_timer = nullptr;
    QTimer *m_coins_timer = nullptr;
    bool m_init = false;
};

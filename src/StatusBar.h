#pragma once

#include <QLabel>
#include <QWidget>
#include <Qontrol>
#include <silent.h>

class AccountController;

class StatusBar : public QWidget {
    Q_OBJECT

public:
    explicit StatusBar(AccountController *controller, QWidget *parent = nullptr);

public slots:
    auto updateConnectionState(bool connected) -> void;
    auto updateScanProgress(uint32_t height, uint32_t tip) -> void;
    auto updateWaitingForBlocks(uint32_t tip_height) -> void;
    auto updateScanError(rust::String error) -> void;
    auto onElectrumConnected(QString address) -> void;
    auto onElectrumDisconnected() -> void;
    auto reloadUrl() -> void;

protected:
    auto onToggled(bool checked) -> void;
    auto onElectrumToggled(bool checked) -> void;
    auto loadBlindbitUrl() -> void;
    auto loadElectrumUrl() -> void;
    auto initUI() -> void;

private:
    AccountController *m_controller = nullptr;
    qontrol::widgets::ToggleSwitch *m_toggle = nullptr;
    QLabel *m_status_label = nullptr;
    QString m_blindbit_url;
    bool m_connected = false;

    qontrol::widgets::ToggleSwitch *m_electrum_toggle = nullptr;
    QLabel *m_electrum_status_label = nullptr;
    QString m_electrum_url;
    bool m_electrum_connected = false;
};

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
    auto reloadUrl() -> void;

private slots:
    auto onToggled(bool checked) -> void;

private:
    auto initUI() -> void;
    auto loadBlindbitUrl() -> void;

    AccountController *m_controller = nullptr;
    qontrol::widgets::ToggleSwitch *m_toggle = nullptr;
    QLabel *m_status_text = nullptr;
    QString m_blindbit_url;
    bool m_connected = false;
};

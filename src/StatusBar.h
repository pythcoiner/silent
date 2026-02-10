#pragma once

#include <QWidget>
#include <QLabel>
#include <Qontrol>
#include <silent.h>

class AccountController;

class StatusBar : public QWidget {
    Q_OBJECT

public:
    explicit StatusBar(AccountController *controller, QWidget *parent = nullptr);

public slots:
    void updateConnectionState(bool connected);
    void updateScanProgress(uint32_t height, uint32_t tip);
    void updateWaitingForBlocks(uint32_t tipHeight);
    void updateScanError(rust::String error);
    void reloadUrl();

private slots:
    void onToggled(bool checked);

private:
    void initUI();
    void loadBlindbitUrl();

    AccountController *m_controller = nullptr;
    qontrol::widgets::ToggleSwitch *m_toggle = nullptr;
    QLabel *m_status_text = nullptr;
    QString m_blindbit_url;
    bool m_connected = false;
};

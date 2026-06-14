#pragma once

#include <QWidget>
#include <Qontrol>
#include <silent.h>

class AccountController;

namespace theme {
class Label;
}

namespace theme {
class Toggle;
}

class StatusBar : public QWidget {
    Q_OBJECT

public:
    explicit StatusBar(AccountController *controller, QWidget *parent = nullptr);

public slots:
    void onUpdateConnectionState(bool connected);
    void onUpdateScanProgress(uint32_t height, uint32_t tip);
    void onUpdateWaitingForBlocks(uint32_t tip_height);
    void onUpdateScanError(rust::String error);
    void onElectrumConnected(const QString &address);
    void onElectrumDisconnected();
    void onReloadUrl();

protected:
    void onToggled(bool checked);
    void onElectrumToggled(bool checked);
    void loadBlindbitUrl();
    void loadElectrumUrl();
    void initUI();

private:
    AccountController *m_controller = nullptr;
    theme::Toggle *m_toggle = nullptr;
    theme::Label *m_status_label = nullptr;
    QString m_blindbit_url;
    bool m_connected = false;

    theme::Toggle *m_electrum_toggle = nullptr;
    theme::Label *m_electrum_status_label = nullptr;
    QString m_electrum_url;
    bool m_electrum_connected = false;
};

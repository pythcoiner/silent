#pragma once

#include <Qontrol>
#include <qevent.h>
#include <qtmetamacros.h>

namespace theme {
class Button;
}

namespace theme {
class ComboBox;
}

namespace theme {
class Input;
}

namespace theme {
class Label;
}

#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace screen {

class Settings : public qontrol::Screen {
    Q_OBJECT
public:
    explicit Settings(AccountController *ctrl);

signals:
    void configSaved();
    void backendInfoReady(BackendInfo info);
    void p2pTestReady(ConnectionResult result);
    void electrumTestReady(ConnectionResult result);

public slots:
    void onActionSave();
    void onActionToggleBlindbit();
    void onActionToggleElectrum();
    void onActionTestBackend();
    void onActionTestP2p();
    void onActionTestElectrum();
    void onActionApplyLanguage();
    void onUpdateBlindbitToggleButton(bool running);
    void onUpdateElectrumToggleButton();
    void onScanProgress(uint32_t height, uint32_t tip);

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    void changeEvent(QEvent *event) override;
    void retranslateUi();

public slots:
    void onBackendInfoReady(BackendInfo info);
    void onP2pTestReady(ConnectionResult result);
    void onElectrumTestReady(ConnectionResult result);

protected:
    void fetchBackendInfo();
    void invalidateBackendTest();
    void clearBackendInfo();
    void invalidateP2pTest();
    void invalidateElectrumTest();
    void updateButtons();

private:
    AccountController *m_controller = nullptr;
    bool m_backend_verified = false;
    bool m_p2p_verified = false;
    bool m_electrum_verified = false;
    QWidget *m_main_widget = nullptr;
    theme::Input *m_blindbit_url_input = nullptr;
    theme::Input *m_p2p_node_input = nullptr;
    theme::Input *m_electrum_url_input = nullptr;
    theme::ComboBox *m_network_selector = nullptr;
    theme::ComboBox *m_language_selector = nullptr;
    theme::Button *m_save_btn = nullptr;
    theme::Button *m_toggle_blindbit_btn = nullptr;
    theme::Button *m_toggle_electrum_btn = nullptr;
    theme::Button *m_test_btn = nullptr;
    theme::Button *m_test_p2p_btn = nullptr;
    theme::Button *m_test_electrum_btn = nullptr;
    theme::Button *m_apply_language_btn = nullptr;
    theme::Label *m_info_network_label = nullptr;
    theme::Label *m_info_height_label = nullptr;
    theme::Label *m_capabilities = nullptr;
    theme::Label *m_language_status_label = nullptr;
    QString m_current_url;
    QString m_current_p2p_node;
    QString m_current_electrum_url;
    Network m_current_network = Network::Signet;
    uint32_t m_current_height = 0;
    bool m_electrum_running = false;
};

} // namespace screen

#pragma once

#include <Qontrol>
#include <qevent.h>
#include <qtmetamacros.h>

namespace catalog {
class Button;
}

namespace catalog {
class ComboBox;
}

namespace catalog {
class Input;
}

namespace catalog {
class ValidationMark;
}

namespace catalog {
class Label;
}

#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace view {

class Settings : public qontrol::Screen {
    Q_OBJECT
public:
    explicit Settings(AccountController *ctrl);

signals:
    void configSaved();
    void backendInfoReady(BackendInfo info);
    void electrumTestReady(ConnectionResult result);

public slots:
    void onActionSave();
    void onActionToggleBlindbit();
    void onActionToggleElectrum();
    void onActionTestBackend();
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
    void onElectrumTestReady(ConnectionResult result);

protected:
    void fetchBackendInfo();
    void invalidateBackendTest();
    void clearBackendInfo();
    void invalidateElectrumTest();
    void updateButtons();

private:
    AccountController *m_controller = nullptr;
    bool m_backend_verified = false;
    bool m_electrum_verified = false;
    QWidget *m_main_widget = nullptr;
    catalog::Input *m_blindbit_url_input = nullptr;
    catalog::Input *m_electrum_url_input = nullptr;
    catalog::ComboBox *m_network_selector = nullptr;
    catalog::ComboBox *m_language_selector = nullptr;
    catalog::Button *m_save_btn = nullptr;
    catalog::Button *m_toggle_blindbit_btn = nullptr;
    catalog::Button *m_toggle_electrum_btn = nullptr;
    catalog::Button *m_test_btn = nullptr;
    catalog::Button *m_test_electrum_btn = nullptr;
    catalog::Button *m_apply_language_btn = nullptr;
    catalog::Label *m_info_network_label = nullptr;
    catalog::Label *m_info_height_label = nullptr;
    catalog::Label *m_capabilities = nullptr;
    catalog::Label *m_language_status_label = nullptr;
    catalog::ValidationMark *m_backend_status = nullptr;
    catalog::ValidationMark *m_electrum_status = nullptr;
    QString m_current_url;
    QString m_current_electrum_url;
    Network m_current_network = Network::Signet;
    uint32_t m_current_height = 0;
    bool m_electrum_running = false;
};

} // namespace view

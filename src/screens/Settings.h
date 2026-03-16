#pragma once

#include <Qontrol>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
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
    auto actionSave() -> void;
    auto actionToggleBlindbit() -> void;
    auto actionToggleElectrum() -> void;
    auto actionTestBackend() -> void;
    auto actionTestP2p() -> void;
    auto actionTestElectrum() -> void;
    auto updateBlindbitToggleButton(bool running) -> void;
    auto updateElectrumToggleButton() -> void;
    auto onScanProgress(uint32_t height, uint32_t tip) -> void;

protected:
    auto init() -> void override;
    auto doConnect() -> void override;
    auto view() -> void override;

public slots:
    auto onBackendInfoReady(BackendInfo info) -> void;
    auto onP2pTestReady(ConnectionResult result) -> void;
    auto onElectrumTestReady(ConnectionResult result) -> void;

protected:
    auto fetchBackendInfo() -> void;
    auto invalidateBackendTest() -> void;
    auto clearBackendInfo() -> void;
    auto invalidateP2pTest() -> void;
    auto invalidateElectrumTest() -> void;
    auto updateButtons() -> void;

private:
    AccountController *m_controller = nullptr;
    bool m_backend_verified = false;
    bool m_p2p_verified = false;
    bool m_electrum_verified = false;
    QWidget *m_main_widget = nullptr;
    QLineEdit *m_blindbit_url = nullptr;
    QLineEdit *m_p2p_node = nullptr;
    QLineEdit *m_electrum_url = nullptr;
    QComboBox *m_network_selector = nullptr;
    QPushButton *m_btn_save = nullptr;
    QPushButton *m_btn_toggle_blindbit = nullptr;
    QPushButton *m_btn_toggle_electrum = nullptr;
    QPushButton *m_btn_test = nullptr;
    QPushButton *m_btn_test_p2p = nullptr;
    QPushButton *m_btn_test_electrum = nullptr;
    QLabel *m_info_network = nullptr;
    QLabel *m_info_height = nullptr;
    QLabel *m_info_tweaks = nullptr;
    QString m_current_url;
    QString m_current_p2p_node;
    QString m_current_electrum_url;
    Network m_current_network = Network::Signet;
    uint32_t m_current_height = 0;
    bool m_electrum_running = false;
};

} // namespace screen

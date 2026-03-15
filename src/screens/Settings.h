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

public slots:
    auto actionSave() -> void;
    auto actionToggle() -> void;
    auto actionTestBackend() -> void;
    auto actionTestP2p() -> void;
    auto updateToggleButton(bool running) -> void;
    auto onScanProgress(uint32_t height, uint32_t tip) -> void;

protected:
    auto init() -> void override;
    auto doConnect() -> void override;
    auto view() -> void override;

private:
    auto fetchBackendInfo() -> void;
    auto onBackendInfoReady(BackendInfo info) -> void;
    auto invalidateBackendTest() -> void;
    auto clearBackendInfo() -> void;
    auto onP2pTestReady(ConnectionResult result) -> void;
    auto invalidateP2pTest() -> void;
    auto updateButtons() -> void;

    AccountController *m_controller = nullptr;
    bool m_backend_verified = false;
    bool m_p2p_verified = false;
    QWidget *m_main_widget = nullptr;
    QLineEdit *m_blindbit_url = nullptr;
    QLineEdit *m_p2p_node = nullptr;
    QComboBox *m_network_selector = nullptr;
    QPushButton *m_btn_save = nullptr;
    QPushButton *m_btn_toggle = nullptr;
    QPushButton *m_btn_test = nullptr;
    QPushButton *m_btn_test_p2p = nullptr;
    QLabel *m_info_network = nullptr;
    QLabel *m_info_height = nullptr;
    QLabel *m_info_tweaks = nullptr;
    QString m_current_url;
    QString m_current_p2p_node;
    Network m_current_network = Network::Signet;
    uint32_t m_current_height = 0;
};

} // namespace screen

#pragma once

#include <Qontrol>
#include <qlabel.h>
#include <qcombobox.h>
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

public slots:
    void actionSave();
    void actionToggle();
    void actionTestBackend();
    void updateToggleButton(bool running);
    void onScanProgress(uint32_t height, uint32_t tip);

protected:
    void init() override;
    void doConnect() override;
    void view() override;

private:
    void fetchBackendInfo();
    void onBackendInfoReady(BackendInfo info);
    void invalidateBackendTest();
    void clearBackendInfo();
    void updateButtons();

    AccountController *m_controller = nullptr;
    bool m_view_init = false;
    bool m_backend_verified = false;
    QWidget *m_main_widget = nullptr;
    QLineEdit *m_blindbit_url = nullptr;
    QComboBox *m_network_selector = nullptr;
    QPushButton *m_btn_save = nullptr;
    QPushButton *m_btn_toggle = nullptr;
    QPushButton *m_btn_test = nullptr;
    QLabel *m_info_network = nullptr;
    QLabel *m_info_height = nullptr;
    QLabel *m_info_tweaks = nullptr;
    QString m_current_url;
    Network m_current_network = Network::Signet;
    uint32_t m_current_height = 0;
};

} // namespace screen

#pragma once

#include <Qontrol>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace screen {

class Receive : public qontrol::Screen {
    Q_OBJECT
public:
    Receive(AccountController *ctrl);

public slots:
    auto onCopyAddress() -> void;
    auto onNewSegwitAddr() -> void;
    auto onCopySegwitAddr() -> void;
    auto onNewTaprootAddr() -> void;
    auto onCopyTaprootAddr() -> void;

protected:
    auto init() -> void override;
    auto doConnect() -> void override;
    auto view() -> void override;

private:
    AccountController *m_controller = nullptr;
    QWidget *m_main_widget = nullptr;
    QPushButton *m_btn_copy = nullptr;
    rust::String m_sp_address;
    bool m_has_sub_accounts = false;
    QPushButton *m_btn_new_segwit = nullptr;
    QPushButton *m_btn_copy_segwit = nullptr;
    QLabel *m_segwit_addr_display = nullptr;
    QPushButton *m_btn_new_taproot = nullptr;
    QPushButton *m_btn_copy_taproot = nullptr;
    QLabel *m_taproot_addr_display = nullptr;
};

} // namespace screen

#pragma once

#include <Qontrol>
#include <qevent.h>
#include <qtmetamacros.h>

namespace theme {
class Button;
}

namespace theme {
class Label;
}

#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace screen {

class Receive : public qontrol::Screen {
    Q_OBJECT
public:
    Receive(AccountController *ctrl);

public slots:
    void onCopyAddress();
    void onNewSegwitAddr();
    void onCopySegwitAddr();
    void onNewTaprootAddr();
    void onCopyTaprootAddr();

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    AccountController *m_controller = nullptr;
    QWidget *m_main_widget = nullptr;
    theme::Button *m_copy_btn = nullptr;
    rust::String m_sp_address;
    bool m_has_sub_accounts = false;
    theme::Button *m_new_segwit_btn = nullptr;
    theme::Button *m_copy_segwit_btn = nullptr;
    theme::Label *m_segwit_addr_label = nullptr;
    theme::Button *m_new_taproot_btn = nullptr;
    theme::Button *m_copy_taproot_btn = nullptr;
    theme::Label *m_taproot_addr_label = nullptr;
};

} // namespace screen

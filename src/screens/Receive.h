#pragma once

#include <Qontrol>
#include <QApplication>
#include <QClipboard>
#include <qlineedit.h>
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

protected:
    void init() override;
    void doConnect() override;
    void view() override;

private:
    AccountController *m_controller;
    QWidget *m_main_widget = nullptr;
    QPushButton *m_btn_copy = nullptr;
    rust::String m_sp_address;
};

} // namespace screen

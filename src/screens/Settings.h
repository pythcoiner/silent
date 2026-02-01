#pragma once

#include <Qontrol>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <templar.h>

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

protected:
    void init() override;
    void doConnect() override;
    void view() override;

private:
    AccountController *m_controller = nullptr;
    bool m_view_init = false;
    QWidget *m_main_widget = nullptr;
    QLineEdit *m_blindbit_url = nullptr;
    QComboBox *m_network_selector = nullptr;
    QPushButton *m_btn_save = nullptr;
};

} // namespace screen

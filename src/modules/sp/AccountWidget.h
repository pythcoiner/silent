#pragma once

#include <QPointer>
#include <QWidget>
#include <Qontrol>

class AccountController;
class StatusBar;

class AccountWidget : public QWidget {
    Q_OBJECT

public:
    explicit AccountWidget(const QString &account, QWidget *parent = nullptr);
    ~AccountWidget() override;

    auto controller() -> AccountController *;
    auto loadPanel(qontrol::Panel *panel) -> void;

protected:
    auto initUI() -> void;

private:
    AccountController *m_controller = nullptr;
    qontrol::Column *m_menu = nullptr;
    QWidget *m_screen_container = nullptr;
    QPointer<qontrol::Panel> m_current_panel;
    StatusBar *m_status_bar = nullptr;
};

#pragma once

#include <QWidget>
#include <QPointer>
#include <Qontrol>

class AccountController;

class AccountWidget : public QWidget {
    Q_OBJECT

public:
    explicit AccountWidget(const QString &account, QWidget *parent = nullptr);
    ~AccountWidget() override;

    auto controller() -> AccountController *;
    void loadPanel(qontrol::Panel *panel);

private:
    AccountController *m_controller = nullptr;
    qontrol::Column *m_menu = nullptr;
    QWidget *m_screen_container = nullptr;
    QPointer<qontrol::Panel> m_current_panel;

    void initUI();
};

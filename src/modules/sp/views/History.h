#pragma once

#include <Qontrol>
#include <qevent.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace catalog {
class BalanceHeader;
class HistoryTable;
} // namespace catalog

namespace view {

class History : public qontrol::Screen {
    Q_OBJECT
public:
    History(AccountController *ctrl);

public slots:
    void onRecvPayload(const CoinState &state);

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    CoinState m_state{};
    QWidget *m_main_widget = nullptr;
    AccountController *m_controller = nullptr;
    catalog::BalanceHeader *m_balance = nullptr;
    catalog::HistoryTable *m_table = nullptr;
};

} // namespace view

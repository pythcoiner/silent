#pragma once

#include "theme/Palette.h"
#include <Qontrol>
#include <qscrollarea.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace theme {
class Label;
} // namespace theme

namespace screen {

class History : public qontrol::Screen {
    Q_OBJECT
public:
    History(AccountController *ctrl);

    static const int DIR_W = resolve(Size::XS);
    static const int TXID_W = resolve(Size::S);
    static const int HEIGHT_W = resolve(Size::XS);
    static const int AMOUNT_W = resolve(Size::S);

public slots:
    void recvPayload(const CoinState &state);

protected:
    void init() override;
    void doConnect() override;
    void view() override;

private:
    CoinState m_state{};
    QWidget *m_main_widget = nullptr;
    QWidget *m_confirmed_row = nullptr;
    QWidget *m_unconfirmed_row = nullptr;
    QScrollArea *m_scroll = nullptr;
    AccountController *m_controller = nullptr;

    // Persistent header labels
    theme::Label *m_h_direction = nullptr;
    theme::Label *m_h_txid = nullptr;
    theme::Label *m_h_height = nullptr;
    theme::Label *m_h_amount = nullptr;
};

} // namespace screen

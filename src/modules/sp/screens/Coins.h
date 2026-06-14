#pragma once

#include "theme/Palette.h"
#include <Qontrol>
#include <optional>
#include <qhash.h>
#include <qscrollarea.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace theme {
class Display;
class Input;
class Button;
class Label;
} // namespace theme

namespace screen {

class Coins : public qontrol::Screen {
    Q_OBJECT
public:
    Coins(AccountController *ctrl);
    auto getCoins() -> std::optional<QList<RustCoin>>;

    static const int TYPE_W = resolve(Size::XS);
    static const int HEIGHT_W = resolve(Size::XS);
    static const int OUTPOINT_W = resolve(Size::S);
    static const int LABEL_W = resolve(Size::M);
    static const int VALUE_W = resolve(Size::S);

signals:
    void coinsUpdated();

public slots:
    void onRecvPayload(const CoinState &state);
    void onEditLabelClicked();
    void onLabelEditFinished();

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    auto eventFilter(QObject *obj, QEvent *event) -> bool override;

private:
    CoinState m_state{};
    rust::Vec<RustCoin> m_coins{};
    QWidget *m_main_widget = nullptr;
    QWidget *m_confirmed_row = nullptr;
    QWidget *m_unconfirmed_row = nullptr;
    QScrollArea *m_scroll = nullptr;
    QHash<QString, theme::Input *> m_label_inputs;
    QList<theme::Button *> m_edit_buttons;
    AccountController *m_controller = nullptr;
    void setEditButtonsEnabled(bool enabled);

    // Persistent header labels
    theme::Label *m_h_type = nullptr;
    theme::Label *m_h_height = nullptr;
    theme::Label *m_h_outpoint = nullptr;
    theme::Label *m_h_label = nullptr;
    QWidget *m_h_edit = nullptr;
    theme::Label *m_h_value = nullptr;
};

} // namespace screen

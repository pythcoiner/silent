#pragma once

#include "theme/Palette.h"
#include <Qontrol>
#include <optional>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace catalog {
class BalanceHeader;
class Table;
} // namespace catalog

namespace view {

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
    void onCellEdited(const QString &outpoint, int column, const QString &text);

protected:
    void init() override;
    void doConnect() override;
    void view() override;

private:
    CoinState m_state{};
    rust::Vec<RustCoin> m_coins{};
    QWidget *m_main_widget = nullptr;
    AccountController *m_controller = nullptr;
    catalog::BalanceHeader *m_balance = nullptr;
    catalog::Table *m_table = nullptr;
};

} // namespace view

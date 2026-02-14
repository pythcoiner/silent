#pragma once

#include <Qontrol>
#include <optional>
#include <qtablewidget.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace screen {

class Coins : public qontrol::Screen {
    Q_OBJECT
public:
    Coins(AccountController *ctrl);
    auto getCoins() -> std::optional<QList<RustCoin>>;

signals:
    void coinsUpdated();

public slots:
    auto recvPayload(const CoinState &state) -> void;
    auto onLabelEdited(QTableWidgetItem *item) -> void;

protected:
    auto init() -> void override;
    auto doConnect() -> void override;
    auto view() -> void override;

private:
    CoinState m_state{};
    rust::Vec<RustCoin> m_coins{};
    QWidget *m_main_widget = nullptr;
    QWidget *m_confirmed_row = nullptr;
    QWidget *m_unconfirmed_row = nullptr;
    QTableWidget *m_table = nullptr;
    AccountController *m_controller = nullptr;
};

} // namespace screen

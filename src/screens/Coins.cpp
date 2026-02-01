#include "Coins.h"
#include "AccountController.h"
#include "common.h"
#include <Qontrol>
#include <cstdint>
#include <optional>
#include <qlabel.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qtablewidget.h>

namespace screen {

Coins::Coins(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

void Coins::init() {
}

void Coins::recvPayload(const CoinState &state) {
    // Check if state has changed
    if (m_state.confirmed_balance == state.confirmed_balance &&
        m_state.confirmed_count == state.confirmed_count &&
        m_state.unconfirmed_balance == state.unconfirmed_balance &&
        m_state.unconfirmed_count == state.unconfirmed_count) {
        return;
    }
    m_state = state;
    this->view();
    emit coinsUpdated();
}

void Coins::doConnect() {
    auto *ctrl = m_controller;
    connect(ctrl, &AccountController::updateCoins, this, &Coins::recvPayload);
}

auto balanceRow(const QString &label_str, uint64_t balance,
                uint64_t coins_count) -> QWidget * {
    auto *label = new QLabel(label_str);
    label->setFixedWidth(LABEL_WIDTH);

    auto priceStr = toBitcoin(balance);
    auto *price = new QLabel(priceStr);
    price->setFixedWidth(PRICE_WIDTH);

    auto coinStr = coinsCount(coins_count);
    auto *coins = new QLabel(coinStr);
    coins->setFixedWidth(PRICE_WIDTH);

    auto *row = (new qontrol::Row)
                    ->push(label)
                    ->push(price)
                    ->push(coins)
                    ->pushSpacer();

    return row;
}

void insertCoin(QTableWidget *table, const RustCoin &coin, int index) {
    QString blockHeight;
    if (!coin.spent && coin.height > 0) {
        blockHeight = QString::number(coin.height);
    } else if (coin.spent) {
        blockHeight = "Spent";
    } else {
        blockHeight = "Unconfirmed";
    }

    auto *value = new QTableWidgetItem(toBitcoin(coin.value, false));
    value->setTextAlignment(Qt::AlignCenter);

    table->setItem(index, 0, new QTableWidgetItem(blockHeight));
    auto op = coin.outpoint;
    table->setItem(index, 1, new QTableWidgetItem(QString(op.c_str())));
    auto label = coin.label;
    table->setItem(index, 2, new QTableWidgetItem(QString(label.c_str())));
    table->setItem(index, 3, value);
}

void Coins::view() {
    auto *oldCR = m_confirmed_row;
    m_confirmed_row = balanceRow("Confirmed:", m_state.confirmed_balance,
                                 m_state.confirmed_count);
    delete oldCR;

    auto *oldUR = m_unconfirmed_row;
    m_unconfirmed_row = balanceRow("Unconfirmed:", m_state.unconfirmed_balance,
                                   m_state.unconfirmed_count);
    delete oldUR;

    // Get coins from controller's account
    if (m_controller != nullptr) {
        m_coins = m_controller->getCoins();
    } else {
        m_coins = rust::Vec<RustCoin>();
    }

    int rowCount = m_coins.size();
    auto *table = new QTableWidget(rowCount, 4);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto headers = QStringList{"Block Height", "OutPoint", "Label", "Value"};
    table->setHorizontalHeaderLabels(headers);
    int index = 0;
    for (const auto &coin : m_coins) {
        insertCoin(table, coin, index);
        index++;
    }
    auto *oldTable = m_table;
    m_table = table;
    delete oldTable;
    table->resizeColumnsToContents();
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);

    auto *mainLayout = (new qontrol::Column(this))
                           ->push(m_confirmed_row)
                           ->pushSpacer(V_SPACER)
                           ->push(m_unconfirmed_row)
                           ->pushSpacer(20)
                           ->push(table);

    auto *boxed = margin(mainLayout);
    delete m_main_widget;
    m_main_widget = boxed;

    auto *old = this->layout();
    delete old;
    this->setLayout(boxed->layout());
}

auto Coins::getCoins() -> std::optional<QList<RustCoin>> {
    auto coins = QList<RustCoin>();
    for (const auto &coin : m_coins) {
        coins.append(coin);
    }
    if (coins.isEmpty()) {
        return std::nullopt;
    }
    return std::make_optional(coins);
}

} // namespace screen

#include "Coins.h"
#include "AccountController.h"
#include "utils.h"
#include <Qontrol>
#include <common.h>
#include <cstdint>
#include <optional>
#include <qlabel.h>
#include <qnamespace.h>
#include <qtablewidget.h>
#include <utility>

namespace screen {

Coins::Coins(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

auto Coins::init() -> void {
}

auto Coins::recvPayload(const CoinState &state) -> void {
    // Always update to ensure coin list changes (like label updates) are
    // reflected
    m_state = state;
    this->view();
    emit coinsUpdated();
}

auto Coins::doConnect() -> void {
    auto *ctrl = m_controller;
    connect(ctrl, &AccountController::updateCoins, this, &Coins::recvPayload, qontrol::UNIQUE);
}

auto balanceRow(const QString &label_str, uint64_t balance, uint64_t coins_count) -> QWidget * {
    auto *label = new QLabel(label_str);
    label->setFixedWidth(LABEL_WIDTH);

    auto priceStr = toBitcoin(balance);
    auto *price = new QLabel(priceStr);
    price->setFixedWidth(PRICE_WIDTH);

    auto coinStr = coinsCount(coins_count);
    auto *coins = new QLabel(coinStr);
    coins->setFixedWidth(PRICE_WIDTH);

    auto *row = (new qontrol::Row)->push(label)->push(price)->push(coins)->pushSpacer();

    return row;
}

auto insertCoin(QTableWidget *table, const RustCoin &coin, int index) -> void {
    auto *typeItem =
        new QTableWidgetItem(QString::fromUtf8(coin.account_type.data(), coin.account_type.size()));
    typeItem->setFlags(typeItem->flags() & ~Qt::ItemIsEditable);
    table->setItem(index, 0, typeItem);

    QString blockHeight;
    if (!coin.spent && coin.height > 0) {
        blockHeight = QString::number(coin.height);
    } else if (coin.spent) {
        blockHeight = "Spent";
    } else {
        blockHeight = "Unconfirmed";
    }

    auto *heightItem = new QTableWidgetItem(blockHeight);
    heightItem->setFlags(heightItem->flags() & ~Qt::ItemIsEditable);
    table->setItem(index, 1, heightItem);

    auto op = coin.outpoint;
    auto *outpointItem = new QTableWidgetItem(QString(op.c_str()));
    outpointItem->setFlags(outpointItem->flags() & ~Qt::ItemIsEditable);
    table->setItem(index, 2, outpointItem);

    auto label = coin.label;
    auto *labelItem = new QTableWidgetItem(QString(label.c_str()));
    // Label is editable, keep default flags
    table->setItem(index, 3, labelItem);

    auto *value = new QTableWidgetItem(toBitcoin(coin.value, false));
    value->setTextAlignment(Qt::AlignCenter);
    value->setFlags(value->flags() & ~Qt::ItemIsEditable);
    table->setItem(index, 4, value);
}

auto Coins::view() -> void {
    auto *oldCR = m_confirmed_row;
    m_confirmed_row = balanceRow("Confirmed:", m_state.confirmed_balance, m_state.confirmed_count);
    delete oldCR;

    auto *oldUR = m_unconfirmed_row;
    m_unconfirmed_row =
        balanceRow("Unconfirmed:", m_state.unconfirmed_balance, m_state.unconfirmed_count);
    delete oldUR;

    // Get coins from controller's account
    if (m_controller != nullptr) {
        m_coins = m_controller->getCoins();
    } else {
        m_coins = rust::Vec<RustCoin>();
    }

    int rowCount = static_cast<int>(m_coins.size());
    auto *table = new QTableWidget(rowCount, 5);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto headers = QStringList{"Type", "Block Height", "OutPoint", "Label", "Value"};
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

    // Enable editing only for the Label column (column 2)
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

    // Connect itemChanged signal to handle label edits
    connect(table, &QTableWidget::itemChanged, this, &Coins::onLabelEdited, qontrol::UNIQUE);

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

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
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

auto Coins::onLabelEdited(QTableWidgetItem *item) -> void {
    if (item == nullptr || m_table == nullptr) {
        return;
    }

    // Only allow editing the Label column (column 3)
    if (item->column() != 3) {
        return;
    }

    int row = item->row();
    if (row < 0 || std::cmp_greater_equal(row, m_coins.size())) {
        return;
    }

    // Get the outpoint from column 2
    auto *outpointItem = m_table->item(row, 2);
    if (outpointItem == nullptr) {
        return;
    }

    QString outpoint = outpointItem->text();
    QString newLabel = item->text();

    qDebug() << "Coins::onLabelEdited() - Updating label for" << outpoint << "to" << newLabel;

    // Update the coin label through the controller
    if (m_controller != nullptr) {
        m_controller->updateCoinLabel(outpoint, newLabel);
    }
}

} // namespace screen

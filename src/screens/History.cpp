#include "History.h"
#include "AccountController.h"
#include "theme/Display.h"
#include "theme/Label.h"
#include "utils.h"
#include <Qontrol>
#include <common.h>
#include <silent.h>

namespace screen {

using theme::Display;
using theme::DisplayRole;
using theme::Label;
using theme::LabelRole;

History::History(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

void History::init() {
    int hPad = 7;

    m_h_direction = new Label("Direction", LabelRole::InfoLabel);
    m_h_direction->setFixedWidth(DIR_W);
    m_h_direction->setContentsMargins(hPad, 0, 0, 0);

    m_h_txid = new Label("TxID", LabelRole::InfoLabel);
    m_h_txid->setFixedWidth(TXID_W);
    m_h_txid->setContentsMargins(hPad, 0, 0, 0);

    m_h_height = new Label("Height", LabelRole::InfoLabel);
    m_h_height->setFixedWidth(HEIGHT_W);
    m_h_height->setContentsMargins(hPad, 0, 0, 0);

    m_h_amount = new Label("Amount", LabelRole::InfoLabel);
    m_h_amount->setFixedWidth(AMOUNT_W);
    m_h_amount->setContentsMargins(hPad, 0, 0, 0);
}

void History::recvPayload(const CoinState &state) {
    m_state = state;
    this->view();
}

void History::doConnect() {
    connect(m_controller, &AccountController::updateCoins, this, &History::recvPayload,
            qontrol::UNIQUE);
}

static auto shortenTxid(const QString &txid) -> QString {
    if (txid.length() < 16) {
        return txid;
    }
    return txid.left(6) + "..." + txid.right(6);
}

static auto balanceRow(const QString &label_str, uint64_t balance, uint64_t coins_count)
    -> QWidget * {
    auto *label = new Label(label_str, LabelRole::InfoLabel);

    auto priceStr = toBitcoin(balance);
    auto *price = new Label(priceStr, LabelRole::Mono);

    auto coinStr = coinsCount(coins_count);
    auto *coins = new Label(coinStr);

    auto *row = (new qontrol::Row)->push(label)->push(price)->push(coins)->pushSpacer();

    return row;
}

void History::view() {
    auto *oldCR = m_confirmed_row;
    m_confirmed_row = balanceRow("Confirmed:", m_state.confirmed_balance, m_state.confirmed_count);
    delete oldCR;

    auto *oldUR = m_unconfirmed_row;
    m_unconfirmed_row =
        balanceRow("Unconfirmed:", m_state.unconfirmed_balance, m_state.unconfirmed_count);
    delete oldUR;

    // Unparent persistent header labels
    m_h_direction->setParent(nullptr);
    m_h_txid->setParent(nullptr);
    m_h_height->setParent(nullptr);
    m_h_amount->setParent(nullptr);

    auto *headerRow = (new qontrol::Row)
                          ->push(m_h_direction)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_txid)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_height)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_amount)
                          ->pushSpacer();

    auto *txColumn = new qontrol::Column;
    txColumn->push(headerRow);

    if (m_controller != nullptr) {
        auto txs = m_controller->getPaymentHistory();

        for (const auto &tx : txs) {
            QString dirStr = QString::fromUtf8(tx.direction.data(), tx.direction.size());
            QString displayDir = (dirStr == "incoming") ? "Incoming" : "Outgoing";
            auto *dirField = new Display(displayDir);
            dirField->setFixedWidth(DIR_W);

            QString txid = QString::fromUtf8(tx.txid.data(), tx.txid.size());
            auto *txidField = new Display(shortenTxid(txid), DisplayRole::Outpoint);
            txidField->setFixedWidth(TXID_W);

            QString heightStr = (tx.height > 0) ? QString::number(tx.height) : "Unconfirmed";
            auto *heightField = new Display(heightStr);
            heightField->setFixedWidth(HEIGHT_W);

            auto *amountField = new Display(toBitcoin(tx.amount), DisplayRole::Amount);
            amountField->setFixedWidth(AMOUNT_W);

            auto *row = (new qontrol::Row)
                            ->push(dirField)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(txidField)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(heightField)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(amountField)
                            ->pushSpacer();

            txColumn->pushSpacer(resolve(Spacing::XS))->push(row);
        }
    }
    txColumn->pushSpacer();

    auto *scroll = new QScrollArea;
    scroll->setWidget(txColumn);
    scroll->setWidgetResizable(true);
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    delete m_scroll;
    m_scroll = scroll;

    auto *mainLayout = (new qontrol::Column(this))
                           ->push(m_confirmed_row)
                           ->pushSpacer(resolve(Spacing::XS))
                           ->push(m_unconfirmed_row)
                           ->pushSpacer(resolve(Spacing::M))
                           ->push(scroll);

    auto *boxed = margin(mainLayout);
    delete m_main_widget;
    m_main_widget = boxed;

    auto *old = this->layout();
    delete old;
    this->setLayout(boxed->layout());
}

} // namespace screen

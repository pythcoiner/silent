#include "History.h"
#include "AccountController.h"
#include "i18n/Tr.h"
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

    m_h_direction = new Label(TR("history-direction"), LabelRole::InfoLabel);
    m_h_direction->setFixedWidth(DIR_W);
    m_h_direction->setContentsMargins(hPad, 0, 0, 0);

    m_h_txid = new Label(TR("history-txid"), LabelRole::InfoLabel);
    m_h_txid->setFixedWidth(TXID_W);
    m_h_txid->setContentsMargins(hPad, 0, 0, 0);

    m_h_height = new Label(TR("history-height"), LabelRole::InfoLabel);
    m_h_height->setFixedWidth(HEIGHT_W);
    m_h_height->setContentsMargins(hPad, 0, 0, 0);

    m_h_amount = new Label(TR("history-amount"), LabelRole::InfoLabel);
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
    m_confirmed_row =
        balanceRow(TR("history-confirmed"), m_state.confirmed_balance, m_state.confirmed_count);
    delete oldCR;

    auto *oldUR = m_unconfirmed_row;
    m_unconfirmed_row =
        balanceRow(TR("history-unconfirmed"), m_state.unconfirmed_balance, m_state.unconfirmed_count);
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
            QString displayDir = (dirStr == "incoming") ? TR("history-incoming") : TR("history-outgoing");
            auto *dirField = new Display(displayDir);
            dirField->setFixedWidth(DIR_W);

            QString txid = QString::fromUtf8(tx.txid.data(), tx.txid.size());
            auto *txidField = new Display(shortenTxid(txid), DisplayRole::Outpoint);
            txidField->setFixedWidth(TXID_W);

            QString heightStr = (tx.height > 0) ? QString::number(tx.height) : TR("history-unconfirmed-no-colon");
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

void History::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        view();
    }
    qontrol::Screen::changeEvent(event);
}

void History::retranslateUi() {
    if (m_h_direction != nullptr) {
        m_h_direction->setText(TR("history-direction"));
    }
    if (m_h_txid != nullptr) {
        m_h_txid->setText(TR("history-txid"));
    }
    if (m_h_height != nullptr) {
        m_h_height->setText(TR("history-height"));
    }
    if (m_h_amount != nullptr) {
        m_h_amount->setText(TR("history-amount"));
    }
}

} // namespace screen

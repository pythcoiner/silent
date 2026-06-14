#include "History.h"
#include "AccountController.h"
#include "catalog/BalanceHeader.h"
#include "catalog/panels/home/HistoryTable.h"
#include "i18n/Tr.h"
#include "views/utils.h"
#include <Qontrol>
#include <common.h>
#include <silent.h>

namespace view {

History::History(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

void History::init() {
}

void History::onRecvPayload(const CoinState &state) {
    m_state = state;
    this->view();
}

void History::doConnect() {
    connect(m_controller, &AccountController::updateCoins, this, &History::onRecvPayload,
            qontrol::UNIQUE);
}

void History::view() {
    if (m_controller == nullptr) {
        return;
    }

    m_balance = new catalog::BalanceHeader;
    m_balance->setConfirmed(toBitcoin(m_state.confirmed_balance, false));
    m_balance->setUnconfirmed(m_state.unconfirmed_balance);
    m_balance->setCoinCount(m_state.confirmed_count + m_state.unconfirmed_count);

    m_table = new catalog::HistoryTable;
    auto txs = m_controller->getPaymentHistory();
    for (const auto &tx : txs) {
        QString direction = QString::fromUtf8(tx.direction.data(), tx.direction.size());
        QString txid = QString::fromUtf8(tx.txid.data(), tx.txid.size());
        m_table->addEntry(direction, txid, tx.height, tx.amount);
    }

    auto *mainLayout = (new qontrol::Column(this))
                           ->push(m_balance)
                           ->push(m_table);

    auto *boxed = dashboard(TR("history-title"), mainLayout);
    setScreenContent(this, m_main_widget, boxed);
}

void History::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        view();
    }
    qontrol::Screen::changeEvent(event);
}

void History::retranslateUi() {
}

} // namespace view

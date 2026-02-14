#include "ConfirmSend.h"
#include "../utils.h"
#include <Qontrol>
#include <common.h>
#include <qlabel.h>
#include <qpushbutton.h>

namespace modal {

ConfirmSend::ConfirmSend(const QStringList &recipients, uint64_t fee,
                         const QString &txid_preview)
    : m_recipients(recipients), m_fee(fee), m_txid_preview(txid_preview) {
    setWindowTitle("Confirm Transaction");
    resize(500, 300);
    init();
    doConnect();
    view();
}

auto ConfirmSend::init() -> void {
    QString details;
    for (const auto &line : m_recipients) {
        details += line + "\n";
    }
    details += "\nFee: " + toBitcoin(m_fee);
    details += "\nTxid: " + m_txid_preview;

    m_details_label = new QLabel(details);
    m_details_label->setWordWrap(true);
    m_details_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_cancel_btn = new QPushButton("Cancel");
    m_confirm_btn = new QPushButton("Confirm");
}

auto ConfirmSend::doConnect() -> void {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(m_confirm_btn, &QPushButton::clicked, this, &ConfirmSend::onConfirmClicked,
            qontrol::UNIQUE);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto ConfirmSend::onConfirmClicked() -> void {
    emit confirmed();
    accept();
}

auto ConfirmSend::view() -> void {
    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(H_SPACER)
                          ->push(m_confirm_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(20)
                    ->push(m_details_label)
                    ->pushSpacer(20)
                    ->push(buttonRow)
                    ->pushSpacer();

    setMainWidget(margin(col));
}

} // namespace modal

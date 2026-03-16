#include "ConfirmSend.h"
#include "../utils.h"
#include "theme/Button.h"
#include "theme/Label.h"
#include <Qontrol>
#include <common.h>

namespace modal {

using theme::Button;
using theme::ButtonRole;
using theme::Label;
using theme::LabelRole;

ConfirmSend::ConfirmSend(const QStringList &recipients, uint64_t fee, const QString &txid_preview)
    : m_recipients(recipients),
      m_fee(fee),
      m_txid_preview(txid_preview) {
    setWindowTitle("Confirm Transaction");
    resize(500, 300);
    init();
    doConnect();
    view();
}

void ConfirmSend::init() {
    QString details;
    for (const auto &line : m_recipients) {
        details += line + "\n";
    }
    details += "\nFee: " + toBitcoin(m_fee);
    details += "\nTxid: " + m_txid_preview;

    m_details_label = new Label(details);
    m_details_label->setWordWrap(true);
    m_details_label->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_cancel_btn = new Button("Cancel");
    m_confirm_btn = new Button("Confirm", ButtonRole::Primary);

    m_status_label = new Label("Broadcasting...");
    m_status_label->setAlignment(Qt::AlignCenter);
    m_status_label->setWordWrap(true);
    m_status_label->setVisible(false);

    m_ok_btn = new Button("OK", ButtonRole::Primary);
    m_ok_btn->setVisible(false);
}

void ConfirmSend::doConnect() {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(m_confirm_btn, &QPushButton::clicked, this, &ConfirmSend::onConfirmClicked,
            qontrol::UNIQUE);
    connect(m_ok_btn, &QPushButton::clicked, this, &QDialog::accept, qontrol::UNIQUE);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onConfirmClicked() {
    setBroadcasting();
    emit confirmed();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::setBroadcasting() {
    m_cancel_btn->setVisible(false);
    m_confirm_btn->setVisible(false);
    m_status_label->setVisible(true);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::setResult(bool ok, const QString &message) {
    m_details_label->setVisible(false);
    if (ok) {
        m_status_label->setText("Transaction Sent\n\nTxid: " + message);
    } else {
        m_status_label->setText("Broadcast Failed\n\n" + message);
    }
    m_status_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_ok_btn->setVisible(true);
}

void ConfirmSend::view() {
    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_confirm_btn)
                          ->pushSpacer();

    auto *okRow = (new qontrol::Row)->pushSpacer()->push(m_ok_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_details_label)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_status_label)
                    ->push(buttonRow)
                    ->push(okRow)
                    ->pushSpacer();

    setMainWidget(margin(col));
}

} // namespace modal

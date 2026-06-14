#include "ConfirmSend.h"
#include "SelectSigner.h"
#include "../utils.h"
#include "i18n/Tr.h"
#include "catalog/Button.h"
#include "catalog/containers/Card.h"
#include "catalog/display/Label.h"
#include "catalog/feedback/ModalStatus.h"
#include "catalog/containers/Separator.h"
#include <Qontrol>
#include <common.h>

namespace modal {

using catalog::Button;
using catalog::ButtonRole;
using catalog::Label;
using catalog::LabelRole;

ConfirmSend::ConfirmSend(const QStringList &recipients, uint64_t fee, const QString &txid_preview)
    : m_recipients(recipients),
      m_fee(fee),
      m_txid_preview(txid_preview) {
    setWindowTitle(TR("confirm-transaction-title"));
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
    m_summary_widget = new QWidget;
    m_recipients_label = new Label(details.trimmed(), LabelRole::Mono);
    m_recipients_label->setWordWrap(true);
    m_recipients_label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_fee_label = new Label(TR("confirm-send-fee").arg(toBitcoin(m_fee)), LabelRole::Mono);
    m_txid_label = new Label(TR("confirm-send-txid").arg(m_txid_preview), LabelRole::Mono);

    m_cancel_btn = new Button(TR("common-cancel"));
    m_confirm_btn = new Button(TR("common-confirm"), ButtonRole::Primary);
    m_broadcast_btn = new Button(TR("common-broadcast"), ButtonRole::Primary);
    m_broadcast_btn->setVisible(false);

    m_status = new catalog::ModalStatus;
    m_status->setState(catalog::ModalStatus::State::Review);
    m_status->setVisible(false);

    m_ok_btn = new Button(TR("common-ok"), ButtonRole::Primary);
    m_ok_btn->setVisible(false);
}

void ConfirmSend::doConnect() {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(m_confirm_btn, &QPushButton::clicked, this, &ConfirmSend::onConfirmClicked,
            qontrol::UNIQUE);
    connect(m_broadcast_btn, &QPushButton::clicked, this, &ConfirmSend::onBroadcastClicked,
            qontrol::UNIQUE);
    connect(m_ok_btn, &QPushButton::clicked, this, &QDialog::accept, qontrol::UNIQUE);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onConfirmClicked() {
    auto *modal = new SelectSigner(TR("send-select-signer"));
    int result = modal->exec();
    delete modal;
    if (result != QDialog::Accepted) {
        return;
    }
    onSetSigning();
    emit signRequested();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onBroadcastClicked() {
    onSetBroadcasting();
    emit broadcastRequested();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onSetSigning() {
    m_cancel_btn->setVisible(false);
    m_confirm_btn->setVisible(false);
    m_broadcast_btn->setVisible(false);
    m_status->setState(catalog::ModalStatus::State::Signing);
    m_status->setSubtitle(TR("confirm-send-signing"));
    m_status->setVisible(true);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onSetSigned() {
    m_status->setState(catalog::ModalStatus::State::Success);
    m_status->setSubtitle(TR("confirm-send-signed"));
    m_status->setVisible(true);
    m_broadcast_btn->setVisible(true);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onSetBroadcasting() {
    m_broadcast_btn->setVisible(false);
    m_status->setState(catalog::ModalStatus::State::Broadcasting);
    m_status->setSubtitle(TR("confirm-send-broadcasting"));
    m_status->setVisible(true);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void ConfirmSend::onSetResult(bool ok, const QString &message) {
    m_summary_widget->setVisible(false);
    if (ok) {
        m_status->setState(catalog::ModalStatus::State::Success);
        m_status->setSubtitle(TR("confirm-send-success").arg(message));
    } else {
        m_status->setState(catalog::ModalStatus::State::Error);
        m_status->setSubtitle(TR("confirm-send-failed").arg(message));
    }
    m_ok_btn->setVisible(true);
}

void ConfirmSend::view() {
    auto *recipientsCardContent = (new qontrol::Column)
                                      ->push(new Label(TR("send-recipients"), LabelRole::Section))
                                      ->pushSpacer(resolve(Spacing::XS))
                                      ->push(m_recipients_label);
    auto *recipientsCard = new catalog::Card(catalog::Card::Role::Inset);
    recipientsCard->setContent(recipientsCardContent);

    auto *metaContent = (new qontrol::Column)
                            ->push(m_fee_label)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(new catalog::Separator)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_txid_label);
    auto *metaCard = new catalog::Card(catalog::Card::Role::Inset);
    metaCard->setContent(metaContent);

    auto *summaryLayout = new qontrol::Column;
    summaryLayout->push(recipientsCard)->pushSpacer(resolve(Spacing::S))->push(metaCard);
    m_summary_widget->setLayout(summaryLayout->layout());

    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_confirm_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_broadcast_btn)
                          ->pushSpacer();

    auto *okRow = (new qontrol::Row)->pushSpacer()->push(m_ok_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_summary_widget)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_status)
                    ->push(buttonRow)
                    ->push(okRow)
                    ->pushSpacer();

    setMainWidget(margin(col));
}

void ConfirmSend::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    qontrol::Modal::changeEvent(event);
}

void ConfirmSend::retranslateUi() {
    setWindowTitle(TR("confirm-transaction-title"));

    QString details;
    for (const auto &line : m_recipients) {
        details += line + "\n";
    }
    m_recipients_label->setText(details.trimmed());
    m_fee_label->setText(TR("confirm-send-fee").arg(toBitcoin(m_fee)));
    m_txid_label->setText(TR("confirm-send-txid").arg(m_txid_preview));

    m_cancel_btn->setText(TR("common-cancel"));
    m_confirm_btn->setText(TR("common-confirm"));
    m_broadcast_btn->setText(TR("common-broadcast"));
    m_ok_btn->setText(TR("common-ok"));

    if (m_status->isVisible() && !m_ok_btn->isVisible()) {
        if (m_broadcast_btn->isVisible()) {
            m_status->setSubtitle(TR("confirm-send-signed"));
        } else {
            m_status->setSubtitle(TR("confirm-send-broadcasting"));
        }
    }
}

} // namespace modal

#include "ConfirmDelete.h"
#include "i18n/Tr.h"
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

ConfirmDelete::ConfirmDelete(const QString &name, [[maybe_unused]] QWidget *parent) : m_name(name) {
    setWindowTitle(TR("delete-wallet-title"));
    resize(400, 150);
    init();
    doConnect();
    view();
}

void ConfirmDelete::init() {
    m_label =
        new Label(TR("delete-wallet-confirmation").arg(m_name));
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignCenter);

    m_cancel_btn = new Button(TR("common-cancel"));
    m_delete_btn = new Button(TR("common-delete"), ButtonRole::Destructive);
}

void ConfirmDelete::doConnect() {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(m_delete_btn, &QPushButton::clicked, this, &ConfirmDelete::onDeleteClicked,
            qontrol::UNIQUE);
}

void ConfirmDelete::onDeleteClicked() {
    emit confirmed(m_name);
    accept();
}

void ConfirmDelete::view() {
    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_delete_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_label)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(buttonRow)
                    ->pushSpacer();

    setMainWidget(margin(col));
}

void ConfirmDelete::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    qontrol::Modal::changeEvent(event);
}

void ConfirmDelete::retranslateUi() {
    setWindowTitle(TR("delete-wallet-title"));
    m_label->setText(TR("delete-wallet-confirmation").arg(m_name));
    m_cancel_btn->setText(TR("common-cancel"));
    m_delete_btn->setText(TR("common-delete"));
}

} // namespace modal

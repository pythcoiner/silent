#include "ConfirmDelete.h"
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
    setWindowTitle("Delete Wallet");
    resize(400, 150);
    init();
    doConnect();
    view();
}

void ConfirmDelete::init() {
    m_label = new Label(QString("Are you sure you want to delete wallet '%1'?\n"
                                "This action cannot be undone.")
                            .arg(m_name));
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignCenter);

    m_cancel_btn = new Button("Cancel");
    m_delete_btn = new Button("Delete", ButtonRole::Destructive);
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

} // namespace modal

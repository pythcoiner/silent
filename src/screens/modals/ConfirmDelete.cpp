#include "ConfirmDelete.h"
#include "../utils.h"
#include <Qontrol>
#include <common.h>
#include <qlabel.h>
#include <qpushbutton.h>

ConfirmDelete::ConfirmDelete(const QString &name, QWidget *parent) : m_name(name) {
    Q_UNUSED(parent);
    setWindowTitle("Delete Wallet");
    resize(400, 150);
    init();
    doConnect();
    view();
}

auto ConfirmDelete::init() -> void {
    m_label = new QLabel(QString("Are you sure you want to delete wallet '%1'?\n"
                                 "This action cannot be undone.")
                             .arg(m_name));
    m_label->setWordWrap(true);
    m_label->setAlignment(Qt::AlignCenter);

    m_cancel_btn = new QPushButton("Cancel");
    m_delete_btn = new QPushButton("Delete");
}

auto ConfirmDelete::doConnect() -> void {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(m_delete_btn, &QPushButton::clicked, this, &ConfirmDelete::onDeleteClicked,
            qontrol::UNIQUE);
}

auto ConfirmDelete::onDeleteClicked() -> void {
    emit confirmed(m_name);
    accept();
}

auto ConfirmDelete::view() -> void {
    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(H_SPACER)
                          ->push(m_delete_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(20)
                    ->push(m_label)
                    ->pushSpacer(20)
                    ->push(buttonRow)
                    ->pushSpacer();

    setMainWidget(margin(col));
}

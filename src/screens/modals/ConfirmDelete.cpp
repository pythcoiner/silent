#include "ConfirmDelete.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>

ConfirmDelete::ConfirmDelete(const QString &name, QWidget *parent)
    : qontrol::Modal(), m_name(name) {
    Q_UNUSED(parent);
    setWindowTitle("Delete Wallet");
    resize(400, 150);
    initUI();
}

void ConfirmDelete::initUI() {
    auto *layout = new QVBoxLayout();

    auto *label = new QLabel(
        QString("Are you sure you want to delete wallet '%1'?\n"
                "This action cannot be undone.")
            .arg(m_name));
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    layout->addSpacing(20);

    auto *button_layout = new QHBoxLayout();
    auto *cancel_btn = new QPushButton("Cancel");
    auto *delete_btn = new QPushButton("Delete");

    button_layout->addStretch();
    button_layout->addWidget(cancel_btn);
    button_layout->addWidget(delete_btn);

    layout->addLayout(button_layout);

    auto *widget = new QWidget();
    widget->setLayout(layout);
    setMainWidget(widget);

    connect(cancel_btn, &QPushButton::clicked, this, &QDialog::reject);
    connect(delete_btn, &QPushButton::clicked, this, [this]() {
        emit confirmed(m_name);
        accept();
    });
}

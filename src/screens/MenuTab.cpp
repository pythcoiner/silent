#include "MenuTab.h"
#include "AppController.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>

MenuTab::MenuTab(QWidget *parent) : QWidget(parent) {
    initUI();
}

void MenuTab::initUI() {
    auto *layout = new QVBoxLayout(this);

    // Add spacer at top
    layout->addStretch();

    // Add title label
    auto *title = new QLabel("Templar - Silent Payments Wallet");
    auto title_font = title->font();
    title_font.setPointSize(24);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    layout->addSpacing(40);

    // Add create account button
    auto *create_btn = new QPushButton("Create Account");
    create_btn->setFixedWidth(300);
    create_btn->setFixedHeight(60);
    auto btn_font = create_btn->font();
    btn_font.setPointSize(16);
    create_btn->setFont(btn_font);

    // Center the button horizontally
    auto *btn_layout = new QHBoxLayout();
    btn_layout->addStretch();
    btn_layout->addWidget(create_btn);
    btn_layout->addStretch();
    layout->addLayout(btn_layout);

    // Add spacer at bottom
    layout->addStretch();

    setLayout(layout);

    // Connect button to AppController
    connect(create_btn, &QPushButton::clicked, this, [this]() {
        AppController::get()->onCreateAccount();
    });
}

#include "MenuTab.h"
#include "AppController.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>

MenuTab::MenuTab(QWidget *parent) : QWidget(parent) {
    initUI();
    connect(AppController::get(), &AppController::accountList,
            this, &MenuTab::onAccountList, qontrol::UNIQUE);
}

void MenuTab::initUI() {
    auto *layout = new QVBoxLayout(this);

    // Add spacer at top
    layout->addStretch();

    // Add title label
    auto *title = new QLabel("Silent - Silent Payments Wallet");
    auto title_font = title->font();
    title_font.setPointSize(24);
    title_font.setBold(true);
    title->setFont(title_font);
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    layout->addSpacing(40);

    // Account list section
    m_accountListLayout = new QVBoxLayout();
    m_accountListLayout->setAlignment(Qt::AlignHCenter);
    layout->addLayout(m_accountListLayout);

    layout->addSpacing(20);

    // Add create account button
    auto *create_btn = new QPushButton("+ Create New Wallet");
    create_btn->setFixedWidth(300);
    create_btn->setFixedHeight(50);
    auto btn_font = create_btn->font();
    btn_font.setPointSize(14);
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

void MenuTab::clearAccountButtons() {
    for (auto *btn : m_accountButtons) {
        m_accountListLayout->removeWidget(btn);
        btn->deleteLater();
    }
    m_accountButtons.clear();
}

void MenuTab::onAccountList(const QList<QString> &accounts) {
    clearAccountButtons();

    if (accounts.isEmpty()) {
        return;
    }

    for (const auto &name : accounts) {
        auto *btn = new QPushButton(name);
        btn->setFixedWidth(300);
        btn->setFixedHeight(50);
        auto font = btn->font();
        font.setPointSize(14);
        btn->setFont(font);

        connect(btn, &QPushButton::clicked, this, [name]() {
            AppController::get()->openAccount(name);
        });

        m_accountListLayout->addWidget(btn, 0, Qt::AlignHCenter);
        m_accountButtons.append(btn);
    }
}

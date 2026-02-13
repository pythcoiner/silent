#include "MenuTab.h"
#include "AppController.h"
#include <Qontrol>
#include <common.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstyle.h>

MenuTab::MenuTab(QWidget *parent) : QWidget(parent) {
    init();
    doConnect();
    view();
}

auto MenuTab::init() -> void {
    m_create_btn = new QPushButton("+ Create New Wallet");
    m_create_btn->setFixedWidth(354);
    m_create_btn->setFixedHeight(50);
    auto btnFont = m_create_btn->font();
    btnFont.setPointSize(14);
    m_create_btn->setFont(btnFont);

    m_accounts_column = new qontrol::Column;
}

auto MenuTab::doConnect() -> void {
    connect(m_create_btn, &QPushButton::clicked, this,
            []() -> void { AppController::get()->onCreateAccount(); });
    connect(AppController::get(), &AppController::accountList, this, &MenuTab::onAccountList,
            qontrol::UNIQUE);
}

auto MenuTab::view() -> void {
    auto *title = new QLabel("Silent - Silent Payments Wallet");
    auto titleFont = title->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);

    auto *titleRow = (new qontrol::Row)->pushSpacer()->push(title)->pushSpacer();
    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_create_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer()
                    ->push(titleRow)
                    ->pushSpacer(40)
                    ->push(m_accounts_column)
                    ->pushSpacer(20)
                    ->push(btnRow)
                    ->pushSpacer();

    setLayout(col->layout());
}

auto MenuTab::clearAccountButtons() -> void {
    m_accounts_column->clear();
    m_account_rows.clear();
}

auto MenuTab::onAccountList(const QList<QString> &accounts) -> void {
    clearAccountButtons();

    if (accounts.isEmpty()) {
        return;
    }

    for (const auto &name : accounts) {
        auto *btn = new QPushButton(name);
        btn->setFixedWidth(300);
        btn->setFixedHeight(50);
        auto btnFont = btn->font();
        btnFont.setPointSize(14);
        btn->setFont(btnFont);

        auto *trashBtn = new QPushButton();
        trashBtn->setFixedSize(50, 50);
        trashBtn->setIcon(trashBtn->style()->standardIcon(QStyle::SP_TrashIcon));
        trashBtn->setToolTip("Delete wallet");

        connect(btn, &QPushButton::clicked, this,
                [name]() -> void { AppController::get()->openAccount(name); });
        connect(trashBtn, &QPushButton::clicked, this,
                [name]() -> void { AppController::get()->deleteAccount(name); });

        auto *row = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(btn)
                        ->pushSpacer(4)
                        ->push(trashBtn)
                        ->pushSpacer();

        if (AppController::get()->isAccountOpen(name)) {
            row->setEnabled(false);
        }

        m_accounts_column->push(row);
        m_account_rows.append(row);
    }
}

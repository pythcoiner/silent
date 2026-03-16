#include "MenuTab.h"
#include "AppController.h"
#include "theme/Button.h"
#include "theme/Icon.h"
#include "theme/Label.h"
#include "theme/Palette.h"
#include <Qontrol>
#include <common.h>

using theme::Button;
using theme::ButtonRole;
using theme::Label;
using theme::LabelRole;

MenuTab::MenuTab(QWidget *parent) : QWidget(parent) {
    init();
    doConnect();
    view();
}

void MenuTab::init() {
    m_create_btn = new Button("+ Create New Wallet", ButtonRole::TabCreate);

    m_accounts_column = new qontrol::Column;
}

void MenuTab::doConnect() {
    connect(m_create_btn, &QPushButton::clicked, this, &MenuTab::onCreateClicked, qontrol::UNIQUE);
    connect(AppController::get(), &AppController::accountList, this, &MenuTab::onAccountList,
            qontrol::UNIQUE);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void MenuTab::onCreateClicked() {
    AppController::get()->onCreateAccount();
}

void MenuTab::onAccountClicked() {
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn != nullptr) {
        auto name = btn->property("accountName").toString();
        AppController::get()->openAccount(name);
    }
}

void MenuTab::onTrashClicked() {
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn != nullptr) {
        auto name = btn->property("accountName").toString();
        AppController::get()->deleteAccount(name);
    }
}

void MenuTab::view() {
    auto *title = new Label("Silent - Silent Payments Wallet", LabelRole::Title);
    title->setAlignment(Qt::AlignCenter);

    auto *titleRow = (new qontrol::Row)->pushSpacer()->push(title)->pushSpacer();
    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_create_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer()
                    ->push(titleRow)
                    ->pushSpacer(resolve(Spacing::XL))
                    ->push(m_accounts_column)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(btnRow)
                    ->pushSpacer();

    setLayout(col->layout());
}

void MenuTab::clearAccountButtons() {
    m_accounts_column->clear();
    m_account_rows.clear();
}

void MenuTab::onAccountList(const QList<QString> &accounts) {
    clearAccountButtons();

    if (accounts.isEmpty()) {
        return;
    }

    for (const auto &name : accounts) {
        auto *btn = new Button(name, ButtonRole::TabOpen);

        auto *trashBtn = new Button(ButtonRole::Icon);
        trashBtn->setIcon(icon::trash());
        trashBtn->setToolTip("Delete wallet");

        btn->setProperty("accountName", name);
        connect(btn, &QPushButton::clicked, this, &MenuTab::onAccountClicked, qontrol::UNIQUE);

        trashBtn->setProperty("accountName", name);
        connect(trashBtn, &QPushButton::clicked, this, &MenuTab::onTrashClicked, qontrol::UNIQUE);

        auto *row = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(btn)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(trashBtn)
                        ->pushSpacer();

        if (AppController::get()->isAccountOpen(name)) {
            row->setEnabled(false);
        }

        if (!m_account_rows.isEmpty()) {
            m_accounts_column->pushSpacer(resolve(Spacing::S));
        }

        m_accounts_column->push(row);
        m_account_rows.append(row);
    }
}

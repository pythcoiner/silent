#include "Receive.h"
#include "AccountController.h"
#include "i18n/Tr.h"
#include "theme/Button.h"
#include "theme/Label.h"
#include "utils.h"
#include <QApplication>
#include <QClipboard>
#include <Qontrol>
#include <common.h>

namespace screen {

using theme::Button;
using theme::Label;
using theme::LabelRole;

Receive::Receive(AccountController *ctrl)
    : m_controller(ctrl)
    , m_sp_address("") {
    this->init();
    this->doConnect();
    this->view();
}

void Receive::init() {
    // Get SP address from controller
    if (m_controller != nullptr) {
        m_sp_address = m_controller->getSpAddress();
        m_has_sub_accounts = m_controller->getAccount().has_value() &&
                             m_controller->getAccount().value()->has_sub_accounts();
    }
    m_copy_btn = new Button;

    if (m_has_sub_accounts) {
        m_segwit_addr_label = new Label("", LabelRole::Mono);
        m_segwit_addr_label->setWordWrap(true);

        m_new_segwit_btn = new Button;
        m_copy_segwit_btn = new Button;

        m_taproot_addr_label = new Label("", LabelRole::Mono);
        m_taproot_addr_label->setWordWrap(true);

        m_new_taproot_btn = new Button;
        m_copy_taproot_btn = new Button;
    }

    retranslateUi();
}

void Receive::doConnect() {
    connect(m_copy_btn, &QPushButton::clicked, this, &Receive::onCopyAddress, qontrol::UNIQUE);

    if (m_has_sub_accounts) {
        connect(m_new_segwit_btn, &QPushButton::clicked, this, &Receive::onNewSegwitAddr,
                qontrol::UNIQUE);
        connect(m_copy_segwit_btn, &QPushButton::clicked, this, &Receive::onCopySegwitAddr,
                qontrol::UNIQUE);
        connect(m_new_taproot_btn, &QPushButton::clicked, this, &Receive::onNewTaprootAddr,
                qontrol::UNIQUE);
        connect(m_copy_taproot_btn, &QPushButton::clicked, this, &Receive::onCopyTaprootAddr,
                qontrol::UNIQUE);
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Receive::onCopyAddress() {
    QApplication::clipboard()->setText(QString(m_sp_address.c_str()));
}

void Receive::onNewSegwitAddr() {
    if (m_controller == nullptr) {
        return;
    }
    auto &account = m_controller->getAccount();
    if (!account.has_value()) {
        return;
    }
    auto addr = account.value()->new_segwit_addr();
    m_segwit_addr_label->setText(QString::fromStdString(std::string(addr.c_str())));
}

void Receive::onCopySegwitAddr() {
    if (m_segwit_addr_label != nullptr) {
        QApplication::clipboard()->setText(m_segwit_addr_label->text());
    }
}

void Receive::onNewTaprootAddr() {
    if (m_controller == nullptr) {
        return;
    }
    auto &account = m_controller->getAccount();
    if (!account.has_value()) {
        return;
    }
    auto addr = account.value()->new_taproot_addr();
    m_taproot_addr_label->setText(QString::fromStdString(std::string(addr.c_str())));
}

void Receive::onCopyTaprootAddr() {
    if (m_taproot_addr_label != nullptr) {
        QApplication::clipboard()->setText(m_taproot_addr_label->text());
    }
}

void Receive::view() {
    auto *addrLabel = new Label(TR("receive-silent-payment-address"), LabelRole::InfoLabel);

    // Display the SP address (large, copyable)
    auto addrQStr = QString(m_sp_address.c_str());
    auto *addrDisplay = new Label(addrQStr, LabelRole::Mono);
    addrDisplay->setWordWrap(true);

    auto *addrRow = (new qontrol::Row)->push(addrLabel)->push(addrDisplay)->pushSpacer();

    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_copy_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::XXL))
                    ->push(addrRow)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(btnRow);

    if (m_has_sub_accounts) {
        // Segwit address section
        auto *segwitLabel = new Label(TR("receive-segwit-address"), LabelRole::InfoLabel);

        auto *segwitAddrRow =
            (new qontrol::Row)->push(segwitLabel)->push(m_segwit_addr_label)->pushSpacer();

        auto *segwitBtnRow = (new qontrol::Row)
                                 ->pushSpacer()
                                 ->push(m_new_segwit_btn)
                                 ->pushSpacer(resolve(Spacing::XS))
                                 ->push(m_copy_segwit_btn)
                                 ->pushSpacer();

        col->pushSpacer(resolve(Spacing::L))
            ->push(segwitAddrRow)
            ->pushSpacer(resolve(Spacing::S))
            ->push(segwitBtnRow);

        // Taproot address section
        auto *taprootLabel = new Label(TR("receive-taproot-address"), LabelRole::InfoLabel);

        auto *taprootAddrRow =
            (new qontrol::Row)->push(taprootLabel)->push(m_taproot_addr_label)->pushSpacer();

        auto *taprootBtnRow = (new qontrol::Row)
                                  ->pushSpacer()
                                  ->push(m_new_taproot_btn)
                                  ->pushSpacer(resolve(Spacing::XS))
                                  ->push(m_copy_taproot_btn)
                                  ->pushSpacer();

        col->pushSpacer(resolve(Spacing::L))
            ->push(taprootAddrRow)
            ->pushSpacer(resolve(Spacing::S))
            ->push(taprootBtnRow);
    }

    col->pushSpacer();

    auto *oldWidget = m_main_widget;
    m_main_widget = margin(col);
    delete oldWidget;
    auto *oldLayout = this->layout();
    delete oldLayout;
    this->setLayout(m_main_widget->layout());
}

void Receive::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
        view();
    }
    qontrol::Screen::changeEvent(event);
}

void Receive::retranslateUi() {
    if (m_copy_btn != nullptr) {
        m_copy_btn->setText(TR("common-copy"));
    }
    if (m_new_segwit_btn != nullptr) {
        m_new_segwit_btn->setText(TR("common-generate"));
    }
    if (m_copy_segwit_btn != nullptr) {
        m_copy_segwit_btn->setText(TR("common-copy"));
    }
    if (m_new_taproot_btn != nullptr) {
        m_new_taproot_btn->setText(TR("common-generate"));
    }
    if (m_copy_taproot_btn != nullptr) {
        m_copy_taproot_btn->setText(TR("common-copy"));
    }
}

} // namespace screen

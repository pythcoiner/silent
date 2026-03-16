#include "Receive.h"
#include "AccountController.h"
#include "utils.h"
#include <QApplication>
#include <QClipboard>
#include <Qontrol>
#include <common.h>
#include <qlabel.h>
#include <qpushbutton.h>

namespace screen {

Receive::Receive(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

auto Receive::init() -> void {
    // Get SP address from controller
    if (m_controller != nullptr) {
        m_sp_address = m_controller->getSpAddress();
        m_has_sub_accounts = m_controller->getAccount().has_value() &&
                             m_controller->getAccount().value()->has_sub_accounts();
    }
    m_copy_btn = new QPushButton("Copy");

    if (m_has_sub_accounts) {
        m_segwit_addr_display = new QLabel("");
        m_segwit_addr_display->setFixedWidth(600);
        m_segwit_addr_display->setWordWrap(true);

        m_new_segwit_btn = new QPushButton("Generate");
        m_copy_segwit_btn = new QPushButton("Copy");

        m_taproot_addr_display = new QLabel("");
        m_taproot_addr_display->setFixedWidth(600);
        m_taproot_addr_display->setWordWrap(true);

        m_new_taproot_btn = new QPushButton("Generate");
        m_copy_taproot_btn = new QPushButton("Copy");
    }
}

auto Receive::doConnect() -> void {
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

auto Receive::onCopyAddress() -> void {
    QApplication::clipboard()->setText(QString(m_sp_address.c_str()));
}

auto Receive::onNewSegwitAddr() -> void {
    if (m_controller == nullptr) {
        return;
    }
    auto &account = m_controller->getAccount();
    if (!account.has_value()) {
        return;
    }
    auto addr = account.value()->new_segwit_addr();
    m_segwit_addr_display->setText(QString::fromStdString(std::string(addr.c_str())));
}

auto Receive::onCopySegwitAddr() -> void {
    if (m_segwit_addr_display != nullptr) {
        QApplication::clipboard()->setText(m_segwit_addr_display->text());
    }
}

auto Receive::onNewTaprootAddr() -> void {
    if (m_controller == nullptr) {
        return;
    }
    auto &account = m_controller->getAccount();
    if (!account.has_value()) {
        return;
    }
    auto addr = account.value()->new_taproot_addr();
    m_taproot_addr_display->setText(QString::fromStdString(std::string(addr.c_str())));
}

auto Receive::onCopyTaprootAddr() -> void {
    if (m_taproot_addr_display != nullptr) {
        QApplication::clipboard()->setText(m_taproot_addr_display->text());
    }
}

auto Receive::view() -> void {
    auto *addrLabel = new QLabel("Silent Payment Address:");
    addrLabel->setFixedWidth(200);

    // Display the SP address (large, copyable)
    auto addrQStr = QString(m_sp_address.c_str());
    auto *addrDisplay = new QLabel(addrQStr);
    addrDisplay->setFixedWidth(600);
    addrDisplay->setWordWrap(true);
    QFont f = addrDisplay->font();
    f.setPointSize(f.pointSize() + 2);
    addrDisplay->setFont(f);

    auto *addrRow = (new qontrol::Row)->push(addrLabel)->push(addrDisplay)->pushSpacer();

    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_copy_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(50)
                    ->push(addrRow)
                    ->pushSpacer(20)
                    ->push(btnRow);

    if (m_has_sub_accounts) {
        // Segwit address section
        auto *segwitLabel = new QLabel("Segwit Address:");
        segwitLabel->setFixedWidth(200);

        auto *segwitAddrRow =
            (new qontrol::Row)->push(segwitLabel)->push(m_segwit_addr_display)->pushSpacer();

        auto *segwitBtnRow = (new qontrol::Row)
                                 ->pushSpacer()
                                 ->push(m_new_segwit_btn)
                                 ->pushSpacer(H_SPACER)
                                 ->push(m_copy_segwit_btn)
                                 ->pushSpacer();

        col->pushSpacer(30)
            ->push(segwitAddrRow)
            ->pushSpacer(10)
            ->push(segwitBtnRow);

        // Taproot address section
        auto *taprootLabel = new QLabel("Taproot Address:");
        taprootLabel->setFixedWidth(200);

        auto *taprootAddrRow =
            (new qontrol::Row)->push(taprootLabel)->push(m_taproot_addr_display)->pushSpacer();

        auto *taprootBtnRow = (new qontrol::Row)
                                  ->pushSpacer()
                                  ->push(m_new_taproot_btn)
                                  ->pushSpacer(H_SPACER)
                                  ->push(m_copy_taproot_btn)
                                  ->pushSpacer();

        col->pushSpacer(30)
            ->push(taprootAddrRow)
            ->pushSpacer(10)
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

} // namespace screen

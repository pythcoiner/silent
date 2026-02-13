#include "Receive.h"
#include "AccountController.h"
#include "utils.h"
#include <QApplication>
#include <QClipboard>
#include <Qontrol>
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
    }
}

auto Receive::doConnect() -> void {
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

    m_btn_copy = new QPushButton("Copy");
    connect(m_btn_copy, &QPushButton::clicked,
            [addrQStr]() -> void { QApplication::clipboard()->setText(addrQStr); });

    auto *addrRow = (new qontrol::Row)->push(addrLabel)->push(addrDisplay)->pushSpacer();

    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_btn_copy)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(50)
                    ->push(addrRow)
                    ->pushSpacer(20)
                    ->push(btnRow)
                    ->pushSpacer();

    auto *oldWidget = m_main_widget;
    m_main_widget = margin(col);
    delete oldWidget;
    auto *oldLayout = this->layout();
    delete oldLayout;
    this->setLayout(m_main_widget->layout());
}

} // namespace screen

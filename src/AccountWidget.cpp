#include "AccountWidget.h"
#include "AccountController.h"
#include "StatusBar.h"
#include <Qontrol>
#include <common.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>

AccountWidget::AccountWidget(const QString &account, QWidget *parent) : QWidget(parent) {
    m_controller = new AccountController(account, this);
    initUI();
    m_controller->loadPanels();
}

auto AccountWidget::initUI() -> void {
    // Create side menu
    m_menu = new qontrol::Column(this);
    m_menu->setFixedWidth(200);

    auto *coinsBtn = new QPushButton("Coins");
    coinsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *sendBtn = new QPushButton("Send");
    sendBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *recvBtn = new QPushButton("Receive");
    recvBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *settingsBtn = new QPushButton("Settings");
    settingsBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_menu->push(coinsBtn);
    m_menu->push(sendBtn);
    m_menu->push(recvBtn);
    m_menu->push(settingsBtn);

    connect(coinsBtn, &QPushButton::clicked, m_controller, &AccountController::coinsClicked,
            qontrol::UNIQUE);
    connect(sendBtn, &QPushButton::clicked, m_controller, &AccountController::sendClicked,
            qontrol::UNIQUE);
    connect(recvBtn, &QPushButton::clicked, m_controller, &AccountController::receiveClicked,
            qontrol::UNIQUE);
    connect(settingsBtn, &QPushButton::clicked, m_controller, &AccountController::settingsClicked,
            qontrol::UNIQUE);

    // Create screen container
    auto *container = new qontrol::Column;
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->layout()->setContentsMargins(0, 0, 0, 0);
    m_screen_container = container;

    // Wrap menu + screen container in horizontal layout
    auto *contentWidget = (new qontrol::Row)->push(m_menu)->push(m_screen_container);
    contentWidget->layout()->setContentsMargins(0, 0, 0, 0);
    contentWidget->layout()->setSpacing(0);

    // Create status bar
    m_status_bar = new StatusBar(m_controller, this);

    // Connect status bar signals
    connect(m_controller, &AccountController::scannerStateChanged, m_status_bar,
            &StatusBar::updateConnectionState, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scanProgress, m_status_bar,
            &StatusBar::updateScanProgress, qontrol::UNIQUE);
    connect(m_controller, &AccountController::waitingForBlocks, m_status_bar,
            &StatusBar::updateWaitingForBlocks, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scanError, m_status_bar, &StatusBar::updateScanError,
            qontrol::UNIQUE);
    connect(m_controller, &AccountController::electrumConnected, m_status_bar,
            &StatusBar::onElectrumConnected, qontrol::UNIQUE);
    connect(m_controller, &AccountController::electrumDisconnected, m_status_bar,
            &StatusBar::onElectrumDisconnected, qontrol::UNIQUE);

    // Start scanner after all connections are established
    m_controller->startScanner();

    // Create main vertical layout with content + status bar
    auto *mainCol = (new qontrol::Column)->push(contentWidget)->push(m_status_bar);
    mainCol->layout()->setContentsMargins(0, 0, 0, 0);
    mainCol->layout()->setSpacing(0);
    setLayout(mainCol->layout());
}

auto AccountWidget::loadPanel(qontrol::Panel *panel) -> void {
    if (panel == nullptr)
        return;

    // Remove current panel if exists
    if (m_current_panel != nullptr) {
        auto *currentWidget = m_current_panel->widget();
        if (currentWidget != nullptr) {
            m_screen_container->layout()->removeWidget(currentWidget);
            currentWidget->setVisible(false);
        }
    }

    // Load new panel
    m_current_panel = panel;
    auto *widget = panel->widget();
    if (widget != nullptr) {
        m_screen_container->layout()->addWidget(widget);
        widget->setVisible(true);
    }
}

auto AccountWidget::controller() -> AccountController * {
    return m_controller;
}

AccountWidget::~AccountWidget() = default;

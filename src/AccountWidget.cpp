#include "AccountWidget.h"
#include "AccountController.h"
#include "StatusBar.h"
#include <Qontrol>
#include <qboxlayout.h>
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

    connect(coinsBtn, &QPushButton::clicked, m_controller, &AccountController::coinsClicked);
    connect(sendBtn, &QPushButton::clicked, m_controller, &AccountController::sendClicked);
    connect(recvBtn, &QPushButton::clicked, m_controller, &AccountController::receiveClicked);
    connect(settingsBtn, &QPushButton::clicked, m_controller, &AccountController::settingsClicked);

    // Create screen container
    m_screen_container = new QWidget(this);
    m_screen_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *containerLayout = new QVBoxLayout(m_screen_container);
    containerLayout->setContentsMargins(0, 0, 0, 0);

    // Wrap menu + screen container in horizontal layout
    auto *contentWidget = new QWidget(this);
    auto *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);
    contentLayout->addWidget(m_menu);
    contentLayout->addWidget(m_screen_container, 1);

    // Create status bar
    m_status_bar = new StatusBar(m_controller, this);

    // Connect status bar signals
    connect(m_controller, &AccountController::scannerStateChanged, m_status_bar,
            &StatusBar::updateConnectionState);
    connect(m_controller, &AccountController::scanProgress, m_status_bar,
            &StatusBar::updateScanProgress);
    connect(m_controller, &AccountController::waitingForBlocks, m_status_bar,
            &StatusBar::updateWaitingForBlocks);
    connect(m_controller, &AccountController::scanError, m_status_bar, &StatusBar::updateScanError);

    // Create main vertical layout with content + status bar
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(contentWidget, 1);
    mainLayout->addWidget(m_status_bar);
    setLayout(mainLayout);
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

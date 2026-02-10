#include "AccountWidget.h"
#include "AccountController.h"
#include <qboxlayout.h>
#include <qpushbutton.h>
#include <qsizepolicy.h>
#include <Qontrol>

AccountWidget::AccountWidget(const QString &account, QWidget *parent)
    : QWidget(parent) {
    m_controller = new AccountController(account, this);
    initUI();
    m_controller->loadPanels();
}

void AccountWidget::initUI() {
    // Create side menu
    m_menu = new qontrol::Column(this);
    m_menu->setFixedWidth(200);

    auto *coins_btn = new QPushButton("Coins");
    coins_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *send_btn = new QPushButton("Send");
    send_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *recv_btn = new QPushButton("Receive");
    recv_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *settings_btn = new QPushButton("Settings");
    settings_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_menu->push(coins_btn);
    m_menu->push(send_btn);
    m_menu->push(recv_btn);
    m_menu->push(settings_btn);

    connect(coins_btn, &QPushButton::clicked, m_controller, &AccountController::coinsClicked);
    connect(send_btn, &QPushButton::clicked, m_controller, &AccountController::sendClicked);
    connect(recv_btn, &QPushButton::clicked, m_controller, &AccountController::receiveClicked);
    connect(settings_btn, &QPushButton::clicked, m_controller, &AccountController::settingsClicked);

    // Create screen container
    m_screen_container = new QWidget(this);
    m_screen_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    auto *container_layout = new QVBoxLayout(m_screen_container);
    container_layout->setContentsMargins(0, 0, 0, 0);

    // Create main layout
    auto *main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0);
    main_layout->addWidget(m_menu);
    main_layout->addWidget(m_screen_container, 1);
    setLayout(main_layout);
}

void AccountWidget::loadPanel(qontrol::Panel *panel) {
    if (panel == nullptr)
        return;

    // Remove current panel if exists
    if (m_current_panel != nullptr) {
        auto *current_widget = m_current_panel->widget();
        if (current_widget != nullptr) {
            m_screen_container->layout()->removeWidget(current_widget);
            current_widget->setVisible(false);
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

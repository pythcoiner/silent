#include "StatusBar.h"
#include "AccountController.h"
#include "screens/common.h"
#include <QHBoxLayout>
#include <QBrush>

StatusBar::StatusBar(AccountController *controller, QWidget *parent)
    : QWidget(parent), m_controller(controller) {
    initUI();
    loadBlindbitUrl();

    // Set initial state
    m_connected = m_controller->isScannerRunning();
    updateConnectionState(m_connected);

    // Connect toggle signal
    connect(m_toggle, &qontrol::widgets::ToggleSwitch::toggled,
            this, &StatusBar::onToggled);
}

void StatusBar::initUI() {
    setFixedHeight(30);

    m_toggle = new qontrol::widgets::ToggleSwitch(this);
    m_toggle->setFixedSize(40, 20);

    m_status_text = new QLabel(this);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(H_SPACER);
    layout->addWidget(m_toggle);
    layout->addWidget(m_status_text);
    layout->addStretch();

    setLayout(layout);
}

void StatusBar::loadBlindbitUrl() {
    auto &account_opt = m_controller->getAccount();
    if (account_opt.has_value()) {
        auto account_name = account_opt.value()->name();
        auto config = config_from_file(account_name);
        m_blindbit_url = QString::fromStdString(
            std::string(config->get_blindbit_url().c_str()));
    }
}

void StatusBar::updateConnectionState(bool connected) {
    m_connected = connected;

    // Update toggle state without triggering signal
    m_toggle->blockSignals(true);
    m_toggle->setChecked(connected);
    m_toggle->blockSignals(false);

    // Update toggle color
    if (connected) {
        m_toggle->setBrush(QBrush(QColor(0, 180, 0)));  // Green
    } else {
        m_toggle->setBrush(QBrush(QColor(180, 0, 0)));  // Red
        m_status_text->setText("Disconnected");
    }

    m_toggle->update();
}

void StatusBar::updateScanProgress(uint32_t height, uint32_t tip) {
    if (height < tip) {
        m_status_text->setText(QString("Scanning... %1 / %2").arg(height).arg(tip));
    } else {
        // Synced
        m_status_text->setText(QString("Connected to blindbit-oracle at %1").arg(m_blindbit_url));
    }
}

void StatusBar::updateScanError(rust::String error) {
    QString errorStr = QString::fromStdString(std::string(error.c_str()));
    m_status_text->setText(QString("Error: %1").arg(errorStr));
}

void StatusBar::reloadUrl() {
    loadBlindbitUrl();
}

void StatusBar::onToggled(bool checked) {
    auto &account_opt = m_controller->getAccount();
    if (!account_opt.has_value()) {
        return;
    }

    if (checked) {
        // Connecting
        m_status_text->setText("Connecting...");
        m_toggle->setBrush(QBrush(QColor(0, 180, 0)));  // Green
        m_toggle->update();
        account_opt.value()->start_scanner();
    } else {
        // Disconnecting
        m_status_text->setText("Disconnecting...");
        m_toggle->setBrush(QBrush(QColor(180, 0, 0)));  // Red
        m_toggle->update();
        account_opt.value()->stop_scanner();
    }
}

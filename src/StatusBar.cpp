#include "StatusBar.h"
#include "AccountController.h"
#include "screens/utils.h"
#include <QBrush>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

StatusBar::StatusBar(AccountController *controller, QWidget *parent)
    : QWidget(parent),
      m_controller(controller) {
    initUI();
    loadBlindbitUrl();

    // Set initial state
    m_connected = m_controller->isScannerRunning();
    updateConnectionState(m_connected);

    // Connect toggle signal
    connect(m_toggle, &qontrol::widgets::ToggleSwitch::toggled, this, &StatusBar::onToggled);
}

auto StatusBar::initUI() -> void {
    setFixedHeight(32);

    // Top separator line
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    m_toggle = new qontrol::widgets::ToggleSwitch(this);
    m_toggle->setFixedSize(40, 20);

    m_status_text = new QLabel(this);

    // Horizontal row for toggle + status
    auto *row = new QHBoxLayout;
    row->setContentsMargins(10, 2, 10, 2);
    row->setSpacing(H_SPACER);
    row->addWidget(m_toggle);
    row->addWidget(m_status_text);
    row->addStretch();

    // Vertical layout: separator on top, content below
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(separator);
    layout->addLayout(row);

    setLayout(layout);
}

auto StatusBar::loadBlindbitUrl() -> void {
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);
        m_blindbit_url = QString::fromStdString(std::string(config->get_blindbit_url().c_str()));
    }
}

auto StatusBar::updateConnectionState(bool connected) -> void {
    m_connected = connected;

    // Update toggle state without triggering signal
    m_toggle->blockSignals(true);
    m_toggle->setChecked(connected);
    m_toggle->blockSignals(false);

    // Update toggle color and status text
    if (connected) {
        m_toggle->setBrush(QBrush(QColor(0, 180, 0))); // Green
        m_status_text->setText("Scanning...");
    } else {
        m_toggle->setBrush(QBrush(QColor(180, 0, 0))); // Red
        m_status_text->setText("Disconnected");
    }

    m_toggle->update();
}

auto StatusBar::updateScanProgress(uint32_t height, uint32_t tip) -> void {
    if (height < tip) {
        m_status_text->setText(QString("Scanning... %1 / %2").arg(height).arg(tip));
    } else {
        // Synced
        m_status_text->setText(QString("Connected to blindbit-oracle at %1").arg(m_blindbit_url));
    }
}

auto StatusBar::updateWaitingForBlocks(uint32_t tip_height) -> void {
    m_status_text->setText(QString("Synced at block %1 • Watching...").arg(tip_height));
}

auto StatusBar::updateScanError(rust::String error) -> void {
    QString errorStr = QString::fromStdString(std::string(error.c_str()));
    m_status_text->setText(QString("Error: %1").arg(errorStr));
}

auto StatusBar::reloadUrl() -> void {
    loadBlindbitUrl();
}

auto StatusBar::onToggled(bool checked) -> void {
    auto &accountOpt = m_controller->getAccount();
    if (!accountOpt.has_value()) {
        return;
    }

    if (checked) {
        // Connecting
        m_status_text->setText("Connecting...");
        m_toggle->setBrush(QBrush(QColor(0, 180, 0))); // Green
        m_toggle->update();
        accountOpt.value()->start_scanner();
    } else {
        // Disconnecting
        m_status_text->setText("Disconnecting...");
        m_toggle->setBrush(QBrush(QColor(180, 0, 0))); // Red
        m_toggle->update();
        accountOpt.value()->stop_scanner();
    }
}

#include "StatusBar.h"
#include "AccountController.h"
#include "screens/utils.h"
#include <QBrush>
#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

static auto formatEta(uint64_t secs) -> QString {
    auto hours = secs / 3600;
    auto mins = (secs % 3600) / 60;
    auto s = secs % 60;
    if (hours > 0) {
        return QString("~%1h %2m").arg(hours).arg(mins);
    }
    if (mins > 0) {
        return QString("~%1m %2s").arg(mins).arg(s);
    }
    return QString("~%1s").arg(s);
}

StatusBar::StatusBar(AccountController *controller, QWidget *parent)
    : QWidget(parent),
      m_controller(controller) {
    initUI();
    loadBlindbitUrl();
    loadElectrumUrl();

    // Set initial blindbit state
    m_connected = m_controller->isScannerRunning();
    updateConnectionState(m_connected);

    // Set initial electrum state (sub-accounts auto-start on construction)
    if (!m_electrum_url.isEmpty()) {
        m_electrum_status->setText("Connecting...");
    } else {
        m_electrum_status->setText("Not configured");
    }

    // Connect toggle signals
    connect(m_toggle, &qontrol::widgets::ToggleSwitch::toggled, this, &StatusBar::onToggled);
    connect(m_electrum_toggle, &qontrol::widgets::ToggleSwitch::toggled, this,
            &StatusBar::onElectrumToggled);
}

auto StatusBar::initUI() -> void {
    setFixedHeight(32);

    // Top separator line
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    // Blindbit toggle + status
    m_toggle = new qontrol::widgets::ToggleSwitch(this);
    m_toggle->setFixedSize(40, 20);

    m_status_text = new QLabel(this);

    // Vertical line separator
    auto *vline = new QFrame(this);
    vline->setFrameShape(QFrame::VLine);
    vline->setFrameShadow(QFrame::Sunken);

    // Electrum toggle + status
    m_electrum_toggle = new qontrol::widgets::ToggleSwitch(this);
    m_electrum_toggle->setFixedSize(40, 20);

    m_electrum_status = new QLabel(this);

    // Left half: blindbit
    auto *leftRow =
        (new qontrol::Row)->push(m_toggle)->pushSpacer(10)->push(m_status_text)->pushSpacer();

    // Right half: electrum
    auto *rightRow = (new qontrol::Row)
                         ->pushSpacer(10)
                         ->push(m_electrum_toggle)
                         ->pushSpacer(10)
                         ->push(m_electrum_status)
                         ->pushSpacer();

    // Combine with equal proportions
    auto *contentRow = new QHBoxLayout;
    contentRow->setContentsMargins(10, 2, 10, 2);
    contentRow->setSpacing(0);
    contentRow->addWidget(leftRow, 1);
    contentRow->addWidget(vline);
    contentRow->addWidget(rightRow, 1);

    // Vertical layout: separator on top, content below
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(separator);
    layout->addLayout(contentRow);

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

auto StatusBar::loadElectrumUrl() -> void {
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);
        m_electrum_url = QString::fromStdString(std::string(config->get_electrum_url().c_str()));
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
        auto eta = m_controller->etaSecs();
        QString text = QString("Scanning... %1 / %2").arg(height).arg(tip);
        if (eta > 0) {
            text += QString(" \u2022 %1").arg(formatEta(eta));
        }
        m_status_text->setText(text);
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

auto StatusBar::onElectrumConnected(QString address) -> void {
    m_electrum_connected = true;

    m_electrum_toggle->blockSignals(true);
    m_electrum_toggle->setChecked(true);
    m_electrum_toggle->blockSignals(false);

    m_electrum_toggle->setBrush(QBrush(QColor(0, 180, 0))); // Green
    m_electrum_toggle->update();
    m_electrum_status->setText(QString("Connected to electrum at %1").arg(address));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto StatusBar::onElectrumDisconnected() -> void {
    m_electrum_connected = false;

    m_electrum_toggle->blockSignals(true);
    m_electrum_toggle->setChecked(false);
    m_electrum_toggle->blockSignals(false);

    m_electrum_toggle->setBrush(QBrush(QColor(180, 0, 0))); // Red
    m_electrum_toggle->update();
    m_electrum_status->setText("Electrum disconnected");
}

auto StatusBar::reloadUrl() -> void {
    loadBlindbitUrl();
    loadElectrumUrl();
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

auto StatusBar::onElectrumToggled(bool checked) -> void {
    auto &accountOpt = m_controller->getAccount();
    if (!accountOpt.has_value()) {
        return;
    }

    if (checked) {
        m_electrum_status->setText("Connecting...");
        m_electrum_toggle->setBrush(QBrush(QColor(0, 180, 0))); // Green
        m_electrum_toggle->update();
        accountOpt.value()->start_electrum();
    } else {
        m_electrum_status->setText("Disconnecting...");
        m_electrum_toggle->setBrush(QBrush(QColor(180, 0, 0))); // Red
        m_electrum_toggle->update();
        accountOpt.value()->stop_electrum();
    }
}

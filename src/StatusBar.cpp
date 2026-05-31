#include "StatusBar.h"
#include "AccountController.h"
#include "i18n/Tr.h"
#include "screens/utils.h"
#include "theme/Label.h"
#include "theme/Toggle.h"
#include <QBrush>
#include <QFrame>
#include <common.h>

using theme::Label;
using theme::LabelRole;
using theme::Toggle;

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
        m_electrum_status_label->setText(TR("status-connecting"));
    } else {
        m_electrum_status_label->setText(TR("status-not-configured"));
    }

    // Connect toggle signals
    connect(m_toggle, &Toggle::clicked, this, &StatusBar::onToggled, qontrol::UNIQUE);
    connect(m_electrum_toggle, &Toggle::clicked, this, &StatusBar::onElectrumToggled,
            qontrol::UNIQUE);
}

void StatusBar::initUI() {
    setFixedHeight(resolve(Spacing::L));

    // Top separator line
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Sunken);

    // Blindbit toggle + status
    m_toggle = new Toggle(theme::ToggleRole::Status, this);

    m_status_label = new Label(LabelRole::Status, this);

    // Vertical line separator
    auto *vline = new QFrame(this);
    vline->setFrameShape(QFrame::VLine);
    vline->setFrameShadow(QFrame::Sunken);

    // Electrum toggle + status
    m_electrum_toggle = new Toggle(theme::ToggleRole::Status, this);

    m_electrum_status_label = new Label(LabelRole::Status, this);

    // Left half: blindbit
    auto *leftRow = (new qontrol::Row)
                        ->push(m_toggle)
                        ->pushSpacer(resolve(Spacing::S))
                        ->push(m_status_label)
                        ->pushSpacer();

    // Right half: electrum
    auto *rightRow = (new qontrol::Row)
                         ->pushSpacer(resolve(Spacing::S))
                         ->push(m_electrum_toggle)
                         ->pushSpacer(resolve(Spacing::S))
                         ->push(m_electrum_status_label)
                         ->pushSpacer();

    // Combine with equal proportions
    auto *contentRow = (new qontrol::Row)->push(leftRow)->push(vline)->push(rightRow);
    contentRow->layout()->setContentsMargins(resolve(Padding::S), resolve(Padding::XXS),
                                             resolve(Padding::S), resolve(Padding::XXS));
    contentRow->layout()->setSpacing(0);

    // Vertical layout: separator on top, content below
    auto *mainCol = (new qontrol::Column)->push(separator)->push(contentRow);
    mainCol->layout()->setContentsMargins(0, 0, 0, 0);
    mainCol->layout()->setSpacing(0);

    setLayout(mainCol->layout());
}

void StatusBar::loadBlindbitUrl() {
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);
        m_blindbit_url = QString::fromStdString(std::string(config->get_blindbit_url().c_str()));
    }
}

void StatusBar::loadElectrumUrl() {
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);
        m_electrum_url = QString::fromStdString(std::string(config->get_electrum_url().c_str()));
    }
}

void StatusBar::updateConnectionState(bool connected) {
    m_connected = connected;

    m_toggle->setChecked(connected);

    if (connected) {
        m_status_label->setText(TR("status-scanning"));
    } else {
        m_status_label->setText(TR("status-disconnected"));
    }
}

void StatusBar::updateScanProgress(uint32_t height, uint32_t tip) {
    if (height < tip) {
        auto eta = m_controller->etaSecs();
        QString text = TR("status-scanning-progress").arg(height).arg(tip);
        if (eta > 0) {
            text += QString(" \u2022 %1").arg(formatEta(eta));
        }
        m_status_label->setText(text);
    } else {
        // Synced
        m_status_label->setText(TR("status-connected-blindbit").arg(m_blindbit_url));
    }
}

void StatusBar::updateWaitingForBlocks(uint32_t tip_height) {
    m_status_label->setText(TR("status-synced-watching").arg(tip_height));
}

void StatusBar::updateScanError(rust::String error) {
    QString errorStr = QString::fromStdString(std::string(error.c_str()));
    m_status_label->setText(mapBackendErrorSummary(errorStr));
}

void StatusBar::onElectrumConnected(const QString &address) {
    m_electrum_connected = true;

    m_electrum_toggle->setChecked(true);

    m_electrum_status_label->setText(TR("status-connected-electrum").arg(address));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void StatusBar::onElectrumDisconnected() {
    m_electrum_connected = false;

    m_electrum_toggle->setChecked(false);

    m_electrum_status_label->setText(TR("status-electrum-disconnected"));
}

void StatusBar::reloadUrl() {
    loadBlindbitUrl();
    loadElectrumUrl();
}

void StatusBar::onToggled(bool checked) {
    auto &accountOpt = m_controller->getAccount();
    if (!accountOpt.has_value()) {
        return;
    }

    if (checked) {
        m_status_label->setText(TR("status-connecting"));
        accountOpt.value()->start_scanner();
    } else {
        m_status_label->setText(TR("status-disconnecting"));
        accountOpt.value()->stop_scanner();
    }
}

void StatusBar::onElectrumToggled(bool checked) {
    auto &accountOpt = m_controller->getAccount();
    if (!accountOpt.has_value()) {
        return;
    }

    if (checked) {
        m_electrum_status_label->setText(TR("status-connecting"));
        accountOpt.value()->start_electrum();
    } else {
        m_electrum_status_label->setText(TR("status-disconnecting"));
        accountOpt.value()->stop_electrum();
    }
}

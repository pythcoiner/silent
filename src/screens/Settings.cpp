#include "Settings.h"
#include "AccountController.h"
#include "AppController.h"
#include "Column.h"
#include "i18n/I18nManager.h"
#include "i18n/Tr.h"
#include "theme/Button.h"
#include "theme/ComboBox.h"
#include "theme/Input.h"
#include "theme/Label.h"
#include "utils.h"
#include <Qontrol>
#include <common.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qthread.h>

namespace screen {

using theme::Button;
using theme::ButtonRole;
using theme::ComboBox;
using theme::Input;
using theme::Label;
using theme::LabelRole;

Settings::Settings(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
    fetchBackendInfo();
}

void Settings::init() {
    // Load current config values
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);

        // Store config values to populate UI later
        m_current_url = QString::fromStdString(std::string(config->get_blindbit_url().c_str()));
        m_current_p2p_node = QString::fromStdString(std::string(config->get_p2p_node().c_str()));
        m_current_electrum_url =
            QString::fromStdString(std::string(config->get_electrum_url().c_str()));
        m_current_network = config->get_network();
    }

    // Create widgets
    m_blindbit_url_input = new Input;
    m_blindbit_url_input->setWidth(Size::XXL);
    m_blindbit_url_input->setPlaceholderText(TR("settings-placeholder-blindbit"));
    m_blindbit_url_input->setText(m_current_url);
    m_blindbit_url_input->setEnabled(!m_controller->isScannerRunning());

    m_test_btn = new Button(TR("common-test"));

    m_p2p_node_input = new Input;
    m_p2p_node_input->setWidth(Size::XXL);
    m_p2p_node_input->setPlaceholderText(TR("settings-placeholder-p2p"));
    m_p2p_node_input->setText(m_current_p2p_node);

    m_test_p2p_btn = new Button(TR("common-test"));

    m_electrum_url_input = new Input;
    m_electrum_url_input->setWidth(Size::XXL);
    m_electrum_url_input->setPlaceholderText(TR("settings-placeholder-electrum"));
    m_electrum_url_input->setText(m_current_electrum_url);
    m_electrum_url_input->setEnabled(!m_controller->isScannerRunning());

    m_test_electrum_btn = new Button(TR("common-test"));

    m_network_selector = new ComboBox;
    m_network_selector->addItem("Regtest", static_cast<int>(Network::Regtest));
    m_network_selector->addItem("Signet", static_cast<int>(Network::Signet));
    m_network_selector->addItem("Testnet", static_cast<int>(Network::Testnet));
    m_network_selector->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
    m_network_selector->setWidth(Size::M);

    int index = m_network_selector->findData(static_cast<int>(m_current_network));
    if (index != -1) {
        m_network_selector->setCurrentIndex(index);
    }
    m_network_selector->setEnabled(false);

    m_info_network_label = new Label("--");
    m_info_height_label = new Label("--");
    m_capabilities = new Label("--");

    m_language_selector = new ComboBox;
    m_language_selector->addItem("English", "en");
    m_language_selector->addItem("Francais", "fr");
    m_language_selector->addItem("Italiano", "it");
    m_language_selector->addItem("Deutsch", "de");
    m_language_selector->addItem("Portugues", "pt");
    m_language_selector->addItem("Espanol", "es");
    m_language_selector->setWidth(Size::M);

    int localeIndex = m_language_selector->findData(i18n::I18nManager::get()->selectedLocale());
    if (localeIndex == -1) {
        localeIndex = m_language_selector->findData("en");
    }
    if (localeIndex != -1) {
        m_language_selector->setCurrentIndex(localeIndex);
    }

    m_apply_language_btn = new Button;
    m_language_status_label = new Label;

    m_save_btn = new Button(TR("settings-save"), ButtonRole::Primary);
    m_toggle_blindbit_btn =
        new Button(m_controller->isScannerRunning() ? "Disconnect Blindbit" : "Connect Blindbit");
    m_toggle_electrum_btn =
        new Button(m_electrum_running ? "Disconnect Electrum" : "Connect Electrum");

    retranslateUi();
}

void Settings::doConnect() {
    connect(m_save_btn, &QPushButton::clicked, this, &Settings::actionSave, qontrol::UNIQUE);
    connect(m_toggle_blindbit_btn, &QPushButton::clicked, this, &Settings::actionToggleBlindbit,
            qontrol::UNIQUE);
    connect(m_toggle_electrum_btn, &QPushButton::clicked, this, &Settings::actionToggleElectrum,
            qontrol::UNIQUE);
    connect(m_test_btn, &QPushButton::clicked, this, &Settings::actionTestBackend, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scannerStateChanged, this,
            &Settings::updateBlindbitToggleButton, qontrol::UNIQUE);
    connect(m_controller, &AccountController::electrumConnected, this,
            &Settings::updateElectrumToggleButton, qontrol::UNIQUE);
    connect(m_controller, &AccountController::electrumDisconnected, this,
            &Settings::updateElectrumToggleButton, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scanProgress, this, &Settings::onScanProgress,
            qontrol::UNIQUE);
    connect(m_blindbit_url_input, &QLineEdit::textChanged, this, &Settings::invalidateBackendTest,
            qontrol::UNIQUE);
    connect(m_test_p2p_btn, &QPushButton::clicked, this, &Settings::actionTestP2p, qontrol::UNIQUE);
    connect(m_p2p_node_input, &QLineEdit::textChanged, this, &Settings::invalidateP2pTest,
            qontrol::UNIQUE);
    connect(m_test_electrum_btn, &QPushButton::clicked, this, &Settings::actionTestElectrum,
            qontrol::UNIQUE);
    connect(m_apply_language_btn, &QPushButton::clicked, this, &Settings::actionApplyLanguage,
            qontrol::UNIQUE);
    connect(m_electrum_url_input, &QLineEdit::textChanged, this, &Settings::invalidateElectrumTest,
            qontrol::UNIQUE);
    connect(this, &Settings::backendInfoReady, this, &Settings::onBackendInfoReady,
            qontrol::UNIQUE);
    connect(this, &Settings::p2pTestReady, this, &Settings::onP2pTestReady, qontrol::UNIQUE);
    connect(this, &Settings::electrumTestReady, this, &Settings::onElectrumTestReady,
            qontrol::UNIQUE);
}

void Settings::actionSave() {
    qDebug() << "Settings::actionSave()";

    auto &accountOpt = m_controller->getAccount();
    if (m_controller == nullptr || !accountOpt.has_value()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-no-account-loaded"));
        AppController::execModal(modal);
        return;
    }

    // Get account name
    auto accountName = accountOpt.value()->name();

    // Load existing config
    auto config = config_from_file(accountName);

    // Update BlindBit URL
    auto url = m_blindbit_url_input->text().toStdString();
    config->set_blindbit_url(rust::String(url));

    // Update P2P node address
    auto p2pNode = m_p2p_node_input->text().trimmed().toStdString();
    config->set_p2p_node(rust::String(p2pNode));

    // Update Electrum URL
    auto electrumUrl = m_electrum_url_input->text().trimmed().toStdString();
    config->set_electrum_url(rust::String(electrumUrl));

    // Update network
    int networkIndex = m_network_selector->currentIndex();
    auto network = static_cast<Network>(m_network_selector->itemData(networkIndex).toInt());
    config->set_network(network);

    // Save to file
    config->to_file();

    m_current_url = m_blindbit_url_input->text();

    emit configSaved();
}

void Settings::actionToggleBlindbit() {
    auto &accountOpt = m_controller->getAccount();
    if (m_controller == nullptr || !accountOpt.has_value()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-no-account-loaded"));
        AppController::execModal(modal);
        return;
    }

    if (m_controller->isScannerRunning()) {
        m_toggle_blindbit_btn->setEnabled(false);
        accountOpt.value()->stop_scanner();
        clearBackendInfo();
    } else {
        if (!accountOpt.value()->start_scanner()) {
            auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-failed-start-scanner"));
            AppController::execModal(modal);
        } else {
            fetchBackendInfo();
        }
    }
}

void Settings::actionToggleElectrum() {
    auto &accountOpt = m_controller->getAccount();
    if (m_controller == nullptr || !accountOpt.has_value()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-no-account-loaded"));
        AppController::execModal(modal);
        return;
    }

    if (m_electrum_running) {
        m_toggle_electrum_btn->setEnabled(false);
        accountOpt.value()->stop_electrum();
    } else {
        if (!accountOpt.value()->start_electrum()) {
            auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-failed-start-electrum"));
            AppController::execModal(modal);
        }
    }
}

void Settings::actionTestBackend() {
    auto url = m_blindbit_url_input->text().trimmed();
    if (url.isEmpty()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-blindbit-empty"));
        AppController::execModal(modal);
        return;
    }

    m_test_btn->setEnabled(false);
    m_test_btn->setText(TR("common-testing"));

    auto *thread = QThread::create([this, url = url.toStdString()]() -> void {
        auto info = ::get_backend_info(rust::String(url));
        emit backendInfoReady(info);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::fetchBackendInfo() {
    auto url = m_blindbit_url_input->text().trimmed();
    if (url.isEmpty())
        return;

    auto *thread = QThread::create([this, url = url.toStdString()]() -> void {
        auto info = ::get_backend_info(rust::String(url));
        emit backendInfoReady(info);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onBackendInfoReady(BackendInfo info) {
    m_test_btn->setEnabled(true);
    m_test_btn->setText(TR("common-test"));

    if (!info.is_ok) {
        m_backend_verified = false;
        clearBackendInfo();
        updateButtons();
        auto rawError = QString::fromStdString(std::string(info.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-connection-failed"), message));
        return;
    }

    QString networkStr;
    switch (info.network) {
    case Network::Regtest:
        networkStr = TR("network-regtest");
        break;
    case Network::Signet:
        networkStr = TR("network-signet");
        break;
    case Network::Testnet:
        networkStr = TR("network-testnet");
        break;
    case Network::Bitcoin:
        networkStr = TR("network-bitcoin");
        break;
    }

    bool networkMatch = false;
    networkMatch = (info.network == m_current_network);
    m_blindbit_url_input->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    updateButtons();
    m_info_network_label->setText(networkStr);
    m_current_height = info.height;
    m_info_height_label->setText(QString::number(info.height));

    auto yesText = TR("common-yes");
    auto noText = TR("common-no");
    auto yn = [&yesText, &noText](bool value) -> QString { return value ? yesText : noText; };
    m_capabilities->setText(TR("settings-capabilities-template")
                                 .arg(yn(info.tweaks_only))
                                 .arg(yn(info.tweaks_full_basic))
                                 .arg(yn(info.tweaks_full_with_dust_filter))
                                 .arg(yn(info.tweaks_cut_through_with_dust_filter)));
    m_capabilities->setAlignment(Qt::AlignTop);

    if (!networkMatch) {
        AppController::execModal(new qontrol::Modal(
            TR("settings-network-mismatch"), TR("settings-network-mismatch-message")
                                    .arg(networkStr)
                                    .arg(m_network_selector->currentText())));
    }
}

void Settings::invalidateBackendTest() {
    m_backend_verified = false;
    clearBackendInfo();
    updateButtons();
}

void Settings::actionTestP2p() {
    auto addr = m_p2p_node_input->text().trimmed();
    if (addr.isEmpty()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-p2p-empty"));
        AppController::execModal(modal);
        return;
    }

    m_test_p2p_btn->setEnabled(false);
    m_test_p2p_btn->setText(TR("common-testing"));

    auto network = m_current_network;
    auto *thread = QThread::create([this, addr = addr.toStdString(), network]() -> void {
        auto result = ::test_p2p_node(rust::String(addr), network);
        emit p2pTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onP2pTestReady(ConnectionResult result) {
    m_test_p2p_btn->setEnabled(true);
    m_test_p2p_btn->setText(TR("common-test"));

    if (!result.is_ok) {
        m_p2p_verified = false;
        updateButtons();
        auto rawError = QString::fromStdString(std::string(result.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-p2p-test-failed"), message));
        return;
    }

    m_p2p_verified = true;
    updateButtons();
}

void Settings::invalidateP2pTest() {
    m_p2p_verified = false;
    updateButtons();
}

void Settings::actionTestElectrum() {
    auto addr = m_electrum_url_input->text().trimmed();
    if (addr.isEmpty()) {
        auto *modal = new qontrol::Modal(TR("common-error"), TR("settings-electrum-empty"));
        AppController::execModal(modal);
        return;
    }

    m_test_electrum_btn->setEnabled(false);
    m_test_electrum_btn->setText(TR("common-testing"));

    auto *thread = QThread::create([this, addr = addr.toStdString()]() -> void {
        auto result = ::test_electrum(rust::String(addr));
        emit electrumTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onElectrumTestReady(ConnectionResult result) {
    m_test_electrum_btn->setEnabled(true);
    m_test_electrum_btn->setText(TR("common-test"));

    if (!result.is_ok) {
        m_electrum_verified = false;
        updateButtons();
        auto rawError = QString::fromStdString(std::string(result.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-electrum-test-failed"), message));
        return;
    }

    m_electrum_verified = true;
    updateButtons();
}

void Settings::invalidateElectrumTest() {
    m_electrum_verified = false;
    updateButtons();
}

void Settings::clearBackendInfo() {
    m_info_network_label->setText("--");
    m_info_height_label->setText("--");
    m_capabilities->setText("--");
    m_current_height = 0;
}

void Settings::updateButtons() {
    bool blindbitRunning = m_controller->isScannerRunning();
    m_save_btn->setEnabled(m_backend_verified && m_p2p_verified && m_electrum_verified);
    m_toggle_blindbit_btn->setEnabled(m_backend_verified || blindbitRunning);
    m_toggle_electrum_btn->setEnabled(m_electrum_verified || m_electrum_running);
    m_blindbit_url_input->setEnabled(!blindbitRunning);
    m_test_btn->setEnabled(!blindbitRunning);
    m_electrum_url_input->setEnabled(!m_electrum_running);
    m_test_electrum_btn->setEnabled(!m_electrum_running);
}

void Settings::onScanProgress(uint32_t height, [[maybe_unused]] uint32_t tip) {
    if (height > m_current_height) {
        m_current_height = height;
        m_info_height_label->setText(QString::number(height));
    }
}

void Settings::updateBlindbitToggleButton(bool running) {
    if (m_toggle_blindbit_btn != nullptr) {
        m_toggle_blindbit_btn->setText(running ? TR("settings-disconnect-blindbit")
                                               : TR("settings-connect-blindbit"));
    }
    updateButtons();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Settings::updateElectrumToggleButton() {
    m_electrum_running = !m_electrum_running;
    if (m_toggle_electrum_btn != nullptr) {
        m_toggle_electrum_btn->setText(m_electrum_running ? TR("settings-disconnect-electrum")
                                                          : TR("settings-connect-electrum"));
    }
    updateButtons();
}

void Settings::actionApplyLanguage() {
    auto locale = m_language_selector->currentData().toString();
    bool ok = i18n::I18nManager::get()->applyLocale(locale, true);
    m_language_status_label->setText(ok ? TR("settings-language-applied")
                                        : TR("settings-language-saved-restart"));
}

void Settings::view() {
    auto *urlLabel = new Label(TR("settings-blindbit-url"), LabelRole::InputLabel);

    auto *urlRow = (new qontrol::Row)
                       ->push(urlLabel)
                       ->push(m_blindbit_url_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_btn)
                       ->pushSpacer();

    auto *p2pLabel = new Label(TR("settings-p2p-node"), LabelRole::InputLabel);

    auto *p2pRow = (new qontrol::Row)
                       ->push(p2pLabel)
                       ->push(m_p2p_node_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_p2p_btn)
                       ->pushSpacer();

    auto *electrumLabel = new Label(TR("settings-electrum-server"), LabelRole::InputLabel);

    auto *electrumRow = (new qontrol::Row)
                            ->push(electrumLabel)
                            ->push(m_electrum_url_input)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_test_electrum_btn)
                            ->pushSpacer();

    auto *networkLabel = new Label(TR("settings-network"), LabelRole::InputLabel);

    auto *networkRow =
        (new qontrol::Row)->push(networkLabel)->push(m_network_selector)->pushSpacer();

    auto *languageLabel = new Label(TR("settings-language"), LabelRole::InputLabel);
    auto *languageRow = (new qontrol::Row)
                            ->push(languageLabel)
                            ->push(m_language_selector)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_apply_language_btn)
                            ->pushSpacer();
    auto *languageStatusTitle = new Label(TR("settings-status"), LabelRole::InfoLabel);
    auto *languageStatusRow =
        (new qontrol::Row)->push(languageStatusTitle)->push(m_language_status_label)->pushSpacer();

    // Backend info section (read-only)
    auto *infoTitle = new Label(TR("settings-backend-info"), LabelRole::Section);
    auto *infoTitleRow = (new qontrol::Row)->push(infoTitle)->pushSpacer();

    auto *netLabel = new Label(TR("settings-network"), LabelRole::InfoLabel);
    auto *infoNetRow = (new qontrol::Row)->push(netLabel)->push(m_info_network_label)->pushSpacer();

    auto *heightLabel = new Label(TR("settings-block-height"), LabelRole::InfoLabel);
    auto *infoHeightRow =
        (new qontrol::Row)->push(heightLabel)->push(m_info_height_label)->pushSpacer();

    auto *capabilities = new Label(TR("settings-capabilities"), LabelRole::InfoLabel);
    capabilities->setAlignment(Qt::AlignTop);
    auto *capRow = (new qontrol::Row)->push(capabilities)->push(m_capabilities)->pushSpacer();

    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_toggle_blindbit_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_toggle_electrum_btn)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_save_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::XXL))
                    ->push(urlRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(p2pRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(electrumRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(networkRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(languageRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(languageStatusRow)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(infoTitleRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(infoNetRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(infoHeightRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(capRow)
                    ->pushSpacer(resolve(Spacing::L))
                    ->push(buttonRow)
                    ->pushSpacer();

    auto *row = (new qontrol::Row)->pushSpacer()->push(col)->pushSpacer()->pushSpacer();

    m_main_widget = margin(row);
    this->setLayout(m_main_widget->layout());
}

void Settings::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    qontrol::Screen::changeEvent(event);
}

void Settings::retranslateUi() {
    m_blindbit_url_input->setPlaceholderText(TR("settings-placeholder-blindbit"));
    m_p2p_node_input->setPlaceholderText(TR("settings-placeholder-p2p"));
    m_electrum_url_input->setPlaceholderText(TR("settings-placeholder-electrum"));

    m_test_btn->setText(TR("common-test"));
    m_test_p2p_btn->setText(TR("common-test"));
    m_test_electrum_btn->setText(TR("common-test"));
    m_save_btn->setText(TR("settings-save"));
    m_apply_language_btn->setText(TR("common-apply"));

    m_toggle_blindbit_btn->setText(m_controller->isScannerRunning() ? TR("settings-disconnect-blindbit")
                                                                     : TR("settings-connect-blindbit"));
    m_toggle_electrum_btn->setText(m_electrum_running ? TR("settings-disconnect-electrum")
                                                       : TR("settings-connect-electrum"));

    m_network_selector->setItemText(0, TR("network-regtest"));
    m_network_selector->setItemText(1, TR("network-signet"));
    m_network_selector->setItemText(2, TR("network-testnet"));
    m_network_selector->setItemText(3, TR("network-bitcoin"));

}

} // namespace screen

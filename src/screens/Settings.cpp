#include "Settings.h"
#include "AccountController.h"
#include "AppController.h"
#include "Column.h"
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
using theme::InputRole;
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
    m_blindbit_url_input->setPlaceholderText("https://blindbit.example.com");
    m_blindbit_url_input->setText(m_current_url);
    m_blindbit_url_input->setEnabled(!m_controller->isScannerRunning());

    m_test_btn = new Button("Test");

    m_p2p_node_input = new Input;
    m_p2p_node_input->setWidth(Size::XXL);
    m_p2p_node_input->setPlaceholderText("127.0.0.1:8333");
    m_p2p_node_input->setText(m_current_p2p_node);

    m_test_p2p_btn = new Button("Test");

    m_electrum_url_input = new Input;
    m_electrum_url_input->setWidth(Size::XXL);
    m_electrum_url_input->setPlaceholderText("host:port (e.g. 127.0.0.1:50001)");
    m_electrum_url_input->setText(m_current_electrum_url);
    m_electrum_url_input->setEnabled(!m_controller->isScannerRunning());

    m_test_electrum_btn = new Button("Test");

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

    m_save_btn = new Button("Save Settings", ButtonRole::Primary);
    m_toggle_blindbit_btn =
        new Button(m_controller->isScannerRunning() ? "Disconnect Blindbit" : "Connect Blindbit");
    m_toggle_electrum_btn =
        new Button(m_electrum_running ? "Disconnect Electrum" : "Connect Electrum");
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
        auto *modal = new qontrol::Modal("Error", "No account loaded");
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
        auto *modal = new qontrol::Modal("Error", "No account loaded");
        AppController::execModal(modal);
        return;
    }

    if (m_controller->isScannerRunning()) {
        m_toggle_blindbit_btn->setEnabled(false);
        accountOpt.value()->stop_scanner();
        clearBackendInfo();
    } else {
        if (!accountOpt.value()->start_scanner()) {
            auto *modal = new qontrol::Modal("Error", "Failed to start scanner");
            AppController::execModal(modal);
        } else {
            fetchBackendInfo();
        }
    }
}

void Settings::actionToggleElectrum() {
    auto &accountOpt = m_controller->getAccount();
    if (m_controller == nullptr || !accountOpt.has_value()) {
        auto *modal = new qontrol::Modal("Error", "No account loaded");
        AppController::execModal(modal);
        return;
    }

    if (m_electrum_running) {
        m_toggle_electrum_btn->setEnabled(false);
        accountOpt.value()->stop_electrum();
    } else {
        if (!accountOpt.value()->start_electrum()) {
            auto *modal = new qontrol::Modal("Error", "Failed to start electrum");
            AppController::execModal(modal);
        }
    }
}

void Settings::actionTestBackend() {
    auto url = m_blindbit_url_input->text().trimmed();
    if (url.isEmpty()) {
        auto *modal = new qontrol::Modal("Error", "BlindBit URL is empty");
        AppController::execModal(modal);
        return;
    }

    m_test_btn->setEnabled(false);
    m_test_btn->setText("Testing...");

    auto *thread = QThread::create([this, url = url.toStdString()]() {
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

    auto *thread = QThread::create([this, url = url.toStdString()]() {
        auto info = ::get_backend_info(rust::String(url));
        emit backendInfoReady(info);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onBackendInfoReady(BackendInfo info) {
    m_test_btn->setEnabled(true);
    m_test_btn->setText("Test");

    if (!info.is_ok) {
        m_backend_verified = false;
        clearBackendInfo();
        updateButtons();
        AppController::execModal(
            new qontrol::Modal("Connection Failed",
                               QString("Failed to connect to backend:\n%1")
                                   .arg(QString::fromStdString(std::string(info.error.c_str())))));
        return;
    }

    QString networkStr;
    switch (info.network) {
    case Network::Regtest:
        networkStr = "Regtest";
        break;
    case Network::Signet:
        networkStr = "Signet";
        break;
    case Network::Testnet:
        networkStr = "Testnet";
        break;
    case Network::Bitcoin:
        networkStr = "Bitcoin";
        break;
    }

    bool networkMatch = (info.network == m_current_network);
    m_blindbit_url_input->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    updateButtons();
    m_info_network_label->setText(networkStr);
    m_current_height = info.height;
    m_info_height_label->setText(QString::number(info.height));

    auto yn = [](bool v) -> const char * { return v ? "Yes" : "No"; };
    m_capabilities->setText(QString("Tweaks Only: %1\nFull Basic: %2\nFull + Dust Filter: "
                                    "%3\nCut-Through + Dust Filter: %4")
                                .arg(yn(info.tweaks_only))
                                .arg(yn(info.tweaks_full_basic))
                                .arg(yn(info.tweaks_full_with_dust_filter))
                                .arg(yn(info.tweaks_cut_through_with_dust_filter)));
    m_capabilities->setAlignment(Qt::AlignTop);

    if (!networkMatch) {
        AppController::execModal(new qontrol::Modal(
            "Network Mismatch", QString("Backend network is %1 but account network is %2")
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
        auto *modal = new qontrol::Modal("Error", "P2P node address is empty");
        AppController::execModal(modal);
        return;
    }

    m_test_p2p_btn->setEnabled(false);
    m_test_p2p_btn->setText("Testing...");

    auto network = m_current_network;
    auto *thread = QThread::create([this, addr = addr.toStdString(), network]() {
        auto result = ::test_p2p_node(rust::String(addr), network);
        emit p2pTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onP2pTestReady(ConnectionResult result) {
    m_test_p2p_btn->setEnabled(true);
    m_test_p2p_btn->setText("Test");

    if (!result.is_ok) {
        m_p2p_verified = false;
        updateButtons();
        AppController::execModal(new qontrol::Modal(
            "P2P Test Failed",
            QString("Failed to connect to P2P node:\n%1")
                .arg(QString::fromStdString(std::string(result.error.c_str())))));
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
        auto *modal = new qontrol::Modal("Error", "Electrum address is empty");
        AppController::execModal(modal);
        return;
    }

    m_test_electrum_btn->setEnabled(false);
    m_test_electrum_btn->setText("Testing...");

    auto *thread = QThread::create([this, addr = addr.toStdString()]() {
        auto result = ::test_electrum(rust::String(addr));
        emit electrumTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Settings::onElectrumTestReady(ConnectionResult result) {
    m_test_electrum_btn->setEnabled(true);
    m_test_electrum_btn->setText("Test");

    if (!result.is_ok) {
        m_electrum_verified = false;
        updateButtons();
        AppController::execModal(new qontrol::Modal(
            "Electrum Test Failed",
            QString("Failed to connect to Electrum server:\n%1")
                .arg(QString::fromStdString(std::string(result.error.c_str())))));
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
        m_toggle_blindbit_btn->setText(running ? "Disconnect Blindbit" : "Connect Blindbit");
    }
    updateButtons();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Settings::updateElectrumToggleButton() {
    m_electrum_running = !m_electrum_running;
    if (m_toggle_electrum_btn != nullptr) {
        m_toggle_electrum_btn->setText(m_electrum_running ? "Disconnect Electrum"
                                                          : "Connect Electrum");
    }
    updateButtons();
}

void Settings::view() {
    auto *urlLabel = new Label("BlindBit URL:", LabelRole::InputLabel);

    auto *urlRow = (new qontrol::Row)
                       ->push(urlLabel)
                       ->push(m_blindbit_url_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_btn)
                       ->pushSpacer();

    auto *p2pLabel = new Label("P2P Node:", LabelRole::InputLabel);

    auto *p2pRow = (new qontrol::Row)
                       ->push(p2pLabel)
                       ->push(m_p2p_node_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_p2p_btn)
                       ->pushSpacer();

    auto *electrumLabel = new Label("Electrum Server:", LabelRole::InputLabel);

    auto *electrumRow = (new qontrol::Row)
                            ->push(electrumLabel)
                            ->push(m_electrum_url_input)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_test_electrum_btn)
                            ->pushSpacer();

    auto *networkLabel = new Label("Network:", LabelRole::InputLabel);

    auto *networkRow =
        (new qontrol::Row)->push(networkLabel)->push(m_network_selector)->pushSpacer();

    // Backend info section (read-only)
    auto *infoTitle = new Label("Backend Info:", LabelRole::Section);
    auto *infoTitleRow = (new qontrol::Row)->push(infoTitle)->pushSpacer();

    auto *netLabel = new Label("Network:", LabelRole::InfoLabel);
    auto *infoNetRow = (new qontrol::Row)->push(netLabel)->push(m_info_network_label)->pushSpacer();

    auto *heightLabel = new Label("Block Height:", LabelRole::InfoLabel);
    auto *infoHeightRow =
        (new qontrol::Row)->push(heightLabel)->push(m_info_height_label)->pushSpacer();

    auto *capabilities = new Label("Capabilities:", LabelRole::InfoLabel);
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

} // namespace screen

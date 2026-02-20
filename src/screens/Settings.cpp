#include "Settings.h"
#include "AccountController.h"
#include "AppController.h"
#include "utils.h"
#include <Qontrol>
#include <common.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qpushbutton.h>
#include <qthread.h>

namespace screen {

Settings::Settings(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
    fetchBackendInfo();
}

auto Settings::init() -> void {
    // Load current config values
    auto &accountOpt = m_controller->getAccount();
    if (accountOpt.has_value()) {
        auto accountName = accountOpt.value()->name();
        auto config = config_from_file(accountName);

        // Store config values to populate UI later
        m_current_url = QString::fromStdString(std::string(config->get_blindbit_url().c_str()));
        m_current_p2p_node = QString::fromStdString(std::string(config->get_p2p_node().c_str()));
        m_current_network = config->get_network();
    }

    // Create widgets
    m_blindbit_url = new QLineEdit;
    m_blindbit_url->setFixedWidth(3 * INPUT_WIDTH);
    m_blindbit_url->setPlaceholderText("https://blindbit.example.com");
    m_blindbit_url->setText(m_current_url);
    m_blindbit_url->setEnabled(!m_controller->isScannerRunning());

    m_btn_test = new QPushButton("Test");

    m_p2p_node = new QLineEdit;
    m_p2p_node->setFixedWidth(3 * INPUT_WIDTH);
    m_p2p_node->setPlaceholderText("127.0.0.1:8333");
    m_p2p_node->setText(m_current_p2p_node);

    m_btn_test_p2p = new QPushButton("Test");

    m_network_selector = new QComboBox;
    m_network_selector->addItem("Regtest", static_cast<int>(Network::Regtest));
    m_network_selector->addItem("Signet", static_cast<int>(Network::Signet));
    m_network_selector->addItem("Testnet", static_cast<int>(Network::Testnet));
    m_network_selector->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
    m_network_selector->setFixedWidth(INPUT_WIDTH);

    int index = m_network_selector->findData(static_cast<int>(m_current_network));
    if (index != -1) {
        m_network_selector->setCurrentIndex(index);
    }
    m_network_selector->setEnabled(false);

    m_info_network = new QLabel("--");
    m_info_height = new QLabel("--");
    m_info_tweaks = new QLabel("--");

    m_btn_save = new QPushButton("Save Settings");
    m_btn_toggle = new QPushButton(m_controller->isScannerRunning() ? "Disconnect" : "Connect");
}

auto Settings::doConnect() -> void {
    connect(m_btn_save, &QPushButton::clicked, this, &Settings::actionSave, qontrol::UNIQUE);
    connect(m_btn_toggle, &QPushButton::clicked, this, &Settings::actionToggle, qontrol::UNIQUE);
    connect(m_btn_test, &QPushButton::clicked, this, &Settings::actionTestBackend, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scannerStateChanged, this,
            &Settings::updateToggleButton, qontrol::UNIQUE);
    connect(m_controller, &AccountController::scanProgress, this, &Settings::onScanProgress,
            qontrol::UNIQUE);
    connect(m_blindbit_url, &QLineEdit::textChanged, this, &Settings::invalidateBackendTest,
            qontrol::UNIQUE);
    connect(m_btn_test_p2p, &QPushButton::clicked, this, &Settings::actionTestP2p, qontrol::UNIQUE);
    connect(m_p2p_node, &QLineEdit::textChanged, this, &Settings::invalidateP2pTest,
            qontrol::UNIQUE);
}

auto Settings::actionSave() -> void {
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
    auto url = m_blindbit_url->text().toStdString();
    config->set_blindbit_url(rust::String(url));

    // Update P2P node address
    auto p2pNode = m_p2p_node->text().trimmed().toStdString();
    config->set_p2p_node(rust::String(p2pNode));

    // Update network
    int networkIndex = m_network_selector->currentIndex();
    auto network = static_cast<Network>(m_network_selector->itemData(networkIndex).toInt());
    config->set_network(network);

    // Save to file
    config->to_file();

    m_current_url = m_blindbit_url->text();

    emit configSaved();
}

auto Settings::actionToggle() -> void {
    qDebug() << "Settings::actionToggle()";

    auto &accountOpt = m_controller->getAccount();
    if (m_controller == nullptr || !accountOpt.has_value()) {
        auto *modal = new qontrol::Modal("Error", "No account loaded");
        AppController::execModal(modal);
        return;
    }

    if (m_controller->isScannerRunning()) {
        // Disconnect
        m_btn_toggle->setEnabled(false);
        accountOpt.value()->stop_scanner();
        clearBackendInfo();
    } else {
        // Connect
        if (!accountOpt.value()->start_scanner()) {
            auto *modal = new qontrol::Modal("Error", "Failed to start scanner");
            AppController::execModal(modal);
        } else {
            fetchBackendInfo();
        }
    }
}

auto Settings::actionTestBackend() -> void {
    auto url = m_blindbit_url->text().trimmed();
    if (url.isEmpty()) {
        auto *modal = new qontrol::Modal("Error", "BlindBit URL is empty");
        AppController::execModal(modal);
        return;
    }

    m_btn_test->setEnabled(false);
    m_btn_test->setText("Testing...");

    auto *thread = QThread::create([this, url = url.toStdString()]() -> void {
        auto info = ::get_backend_info(rust::String(url));
        QMetaObject::invokeMethod(this, [this, info]() -> void {
            m_btn_test->setEnabled(true);
            m_btn_test->setText("Test");
            onBackendInfoReady(info);
        });
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto Settings::fetchBackendInfo() -> void {
    auto url = m_blindbit_url->text().trimmed();
    if (url.isEmpty())
        return;

    auto *thread = QThread::create([this, url = url.toStdString()]() -> void {
        auto info = ::get_backend_info(rust::String(url));
        QMetaObject::invokeMethod(this, [this, info]() -> void { onBackendInfoReady(info); });
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto Settings::onBackendInfoReady(BackendInfo info) -> void {
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
    m_blindbit_url->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    updateButtons();
    m_info_network->setText(networkStr);
    m_current_height = info.height;
    m_info_height->setText(QString::number(info.height));

    auto yn = [](bool v) -> const char * { return v ? "Yes" : "No"; };
    m_info_tweaks->setText(QString("Tweaks Only: %1\nFull Basic: %2\nFull + Dust Filter: "
                                   "%3\nCut-Through + Dust Filter: %4")
                               .arg(yn(info.tweaks_only))
                               .arg(yn(info.tweaks_full_basic))
                               .arg(yn(info.tweaks_full_with_dust_filter))
                               .arg(yn(info.tweaks_cut_through_with_dust_filter)));

    if (!networkMatch) {
        AppController::execModal(new qontrol::Modal(
            "Network Mismatch", QString("Backend network is %1 but account network is %2")
                                    .arg(networkStr)
                                    .arg(m_network_selector->currentText())));
    }
}

auto Settings::invalidateBackendTest() -> void {
    m_backend_verified = false;
    clearBackendInfo();
    updateButtons();
}

auto Settings::actionTestP2p() -> void {
    auto addr = m_p2p_node->text().trimmed();
    if (addr.isEmpty()) {
        auto *modal = new qontrol::Modal("Error", "P2P node address is empty");
        AppController::execModal(modal);
        return;
    }

    m_btn_test_p2p->setEnabled(false);
    m_btn_test_p2p->setText("Testing...");

    auto network = m_current_network;
    auto *thread = QThread::create([this, addr = addr.toStdString(), network]() -> void {
        auto result = ::test_p2p_node(rust::String(addr), network);
        QMetaObject::invokeMethod(this, [this, result]() -> void {
            m_btn_test_p2p->setEnabled(true);
            m_btn_test_p2p->setText("Test");
            onP2pTestReady(result);
        });
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto Settings::onP2pTestReady(ConnectionResult result) -> void {
    if (!result.is_ok) {
        m_p2p_verified = false;
        updateButtons();
        AppController::execModal(
            new qontrol::Modal("P2P Test Failed",
                               QString("Failed to connect to P2P node:\n%1")
                                   .arg(QString::fromStdString(std::string(result.error.c_str())))));
        return;
    }

    m_p2p_verified = true;
    updateButtons();
}

auto Settings::invalidateP2pTest() -> void {
    m_p2p_verified = false;
    updateButtons();
}

auto Settings::clearBackendInfo() -> void {
    m_info_network->setText("--");
    m_info_height->setText("--");
    m_info_tweaks->setText("--");
    m_current_height = 0;
}

auto Settings::updateButtons() -> void {
    bool connected = m_controller->isScannerRunning();
    m_btn_save->setEnabled(m_backend_verified && m_p2p_verified);
    m_btn_toggle->setEnabled(m_backend_verified || connected);
    m_blindbit_url->setEnabled(!connected);
    m_btn_test->setEnabled(!connected);
}

auto Settings::onScanProgress(uint32_t height, [[maybe_unused]] uint32_t tip) -> void {
    if (height > m_current_height) {
        m_current_height = height;
        m_info_height->setText(QString::number(height));
    }
}

auto Settings::updateToggleButton(bool running) -> void {
    if (m_btn_toggle != nullptr) {
        m_btn_toggle->setText(running ? "Disconnect" : "Connect");
    }
    updateButtons();
}

auto Settings::view() -> void {
    auto *urlLabel = new QLabel("BlindBit URL:");
    urlLabel->setFixedWidth(LABEL_WIDTH);

    auto *urlRow = (new qontrol::Row)
                       ->push(urlLabel)
                       ->push(m_blindbit_url)
                       ->pushSpacer(H_SPACER)
                       ->push(m_btn_test)
                       ->pushSpacer();

    auto *p2pLabel = new QLabel("P2P Node:");
    p2pLabel->setFixedWidth(LABEL_WIDTH);

    auto *p2pRow = (new qontrol::Row)
                       ->push(p2pLabel)
                       ->push(m_p2p_node)
                       ->pushSpacer(H_SPACER)
                       ->push(m_btn_test_p2p)
                       ->pushSpacer();

    auto *networkLabel = new QLabel("Network:");
    networkLabel->setFixedWidth(LABEL_WIDTH);

    auto *networkRow =
        (new qontrol::Row)->push(networkLabel)->push(m_network_selector)->pushSpacer();

    // Backend info section (read-only)
    auto *infoTitle = new QLabel("Backend Info:");
    infoTitle->setFixedWidth(LABEL_WIDTH);
    auto *infoTitleRow = (new qontrol::Row)->push(infoTitle)->pushSpacer();

    auto *netLabel = new QLabel("Network:");
    netLabel->setFixedWidth(LABEL_WIDTH);
    m_info_network->setFixedWidth(INPUT_WIDTH);
    auto *infoNetRow = (new qontrol::Row)->push(netLabel)->push(m_info_network)->pushSpacer();

    auto *heightLabel = new QLabel("Block Height:");
    heightLabel->setFixedWidth(LABEL_WIDTH);
    m_info_height->setFixedWidth(INPUT_WIDTH);
    auto *infoHeightRow = (new qontrol::Row)->push(heightLabel)->push(m_info_height)->pushSpacer();

    auto *tweaksLabel = new QLabel("Capabilities:");
    tweaksLabel->setFixedWidth(LABEL_WIDTH);
    m_info_tweaks->setFixedWidth(3 * INPUT_WIDTH);
    auto *infoTweaksRow = (new qontrol::Row)->push(tweaksLabel)->push(m_info_tweaks)->pushSpacer();

    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_btn_toggle)
                          ->pushSpacer(H_SPACER)
                          ->push(m_btn_save)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(50)
                    ->push(urlRow)
                    ->pushSpacer(V_SPACER)
                    ->push(p2pRow)
                    ->pushSpacer(V_SPACER)
                    ->push(networkRow)
                    ->pushSpacer(20)
                    ->push(infoTitleRow)
                    ->pushSpacer(V_SPACER)
                    ->push(infoNetRow)
                    ->pushSpacer(V_SPACER)
                    ->push(infoHeightRow)
                    ->pushSpacer(V_SPACER)
                    ->push(infoTweaksRow)
                    ->pushSpacer(30)
                    ->push(buttonRow)
                    ->pushSpacer();

    auto *row = (new qontrol::Row)->pushSpacer()->push(col)->pushSpacer()->pushSpacer();

    m_main_widget = margin(row);
    this->setLayout(m_main_widget->layout());
}

} // namespace screen

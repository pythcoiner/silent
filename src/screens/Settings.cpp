#include "Settings.h"
#include "AccountController.h"
#include "AppController.h"
#include "common.h"
#include <Qontrol>
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
  this->view();
  this->doConnect();
  fetchBackendInfo();
}

void Settings::init() {
  // Load current config values
  auto &account_opt = m_controller->getAccount();
  if (account_opt.has_value()) {
    auto account_name = account_opt.value()->name();
    auto config = config_from_file(account_name);

    // Store config values to populate UI later
    m_current_url =
        QString::fromStdString(std::string(config->get_blindbit_url().c_str()));
    m_current_network = config->get_network();
  }
}

void Settings::doConnect() {
  connect(m_btn_save, &QPushButton::clicked, this, &Settings::actionSave,
          qontrol::UNIQUE);
  connect(m_btn_toggle, &QPushButton::clicked, this, &Settings::actionToggle,
          qontrol::UNIQUE);
  connect(m_btn_test, &QPushButton::clicked, this, &Settings::actionTestBackend,
          qontrol::UNIQUE);
  connect(m_controller, &AccountController::scannerStateChanged, this,
          &Settings::updateToggleButton, qontrol::UNIQUE);
  connect(m_controller, &AccountController::scanProgress, this,
          &Settings::onScanProgress, qontrol::UNIQUE);
  connect(m_blindbit_url, &QLineEdit::textChanged, this,
          [this]() { invalidateBackendTest(); });
}

void Settings::actionSave() {
  qDebug() << "Settings::actionSave()";

  auto &account_opt = m_controller->getAccount();
  if (!m_controller || !account_opt.has_value()) {
    auto *modal = new qontrol::Modal("Error", "No account loaded");
    AppController::execModal(modal);
    return;
  }

  // Get account name
  auto account_name = account_opt.value()->name();

  // Load existing config
  auto config = config_from_file(account_name);

  // Update BlindBit URL
  auto url = m_blindbit_url->text().toStdString();
  config->set_blindbit_url(rust::String(url));

  // Update network
  int network_index = m_network_selector->currentIndex();
  auto network =
      static_cast<Network>(m_network_selector->itemData(network_index).toInt());
  config->set_network(network);

  // Save to file
  config->to_file();

  m_current_url = m_blindbit_url->text();

  auto *modal = new qontrol::Modal("Settings", "Settings saved successfully");
  AppController::execModal(modal);
  emit configSaved();
}

void Settings::actionToggle() {
  qDebug() << "Settings::actionToggle()";

  auto &account_opt = m_controller->getAccount();
  if (!m_controller || !account_opt.has_value()) {
    auto *modal = new qontrol::Modal("Error", "No account loaded");
    AppController::execModal(modal);
    return;
  }

  if (m_controller->isScannerRunning()) {
    // Disconnect
    m_btn_toggle->setEnabled(false);
    account_opt.value()->stop_scanner();
    clearBackendInfo();
  } else {
    // Connect
    try {
      account_opt.value()->start_scanner();
      fetchBackendInfo();
    } catch (const std::exception &e) {
      auto msg = QString("Failed to connect: %1").arg(e.what());
      auto *modal = new qontrol::Modal("Error", msg);
      AppController::execModal(modal);
    }
  }
}

void Settings::actionTestBackend() {
  auto url = m_blindbit_url->text().trimmed();
  if (url.isEmpty()) {
    auto *modal = new qontrol::Modal("Error", "BlindBit URL is empty");
    AppController::execModal(modal);
    return;
  }

  m_btn_test->setEnabled(false);
  m_btn_test->setText("Testing...");

  auto *thread = QThread::create([this, url = url.toStdString()]() {
    auto info = ::get_backend_info(rust::String(url));
    QMetaObject::invokeMethod(this, [this, info]() {
      m_btn_test->setEnabled(true);
      m_btn_test->setText("Test");
      onBackendInfoReady(info);
    });
  });
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

void Settings::fetchBackendInfo() {
  auto url = m_blindbit_url->text().trimmed();
  if (url.isEmpty())
    return;

  auto *thread = QThread::create([this, url = url.toStdString()]() {
    auto info = ::get_backend_info(rust::String(url));
    QMetaObject::invokeMethod(this, [this, info]() {
      onBackendInfoReady(info);
    });
  });
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

void Settings::onBackendInfoReady(BackendInfo info) {
  if (!info.is_ok) {
    m_backend_verified = false;
    clearBackendInfo();
    updateButtons();
    AppController::execModal(new qontrol::Modal(
        "Connection Failed",
        QString("Failed to connect to backend:\n%1")
            .arg(QString::fromStdString(std::string(info.error.c_str())))));
    return;
  }

  QString networkStr;
  switch (info.network) {
    case Network::Regtest: networkStr = "Regtest"; break;
    case Network::Signet: networkStr = "Signet"; break;
    case Network::Testnet: networkStr = "Testnet"; break;
    case Network::Bitcoin: networkStr = "Bitcoin"; break;
  }

  bool network_match = (info.network == m_current_network);
  m_backend_verified = network_match;
  updateButtons();

  m_info_network->setText(networkStr);
  m_current_height = info.height;
  m_info_height->setText(QString::number(info.height));

  auto yn = [](bool v) { return v ? "Yes" : "No"; };
  m_info_tweaks->setText(
      QString("Tweaks Only: %1\nFull Basic: %2\nFull + Dust Filter: %3\nCut-Through + Dust Filter: %4")
          .arg(yn(info.tweaks_only))
          .arg(yn(info.tweaks_full_basic))
          .arg(yn(info.tweaks_full_with_dust_filter))
          .arg(yn(info.tweaks_cut_through_with_dust_filter)));

  if (!network_match) {
    AppController::execModal(new qontrol::Modal(
        "Network Mismatch",
        QString("Backend network is %1 but account network is %2")
            .arg(networkStr)
            .arg(m_network_selector->currentText())));
  }
}

void Settings::invalidateBackendTest() {
  m_backend_verified = false;
  clearBackendInfo();
  updateButtons();
}

void Settings::clearBackendInfo() {
  m_info_network->setText("--");
  m_info_height->setText("--");
  m_info_tweaks->setText("--");
  m_current_height = 0;
}

void Settings::updateButtons() {
  bool connected = m_controller->isScannerRunning();
  m_btn_save->setEnabled(m_backend_verified && !connected);
  m_btn_toggle->setEnabled(m_backend_verified || connected);
  m_blindbit_url->setEnabled(!connected);
  m_btn_test->setEnabled(!connected);
}

void Settings::onScanProgress(uint32_t height, uint32_t tip) {
  if (height > m_current_height) {
    m_current_height = height;
    m_info_height->setText(QString::number(height));
  }
}

void Settings::updateToggleButton(bool running) {
  if (m_btn_toggle != nullptr) {
    m_btn_toggle->setText(running ? "Disconnect" : "Connect");
  }
  updateButtons();
}

void Settings::view() {
  if (m_view_init)
    return;

  auto *url_label = new QLabel("BlindBit URL:");
  url_label->setFixedWidth(LABEL_WIDTH);

  m_blindbit_url = new QLineEdit;
  m_blindbit_url->setFixedWidth(3 * INPUT_WIDTH);
  m_blindbit_url->setPlaceholderText("https://blindbit.example.com");
  m_blindbit_url->setText(m_current_url);
  m_blindbit_url->setEnabled(!m_controller->isScannerRunning());

  m_btn_test = new QPushButton("Test");

  auto *url_row = (new qontrol::Row)
                      ->push(url_label)
                      ->push(m_blindbit_url)
                      ->pushSpacer(H_SPACER)
                      ->push(m_btn_test)
                      ->pushSpacer();

  auto *network_label = new QLabel("Network:");
  network_label->setFixedWidth(LABEL_WIDTH);

  m_network_selector = new QComboBox;
  m_network_selector->addItem("Regtest", static_cast<int>(Network::Regtest));
  m_network_selector->addItem("Signet", static_cast<int>(Network::Signet));
  m_network_selector->addItem("Testnet", static_cast<int>(Network::Testnet));
  m_network_selector->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
  m_network_selector->setFixedWidth(INPUT_WIDTH);

  // Set current network selection
  int index = m_network_selector->findData(static_cast<int>(m_current_network));
  if (index != -1) {
    m_network_selector->setCurrentIndex(index);
  }
  m_network_selector->setEnabled(false);

  auto *network_row = (new qontrol::Row)
                          ->push(network_label)
                          ->push(m_network_selector)
                          ->pushSpacer();

  // Backend info section (read-only)
  auto *info_title = new QLabel("Backend Info:");
  info_title->setFixedWidth(LABEL_WIDTH);
  auto *info_title_row =
      (new qontrol::Row)->push(info_title)->pushSpacer();

  auto *net_label = new QLabel("Network:");
  net_label->setFixedWidth(LABEL_WIDTH);
  m_info_network = new QLabel("--");
  m_info_network->setFixedWidth(INPUT_WIDTH);
  auto *info_net_row =
      (new qontrol::Row)->push(net_label)->push(m_info_network)->pushSpacer();

  auto *height_label = new QLabel("Block Height:");
  height_label->setFixedWidth(LABEL_WIDTH);
  m_info_height = new QLabel("--");
  m_info_height->setFixedWidth(INPUT_WIDTH);
  auto *info_height_row =
      (new qontrol::Row)->push(height_label)->push(m_info_height)->pushSpacer();

  auto *tweaks_label = new QLabel("Capabilities:");
  tweaks_label->setFixedWidth(LABEL_WIDTH);
  m_info_tweaks = new QLabel("--");
  m_info_tweaks->setFixedWidth(3 * INPUT_WIDTH);
  auto *info_tweaks_row =
      (new qontrol::Row)->push(tweaks_label)->push(m_info_tweaks)->pushSpacer();

  m_btn_save = new QPushButton("Save Settings");
  m_btn_toggle = new QPushButton(m_controller->isScannerRunning() ? "Disconnect"
                                                                  : "Connect");

  auto *buttonRow = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(m_btn_toggle)
                        ->pushSpacer(H_SPACER)
                        ->push(m_btn_save)
                        ->pushSpacer();

  auto *col = (new qontrol::Column)
                  ->pushSpacer(50)
                  ->push(url_row)
                  ->pushSpacer(V_SPACER)
                  ->push(network_row)
                  ->pushSpacer(20)
                  ->push(info_title_row)
                  ->pushSpacer(V_SPACER)
                  ->push(info_net_row)
                  ->pushSpacer(V_SPACER)
                  ->push(info_height_row)
                  ->pushSpacer(V_SPACER)
                  ->push(info_tweaks_row)
                  ->pushSpacer(30)
                  ->push(buttonRow)
                  ->pushSpacer();

  auto *row =
      (new qontrol::Row)->pushSpacer()->push(col)->pushSpacer()->pushSpacer();

  m_main_widget = margin(row);
  this->setLayout(m_main_widget->layout());
  m_view_init = true;
}

} // namespace screen

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

namespace screen {

Settings::Settings(AccountController *ctrl) {
  m_controller = ctrl;
  this->init();
  this->view();
  this->doConnect();
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
  connect(m_controller, &AccountController::scannerStateChanged, this,
          &Settings::updateToggleButton, qontrol::UNIQUE);
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
    account_opt.value()->stop_scanner();
    auto *modal =
        new qontrol::Modal("Disconnect", "Disconnected from BlindBit server");
    AppController::execModal(modal);
  } else {
    // Connect
    try {
      account_opt.value()->start_scanner();
      auto *modal =
          new qontrol::Modal("Connect", "Connected to BlindBit server");
      AppController::execModal(modal);
    } catch (const std::exception &e) {
      auto msg = QString("Failed to connect: %1").arg(e.what());
      auto *modal = new qontrol::Modal("Error", msg);
      AppController::execModal(modal);
    }
  }
}

void Settings::updateToggleButton(bool running) {
  if (m_btn_toggle != nullptr) {
    m_btn_toggle->setText(running ? "Disconnect" : "Connect");
  }
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

  auto *url_row =
      (new qontrol::Row)->push(url_label)->push(m_blindbit_url)->pushSpacer();

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

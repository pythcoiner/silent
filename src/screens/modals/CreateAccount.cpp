#include "CreateAccount.h"
#include "AppController.h"
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qformlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qthread.h>
#include <qtextedit.h>

CreateAccount::CreateAccount(QWidget *parent) : qontrol::Modal() {
  Q_UNUSED(parent);
  setWindowTitle("Create Account");
  resize(600, 650);
  initUI();
}

void CreateAccount::initUI() {
  auto *layout = new QFormLayout();

  // Account name field
  m_name_input = new QLineEdit();
  m_name_input->setPlaceholderText("Enter account name");
  connect(m_name_input, &QLineEdit::textChanged, this,
          [this]() { updateCreateButton(); });
  layout->addRow("Account Name:", m_name_input);

  // Mode selection
  auto *mode_layout = new QVBoxLayout();
  m_generate_radio = new QRadioButton("Generate new mnemonic");
  m_restore_radio = new QRadioButton("Restore from mnemonic");
  m_generate_radio->setChecked(true);
  mode_layout->addWidget(m_generate_radio);
  mode_layout->addWidget(m_restore_radio);
  layout->addRow("Mode:", mode_layout);

  connect(m_generate_radio, &QRadioButton::toggled, this,
          &CreateAccount::onModeChanged);
  connect(m_restore_radio, &QRadioButton::toggled, this,
          &CreateAccount::onModeChanged);

  // Mnemonic field
  m_mnemonic_input = new QTextEdit();
  m_mnemonic_input->setPlaceholderText("12 or 24 word mnemonic");
  m_mnemonic_input->setReadOnly(true); // Start in generate mode
  m_mnemonic_input->setMaximumHeight(80);
  connect(m_mnemonic_input, &QTextEdit::textChanged, this,
          [this]() { updateCreateButton(); });
  layout->addRow("Mnemonic:", m_mnemonic_input);

  // Generate button
  m_generate_btn = new QPushButton("Generate");
  connect(m_generate_btn, &QPushButton::clicked, this,
          &CreateAccount::onGenerate);
  layout->addRow("", m_generate_btn);

  // Network selector
  m_network_combo = new QComboBox();
  m_network_combo->addItem("Regtest", static_cast<int>(Network::Regtest));
  m_network_combo->addItem("Signet", static_cast<int>(Network::Signet));
  m_network_combo->addItem("Testnet", static_cast<int>(Network::Testnet));
  m_network_combo->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
  m_network_combo->setCurrentIndex(0); // Default to Regtest
  connect(m_network_combo, &QComboBox::currentIndexChanged, this,
          &CreateAccount::onNetworkChanged);
  layout->addRow("Network:", m_network_combo);

  // BlindBit URL + Test button
  m_blindbit_input = new QLineEdit();
  m_blindbit_input->setPlaceholderText("http://localhost:8080");
  m_blindbit_input->setText("http://localhost:8080"); // Default

  m_test_btn = new QPushButton("Test");
  connect(m_test_btn, &QPushButton::clicked, this,
          &CreateAccount::onTestBackend);
  connect(m_blindbit_input, &QLineEdit::textChanged, this,
          [this]() { invalidateBackendTest(); });

  auto *url_layout = new QHBoxLayout();
  url_layout->addWidget(m_blindbit_input);
  url_layout->addWidget(m_test_btn);
  layout->addRow("BlindBit URL:", url_layout);

  // Buttons
  auto *button_layout = new QHBoxLayout();
  m_create_btn = new QPushButton("Create");
  m_create_btn->setEnabled(false);
  m_cancel_btn = new QPushButton("Cancel");
  button_layout->addStretch();
  button_layout->addWidget(m_cancel_btn);
  button_layout->addWidget(m_create_btn);

  connect(m_create_btn, &QPushButton::clicked, this, &CreateAccount::onCreate);
  connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject);

  layout->addRow(button_layout);

  auto *widget = new QWidget();
  widget->setLayout(layout);
  setMainWidget(widget);

  // Connect to AppController
  connect(this, &CreateAccount::createAccount, AppController::get(),
          &AppController::createAccount, qontrol::UNIQUE);
}

void CreateAccount::onGenerate() {
  auto mnemonic = generateMnemonic();
  m_mnemonic_input->setPlainText(mnemonic);
}

void CreateAccount::onCreate() {
  auto name = m_name_input->text().trimmed();
  auto mnemonic = m_mnemonic_input->toPlainText().trimmed();
  auto blindbit_url = m_blindbit_input->text().trimmed();
  auto network = static_cast<Network>(m_network_combo->currentData().toInt());

  emit createAccount(name, mnemonic, network, blindbit_url);
  accept();
}

void CreateAccount::onModeChanged() {
  bool is_mainnet =
      static_cast<Network>(m_network_combo->currentData().toInt()) ==
      Network::Bitcoin;

  if (m_generate_radio->isChecked()) {
    // Generate mode: mnemonic is readonly, generate button enabled (unless
    // mainnet)
    m_mnemonic_input->setReadOnly(true);
    m_generate_btn->setEnabled(!is_mainnet);
  } else {
    // Restore mode: mnemonic is editable, generate button disabled
    m_mnemonic_input->setReadOnly(false);
    m_mnemonic_input->clear();
    m_generate_btn->setEnabled(false);
  }
}

void CreateAccount::onNetworkChanged() {
  bool is_mainnet =
      static_cast<Network>(m_network_combo->currentData().toInt()) ==
      Network::Bitcoin;

  if (is_mainnet && m_generate_radio->isChecked()) {
    // Switch to restore mode on mainnet
    m_restore_radio->setChecked(true);
    m_generate_radio->setEnabled(false);
  } else {
    m_generate_radio->setEnabled(true);
  }

  invalidateBackendTest();
}

void CreateAccount::onTestBackend() {
  auto url = m_blindbit_input->text().trimmed();
  if (url.isEmpty()) {
    AppController::execModal(
        new qontrol::Modal("Invalid Input", "BlindBit URL cannot be empty."));
    return;
  }

  m_test_btn->setEnabled(false);
  m_test_btn->setText("Testing...");

  auto *thread = QThread::create([this, url = url.toStdString()]() {
    auto info = ::get_backend_info(rust::String(url));
    QMetaObject::invokeMethod(this, [this, info]() {
      onBackendInfoReady(info);
    });
  });
  connect(thread, &QThread::finished, thread, &QThread::deleteLater);
  thread->start();
}

void CreateAccount::onBackendInfoReady(BackendInfo info) {
  m_test_btn->setEnabled(true);
  m_test_btn->setText("Test");

  if (!info.is_ok) {
    m_backend_verified = false;
    updateCreateButton();
    AppController::execModal(new qontrol::Modal(
        "Connection Failed",
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

  auto selected_network =
      static_cast<Network>(m_network_combo->currentData().toInt());
  bool network_match = (info.network == selected_network);

  m_backend_verified = network_match;
  updateCreateButton();

  auto yn = [](bool v) { return v ? "Yes" : "No"; };
  QString msg =
      QString("Network: %1%2\n"
              "Block Height: %3\n\n"
              "Capabilities:\n"
              "  Tweaks Only: %4\n"
              "  Full Basic: %5\n"
              "  Full + Dust Filter: %6\n"
              "  Cut-Through + Dust Filter: %7")
          .arg(networkStr)
          .arg(network_match ? ""
                             : QString(" (mismatch: selected %1)")
                                   .arg(m_network_combo->currentText()))
          .arg(info.height)
          .arg(yn(info.tweaks_only))
          .arg(yn(info.tweaks_full_basic))
          .arg(yn(info.tweaks_full_with_dust_filter))
          .arg(yn(info.tweaks_cut_through_with_dust_filter));

  auto modal = new qontrol::Modal("Backend Info", msg);
  modal->setFixedSize(300, 230);
  AppController::execModal(modal);
}

void CreateAccount::invalidateBackendTest() {
  m_backend_verified = false;
  updateCreateButton();
}

void CreateAccount::updateCreateButton() {
  auto name = m_name_input->text().trimmed();
  auto mnemonic = m_mnemonic_input->toPlainText().trimmed();

  if (name.isEmpty() || name.contains(" ")) {
    m_create_btn->setEnabled(false);
    return;
  }

  // Check if account name already exists
  auto configs = ::list_configs();
  for (const auto &existing : configs) {
    if (QString::fromStdString(std::string(existing)) == name) {
      m_create_btn->setEnabled(false);
      return;
    }
  }

  if (mnemonic.isEmpty() ||
      !::validate_mnemonic(rust::String(mnemonic.toStdString()))) {
    m_create_btn->setEnabled(false);
    return;
  }

  m_create_btn->setEnabled(m_backend_verified);
}

auto CreateAccount::generateMnemonic() -> QString {
  auto mnemonic = ::generate_mnemonic();
  return QString::fromStdString(std::string(mnemonic));
}

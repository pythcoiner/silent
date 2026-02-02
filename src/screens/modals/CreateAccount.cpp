#include "CreateAccount.h"
#include "AppController.h"
#include <qboxlayout.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>

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
    layout->addRow("Account Name:", m_name_input);

    // Mode selection
    auto *mode_layout = new QVBoxLayout();
    m_generate_radio = new QRadioButton("Generate new mnemonic");
    m_restore_radio = new QRadioButton("Restore from mnemonic");
    m_generate_radio->setChecked(true);
    mode_layout->addWidget(m_generate_radio);
    mode_layout->addWidget(m_restore_radio);
    layout->addRow("Mode:", mode_layout);

    connect(m_generate_radio, &QRadioButton::toggled,
            this, &CreateAccount::onModeChanged);
    connect(m_restore_radio, &QRadioButton::toggled,
            this, &CreateAccount::onModeChanged);

    // Mnemonic field
    m_mnemonic_input = new QTextEdit();
    m_mnemonic_input->setPlaceholderText("12 or 24 word mnemonic");
    m_mnemonic_input->setReadOnly(true); // Start in generate mode
    m_mnemonic_input->setMaximumHeight(80);
    layout->addRow("Mnemonic:", m_mnemonic_input);

    // Generate button
    m_generate_btn = new QPushButton("Generate");
    connect(m_generate_btn, &QPushButton::clicked,
            this, &CreateAccount::onGenerate);
    layout->addRow("", m_generate_btn);

    // Network selector
    m_network_combo = new QComboBox();
    m_network_combo->addItem("Regtest", static_cast<int>(Network::Regtest));
    m_network_combo->addItem("Signet", static_cast<int>(Network::Signet));
    m_network_combo->addItem("Testnet", static_cast<int>(Network::Testnet));
    m_network_combo->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
    m_network_combo->setCurrentIndex(0); // Default to Regtest
    connect(m_network_combo, &QComboBox::currentIndexChanged,
            this, &CreateAccount::onNetworkChanged);
    layout->addRow("Network:", m_network_combo);

    // BlindBit URL
    m_blindbit_input = new QLineEdit();
    m_blindbit_input->setPlaceholderText("http://localhost:8080");
    m_blindbit_input->setText("http://localhost:8080"); // Default
    layout->addRow("BlindBit URL:", m_blindbit_input);

    // Buttons
    auto *button_layout = new QHBoxLayout();
    m_create_btn = new QPushButton("Create");
    m_cancel_btn = new QPushButton("Cancel");
    button_layout->addStretch();
    button_layout->addWidget(m_cancel_btn);
    button_layout->addWidget(m_create_btn);

    connect(m_create_btn, &QPushButton::clicked,
            this, &CreateAccount::onCreate);
    connect(m_cancel_btn, &QPushButton::clicked,
            this, &QDialog::reject);

    layout->addRow(button_layout);

    auto *widget = new QWidget();
    widget->setLayout(layout);
    setMainWidget(widget);

    // Connect to AppController
    connect(this, &CreateAccount::createAccount,
            AppController::get(), &AppController::createAccount,
            qontrol::UNIQUE);
}

void CreateAccount::onGenerate() {
    auto mnemonic = generateMnemonic();
    m_mnemonic_input->setPlainText(mnemonic);
}

void CreateAccount::onCreate() {
    // Validate inputs
    QString name = m_name_input->text().trimmed();
    QString mnemonic = m_mnemonic_input->toPlainText().trimmed();
    QString blindbit_url = m_blindbit_input->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Account name cannot be empty.");
        return;
    }

    if (name.contains(" ")) {
        QMessageBox::warning(this, "Invalid Input", "Account name cannot contain spaces.");
        return;
    }

    if (mnemonic.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Mnemonic cannot be empty.");
        return;
    }

    if (blindbit_url.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "BlindBit URL cannot be empty.");
        return;
    }

    // Get network
    auto network = static_cast<Network>(m_network_combo->currentData().toInt());

    // Emit signal to create account
    emit createAccount(name, mnemonic, network, blindbit_url);

    // Close dialog
    accept();
}

void CreateAccount::onModeChanged() {
    bool is_mainnet = static_cast<Network>(m_network_combo->currentData().toInt()) == Network::Bitcoin;

    if (m_generate_radio->isChecked()) {
        // Generate mode: mnemonic is readonly, generate button enabled (unless mainnet)
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
    bool is_mainnet = static_cast<Network>(m_network_combo->currentData().toInt()) == Network::Bitcoin;

    if (is_mainnet && m_generate_radio->isChecked()) {
        // Switch to restore mode on mainnet
        m_restore_radio->setChecked(true);
        m_generate_radio->setEnabled(false);
    } else {
        m_generate_radio->setEnabled(true);
    }
}

auto CreateAccount::generateMnemonic() -> QString {
    auto mnemonic = ::generate_mnemonic();
    return QString::fromStdString(std::string(mnemonic));
}

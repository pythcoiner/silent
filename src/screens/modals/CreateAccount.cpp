#include "CreateAccount.h"
#include "../utils.h"
#include "AppController.h"
#include "common.h"
#include <qlabel.h>
#include <qthread.h>

CreateAccount::CreateAccount(QWidget *parent) {
    Q_UNUSED(parent);
    setWindowTitle("Create Account");
    init();
    doConnect();
    view();
}

auto CreateAccount::init() -> void {
    m_name_input = new QLineEdit();
    m_name_input->setPlaceholderText("Enter account name");

    m_mnemonic_input = new QTextEdit();
    m_mnemonic_input->setPlaceholderText("Mnemonic phrase");
    m_mnemonic_input->setMaximumHeight(50);
    m_mnemonic_input->setMinimumWidth(200);

    m_generate_btn = new QPushButton("Generate");

    m_network_combo = new QComboBox();
    m_network_combo->addItem("Regtest", static_cast<int>(Network::Regtest));
    m_network_combo->addItem("Signet", static_cast<int>(Network::Signet));
    m_network_combo->addItem("Testnet", static_cast<int>(Network::Testnet));
    m_network_combo->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
    m_network_combo->setCurrentIndex(0);

    m_blindbit_input = new QLineEdit();
    m_blindbit_input->setPlaceholderText("http://localhost:8080");
    m_blindbit_input->setText("http://localhost:8080");

    m_p2p_input = new QLineEdit();
    m_p2p_input->setPlaceholderText("127.0.0.1:18444");

    m_test_btn = new QPushButton("Test");
    m_test_p2p_btn = new QPushButton("Test");

    m_create_btn = new QPushButton("Create");
    m_create_btn->setEnabled(false);

    m_cancel_btn = new QPushButton("Cancel");
}

auto CreateAccount::doConnect() -> void {
    connect(m_name_input, &QLineEdit::textChanged, this, &CreateAccount::onUpdateCreateButton,
            qontrol::UNIQUE);
    connect(m_mnemonic_input, &QTextEdit::textChanged, this, &CreateAccount::onUpdateCreateButton,
            qontrol::UNIQUE);
    connect(m_generate_btn, &QPushButton::clicked, this, &CreateAccount::onGenerate,
            qontrol::UNIQUE);
    connect(m_network_combo, &QComboBox::currentIndexChanged, this,
            &CreateAccount::onNetworkChanged, qontrol::UNIQUE);
    connect(m_test_btn, &QPushButton::clicked, this, &CreateAccount::onTestBackend,
            qontrol::UNIQUE);
    connect(m_blindbit_input, &QLineEdit::textChanged, this, &CreateAccount::invalidateBackendTest,
            qontrol::UNIQUE);
    connect(m_test_p2p_btn, &QPushButton::clicked, this, &CreateAccount::onTestP2p,
            qontrol::UNIQUE);
    connect(m_p2p_input, &QLineEdit::textChanged, this, &CreateAccount::invalidateP2pTest,
            qontrol::UNIQUE);
    connect(m_create_btn, &QPushButton::clicked, this, &CreateAccount::onCreate, qontrol::UNIQUE);
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(this, &CreateAccount::createAccount, AppController::get(),
            &AppController::createAccount, qontrol::UNIQUE);
}

auto CreateAccount::view() -> void {
    auto *nameRow = (new qontrol::Row)
                        ->push(new QLabel("Account Name:"))
                        ->pushSpacer(H_SPACER)
                        ->push(m_name_input);

    auto *mnemonicRow = (new qontrol::Row)
                            ->push(new QLabel("Mnemonic:"))
                            ->pushSpacer(H_SPACER)
                            ->push(m_mnemonic_input);

    auto *generateRow = (new qontrol::Row)->pushSpacer()->push(m_generate_btn)->pushSpacer();

    auto *networkRow = (new qontrol::Row)
                           ->push(new QLabel("Network:"))
                           ->pushSpacer(H_SPACER)
                           ->push(m_network_combo);

    auto *urlRow = (new qontrol::Row)
                       ->push(new QLabel("BlindBit URL:"))
                       ->pushSpacer(H_SPACER)
                       ->push(m_blindbit_input)
                       ->pushSpacer(H_SPACER)
                       ->push(m_test_btn);

    auto *p2pRow = (new qontrol::Row)
                       ->push(new QLabel("P2P Node:"))
                       ->pushSpacer(H_SPACER)
                       ->push(m_p2p_input)
                       ->pushSpacer(H_SPACER)
                       ->push(m_test_p2p_btn);

    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(H_SPACER * 3)
                          ->push(m_create_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->push(nameRow)
                    ->pushSpacer(V_SPACER)
                    ->push(mnemonicRow)
                    ->pushSpacer(V_SPACER)
                    ->push(generateRow)
                    ->pushSpacer(V_SPACER)
                    ->push(networkRow)
                    ->pushSpacer(V_SPACER)
                    ->push(urlRow)
                    ->pushSpacer(V_SPACER)
                    ->push(p2pRow)
                    ->pushSpacer(V_SPACER)
                    ->push(buttonRow);

    setMainWidget(margin(col));
}

auto CreateAccount::onGenerate() -> void {
    auto mnemonic = generateMnemonic();
    m_mnemonic_input->setPlainText(mnemonic);
}

auto CreateAccount::onCreate() -> void {
    auto name = m_name_input->text().trimmed();
    auto mnemonic = m_mnemonic_input->toPlainText().trimmed();
    auto blindbitUrl = m_blindbit_input->text().trimmed();
    auto p2pNode = m_p2p_input->text().trimmed();
    auto network = static_cast<Network>(m_network_combo->currentData().toInt());

    emit createAccount(name, mnemonic, network, blindbitUrl, p2pNode);
    accept();
}

auto CreateAccount::onNetworkChanged() -> void {
    bool isMainnet =
        static_cast<Network>(m_network_combo->currentData().toInt()) == Network::Bitcoin;

    m_generate_btn->setEnabled(!isMainnet);
    invalidateBackendTest();
    invalidateP2pTest();
}

auto CreateAccount::onTestBackend() -> void {
    auto url = m_blindbit_input->text().trimmed();
    if (url.isEmpty()) {
        AppController::execModal(
            new qontrol::Modal("Invalid Input", "BlindBit URL cannot be empty."));
        return;
    }

    m_test_btn->setEnabled(false);
    m_test_btn->setText("Testing...");

    auto *thread = QThread::create([this, url = url.toStdString()]() -> void {
        auto info = ::get_backend_info(rust::String(url));
        QMetaObject::invokeMethod(this, [this, info]() -> void { onBackendInfoReady(info); });
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto CreateAccount::onBackendInfoReady(BackendInfo info) -> void {
    m_test_btn->setEnabled(true);
    m_test_btn->setText("Test");

    if (!info.is_ok) {
        m_backend_verified = false;
        onUpdateCreateButton();
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

    auto selectedNetwork = static_cast<Network>(m_network_combo->currentData().toInt());
    bool networkMatch = (info.network == selectedNetwork);

    m_blindbit_input->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    onUpdateCreateButton();

    auto yn = [](bool v) -> const char * { return v ? "Yes" : "No"; };
    QString msg =
        QString("Network: %1%2\n"
                "Block Height: %3\n\n"
                "Capabilities:\n"
                "  Tweaks Only: %4\n"
                "  Full Basic: %5\n"
                "  Full + Dust Filter: %6\n"
                "  Cut-Through + Dust Filter: %7")
            .arg(networkStr)
            .arg(networkMatch
                     ? ""
                     : QString(" (mismatch: selected %1)").arg(m_network_combo->currentText()))
            .arg(info.height)
            .arg(yn(info.tweaks_only))
            .arg(yn(info.tweaks_full_basic))
            .arg(yn(info.tweaks_full_with_dust_filter))
            .arg(yn(info.tweaks_cut_through_with_dust_filter));

    auto *modal = new qontrol::Modal("Backend Info", msg);
    modal->setFixedSize(300, 230);
    AppController::execModal(modal);
}

auto CreateAccount::invalidateBackendTest() -> void {
    m_backend_verified = false;
    onUpdateCreateButton();
}

auto CreateAccount::onTestP2p() -> void {
    auto addr = m_p2p_input->text().trimmed();
    if (addr.isEmpty()) {
        AppController::execModal(
            new qontrol::Modal("Invalid Input", "P2P node address cannot be empty."));
        return;
    }

    m_test_p2p_btn->setEnabled(false);
    m_test_p2p_btn->setText("Testing...");

    auto network = static_cast<Network>(m_network_combo->currentData().toInt());
    auto *thread = QThread::create([this, addr = addr.toStdString(), network]() -> void {
        auto result = ::test_p2p_node(rust::String(addr), network);
        QMetaObject::invokeMethod(this, [this, result]() -> void { onP2pTestReady(result); });
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto CreateAccount::onP2pTestReady(ConnectionResult result) -> void {
    m_test_p2p_btn->setEnabled(true);
    m_test_p2p_btn->setText("Test");

    if (!result.is_ok) {
        m_p2p_verified = false;
        onUpdateCreateButton();
        AppController::execModal(
            new qontrol::Modal("P2P Test Failed",
                               QString("Failed to connect to P2P node:\n%1")
                                   .arg(QString::fromStdString(std::string(result.error.c_str())))));
        return;
    }

    m_p2p_verified = true;
    onUpdateCreateButton();
}

auto CreateAccount::invalidateP2pTest() -> void {
    m_p2p_verified = false;
    onUpdateCreateButton();
}

auto CreateAccount::onUpdateCreateButton() -> void {
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

    if (mnemonic.isEmpty() || !::validate_mnemonic(rust::String(mnemonic.toStdString()))) {
        m_create_btn->setEnabled(false);
        return;
    }

    m_create_btn->setEnabled(m_backend_verified && m_p2p_verified);
}

auto CreateAccount::generateMnemonic() -> QString { // NOLINT
    auto mnemonic = ::generate_mnemonic();
    return QString::fromStdString(std::string(mnemonic));
}

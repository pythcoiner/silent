#include "CreateAccount.h"
#include "screens/utils.h"
#include "AppController.h"
#include "common.h"
#include "i18n/Tr.h"
#include "theme/Button.h"
#include "theme/ComboBox.h"
#include "theme/Input.h"
#include "theme/Label.h"
#include "theme/TextEdit.h"
#include <qthread.h>

namespace modal {

using theme::Button;
using theme::ButtonRole;
using theme::ComboBox;
using theme::Input;
using theme::InputRole;
using theme::Label;
using theme::LabelRole;
using theme::TextEdit;

CreateAccount::CreateAccount([[maybe_unused]] QWidget *parent) {
    setWindowTitle(TR("create-account-title"));
    init();
    doConnect();
    view();
    applyRegtestDefaults();
}

void CreateAccount::init() {
    m_name_input = new Input;
    m_name_input->setPlaceholderText(TR("create-account-placeholder-name"));

    m_mnemonic_input = new TextEdit;
    m_mnemonic_input->setPlaceholderText(TR("create-account-placeholder-mnemonic"));
    m_mnemonic_input->setMaximumHeight(50);
    m_mnemonic_input->setMinimumWidth(200);

    m_generate_btn = new Button(TR("common-generate"));

    m_network_combo = new ComboBox;
    m_network_combo->addItem(TR("network-regtest"), static_cast<int>(Network::Regtest));
    m_network_combo->addItem(TR("network-signet"), static_cast<int>(Network::Signet));
    m_network_combo->addItem(TR("network-testnet"), static_cast<int>(Network::Testnet));
    m_network_combo->addItem(TR("network-bitcoin"), static_cast<int>(Network::Bitcoin));
    m_network_combo->setCurrentIndex(0);

    m_blindbit_input = new Input;
    m_blindbit_input->setPlaceholderText(TR("create-account-placeholder-blindbit"));
    m_blindbit_input->setText(TR("create-account-default-blindbit"));

    m_p2p_input = new Input;
    m_p2p_input->setPlaceholderText(TR("create-account-placeholder-p2p"));

    m_test_btn = new Button(TR("common-test"));
    m_test_p2p_btn = new Button(TR("common-test"));

    m_electrum_input = new Input;
    m_electrum_input->setPlaceholderText(TR("settings-placeholder-electrum"));

    m_test_electrum_btn = new Button(TR("common-test"));

    m_create_btn = new Button(TR("create-account-action-create"), ButtonRole::Primary);
    m_create_btn->setEnabled(false);

    m_cancel_btn = new Button(TR("common-cancel"));
}

void CreateAccount::doConnect() {
    connect(m_name_input, &QLineEdit::textChanged, this, &CreateAccount::onUpdateCreateButton,
            qontrol::UNIQUE);
    connect(m_mnemonic_input, &TextEdit::textChanged, this, &CreateAccount::onUpdateCreateButton,
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
    connect(m_test_electrum_btn, &QPushButton::clicked, this, &CreateAccount::onTestElectrum,
            qontrol::UNIQUE);
    connect(m_electrum_input, &QLineEdit::textChanged, this, &CreateAccount::invalidateElectrumTest,
            qontrol::UNIQUE);
    connect(m_create_btn, &QPushButton::clicked, this, &CreateAccount::onCreate, qontrol::UNIQUE);
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(this, &CreateAccount::backendInfoReady, this, &CreateAccount::onBackendInfoReady,
            qontrol::UNIQUE);
    connect(this, &CreateAccount::p2pTestReady, this, &CreateAccount::onP2pTestReady,
            qontrol::UNIQUE);
    connect(this, &CreateAccount::electrumTestReady, this, &CreateAccount::onElectrumTestReady,
            qontrol::UNIQUE);
}

void CreateAccount::view() {
    auto *nameRow = (new qontrol::Row)
                        ->push(new Label(TR("create-account-name"), LabelRole::InputLabel))
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(m_name_input);

    auto *mnemonicRow = (new qontrol::Row)
                            ->push(new Label(TR("create-account-mnemonic"), LabelRole::InputLabel))
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_mnemonic_input);

    auto *generateRow = (new qontrol::Row)->pushSpacer()->push(m_generate_btn)->pushSpacer();

    auto *networkRow = (new qontrol::Row)
                           ->push(new Label(TR("settings-network"), LabelRole::InputLabel))
                           ->pushSpacer(resolve(Spacing::XS))
                           ->push(m_network_combo);

    auto *urlRow = (new qontrol::Row)
                       ->push(new Label(TR("settings-blindbit-url"), LabelRole::InputLabel))
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_blindbit_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_btn);

    auto *p2pRow = (new qontrol::Row)
                       ->push(new Label(TR("settings-p2p-node"), LabelRole::InputLabel))
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_p2p_input)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_test_p2p_btn);

    auto *electrumRow = (new qontrol::Row)
                            ->push(new Label(TR("create-account-electrum"), LabelRole::InputLabel))
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_electrum_input)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_test_electrum_btn);

    auto *buttonRow = (new qontrol::Row)
                          ->pushSpacer()
                          ->push(m_cancel_btn)
                          ->pushSpacer(resolve(Padding::M))
                          ->push(m_create_btn)
                          ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->push(nameRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(mnemonicRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(generateRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(networkRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(urlRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(p2pRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(electrumRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(buttonRow);

    setMainWidget(margin(col));
}

void CreateAccount::onGenerate() {
    auto mnemonic = generateMnemonic();
    m_mnemonic_input->setText(mnemonic);
}

void CreateAccount::onCreate() {
    auto name = m_name_input->text().trimmed();
    auto mnemonic = m_mnemonic_input->toPlainText().trimmed();
    auto blindbitUrl = m_blindbit_input->text().trimmed();
    auto p2pNode = m_p2p_input->text().trimmed();
    auto electrumUrl = m_electrum_input->text().trimmed();
    auto network = static_cast<Network>(m_network_combo->currentData().toInt());

    auto config =
        new_config(rust::String(name.toStdString()), network, rust::String(mnemonic.toStdString()),
                   rust::String(blindbitUrl.toStdString()), rust::String(p2pNode.toStdString()),
                   rust::String(electrumUrl.toStdString()), 546, rust::String("sp"));
    config->to_file();

    AppController::get()->onAccountCreated(name);
    accept();
}

void CreateAccount::onNetworkChanged() {
    bool isMainnet = false;
    isMainnet =
        static_cast<Network>(m_network_combo->currentData().toInt()) == Network::Bitcoin;

    m_generate_btn->setEnabled(!isMainnet);
    invalidateBackendTest();
    invalidateP2pTest();
    invalidateElectrumTest();
    applyRegtestDefaults();
}

void CreateAccount::onTestBackend() {
    auto url = m_blindbit_input->text().trimmed();
    if (url.isEmpty()) {
        AppController::execModal(
            new qontrol::Modal(TR("create-account-invalid-input"), TR("create-account-blindbit-empty")));
        return;
    }

    m_test_btn->setEnabled(false);
    m_test_btn->setText(TR("common-testing"));

    auto *thread = QThread::create([this, url = url.toStdString()]() {
        auto info = ::get_backend_info(rust::String(url));
        emit backendInfoReady(info);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void CreateAccount::onBackendInfoReady(BackendInfo info) {
    m_test_btn->setEnabled(true);
    m_test_btn->setText(TR("common-test"));

    if (!info.is_ok) {
        m_backend_verified = false;
        onUpdateCreateButton();
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

    auto selectedNetwork = static_cast<Network>(m_network_combo->currentData().toInt());
    bool networkMatch = (info.network == selectedNetwork);

    m_blindbit_input->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    onUpdateCreateButton();

    auto yn = [](bool v) -> QString { return v ? TR("common-yes") : TR("common-no"); };
    QString msg =
        TR("create-account-backend-info-template")
            .arg(networkStr)
            .arg(networkMatch
                     ? QString()
                     : TR("create-account-network-mismatch-suffix").arg(m_network_combo->currentText()))
            .arg(info.height)
            .arg(yn(info.tweaks_only))
            .arg(yn(info.tweaks_full_basic))
            .arg(yn(info.tweaks_full_with_dust_filter))
            .arg(yn(info.tweaks_cut_through_with_dust_filter));

    auto *modal = new qontrol::Modal(TR("settings-backend-info"), msg);
    modal->setFixedSize(300, 230);
    AppController::execModal(modal);
}

void CreateAccount::applyRegtestDefaults() {
    auto network = static_cast<Network>(m_network_combo->currentData().toInt());
    if (network != Network::Regtest) {
        return;
    }

    auto defaults = AppController::get()->regtestDefaults();
    if (!defaults.has_value()) {
        return;
    }

    m_blindbit_input->setText(defaults.value().blindbit_url);
    m_p2p_input->setText(defaults.value().p2p_node);
    m_electrum_input->setText(defaults.value().electrum_url);
    m_backend_verified = true;
    m_p2p_verified = true;
    m_electrum_verified = true;
    onUpdateCreateButton();
}

void CreateAccount::invalidateBackendTest() {
    m_backend_verified = false;
    onUpdateCreateButton();
}

void CreateAccount::onTestP2p() {
    auto addr = m_p2p_input->text().trimmed();
    if (addr.isEmpty()) {
        AppController::execModal(
            new qontrol::Modal(TR("create-account-invalid-input"), TR("create-account-p2p-empty")));
        return;
    }

    m_test_p2p_btn->setEnabled(false);
    m_test_p2p_btn->setText(TR("common-testing"));

    auto network = static_cast<Network>(m_network_combo->currentData().toInt());
    auto *thread = QThread::create([this, addr = addr.toStdString(), network]() {
        auto result = ::test_p2p_node(rust::String(addr), network);
        emit p2pTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void CreateAccount::onP2pTestReady(ConnectionResult result) {
    m_test_p2p_btn->setEnabled(true);
    m_test_p2p_btn->setText(TR("common-test"));

    if (!result.is_ok) {
        m_p2p_verified = false;
        onUpdateCreateButton();
        auto rawError = QString::fromStdString(std::string(result.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-p2p-test-failed"), message));
        return;
    }

    m_p2p_verified = true;
    onUpdateCreateButton();
}

void CreateAccount::invalidateP2pTest() {
    m_p2p_verified = false;
    onUpdateCreateButton();
}

void CreateAccount::onTestElectrum() {
    auto addr = m_electrum_input->text().trimmed();
    if (addr.isEmpty()) {
        AppController::execModal(
            new qontrol::Modal(TR("create-account-invalid-input"), TR("create-account-electrum-empty")));
        return;
    }

    m_test_electrum_btn->setEnabled(false);
    m_test_electrum_btn->setText(TR("common-testing"));

    auto *thread = QThread::create([this, addr = addr.toStdString()]() {
        auto result = ::test_electrum(rust::String(addr));
        emit electrumTestReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void CreateAccount::onElectrumTestReady(ConnectionResult result) {
    m_test_electrum_btn->setEnabled(true);
    m_test_electrum_btn->setText(TR("common-test"));

    if (!result.is_ok) {
        m_electrum_verified = false;
        onUpdateCreateButton();
        auto rawError = QString::fromStdString(std::string(result.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-electrum-test-failed"), message));
        return;
    }

    m_electrum_verified = true;
    onUpdateCreateButton();
}

void CreateAccount::invalidateElectrumTest() {
    m_electrum_verified = false;
    onUpdateCreateButton();
}

void CreateAccount::onUpdateCreateButton() {
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

    m_create_btn->setEnabled(m_backend_verified && m_p2p_verified && m_electrum_verified);
}

auto CreateAccount::generateMnemonic() -> QString { // NOLINT
    auto mnemonic = ::generate_mnemonic();
    return QString::fromStdString(std::string(mnemonic));
}

} // namespace modal

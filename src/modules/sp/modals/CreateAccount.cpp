#include "CreateAccount.h"
#include "views/utils.h"
#include "AppController.h"
#include "common.h"
#include "i18n/Tr.h"
#include "catalog/Button.h"
#include "catalog/inputs/ComboBox.h"
#include "catalog/inputs/Input.h"
#include "catalog/display/Label.h"
#include "catalog/form/ValidationMark.h"
#include "catalog/inputs/TextEdit.h"
#include <qthread.h>

namespace modal {

using catalog::Button;
using catalog::ButtonRole;
using catalog::ComboBox;
using catalog::Input;
using catalog::InputRole;
using catalog::Label;
using catalog::LabelRole;
using catalog::TextEdit;
using catalog::ValidationMark;

/// Default endpoints for a new mainnet account. Both are prefilled without a
/// scheme so the test resolves it: http/https for blindbit, tcp/ssl for electrum.
constexpr auto MAINNET_BLINDBIT_URL = "blindbit.pythcoiner.dev";
constexpr auto MAINNET_ELECTRUM_URL = "electrum.pythcoiner.dev:50002";

CreateAccount::CreateAccount([[maybe_unused]] QWidget *parent) {
    setWindowTitle(TR("create-account-title"));
    init();
    doConnect();
    view();
    applyNetworkDefaults();
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

    m_test_btn = new Button(TR("common-test"));
    m_backend_status = new ValidationMark;

    m_electrum_input = new Input;
    m_electrum_input->setPlaceholderText(TR("settings-placeholder-electrum"));

    m_test_electrum_btn = new Button(TR("common-test"));
    m_electrum_status = new ValidationMark;

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
    connect(m_test_electrum_btn, &QPushButton::clicked, this, &CreateAccount::onTestElectrum,
            qontrol::UNIQUE);
    connect(m_electrum_input, &QLineEdit::textChanged, this, &CreateAccount::invalidateElectrumTest,
            qontrol::UNIQUE);
    connect(m_create_btn, &QPushButton::clicked, this, &CreateAccount::onCreate, qontrol::UNIQUE);
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
    connect(this, &CreateAccount::backendInfoReady, this, &CreateAccount::onBackendInfoReady,
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
                       ->push(m_test_btn)
                       ->pushSpacer(resolve(Spacing::XS))
                       ->push(m_backend_status);

    auto *electrumRow = (new qontrol::Row)
                            ->push(new Label(TR("create-account-electrum"), LabelRole::InputLabel))
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_electrum_input)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_test_electrum_btn)
                            ->pushSpacer(resolve(Spacing::XS))
                            ->push(m_electrum_status);

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
    auto electrumUrl = m_electrum_input->text().trimmed();
    auto network = static_cast<Network>(m_network_combo->currentData().toInt());

    auto config =
        new_config(rust::String(name.toStdString()), network, rust::String(mnemonic.toStdString()),
                   rust::String(blindbitUrl.toStdString()), rust::String(electrumUrl.toStdString()),
                   546, rust::String("sp"));
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
    invalidateElectrumTest();
    applyNetworkDefaults();
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
    m_backend_status->setState(ValidationMark::State::None);

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
        m_backend_status->setState(ValidationMark::State::Invalid);
        onUpdateCreateButton();
        auto rawError = QString::fromStdString(std::string(info.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-connection-failed"), message));
        return;
    }

    auto selectedNetwork = static_cast<Network>(m_network_combo->currentData().toInt());
    bool networkMatch = (info.network == selectedNetwork);

    m_blindbit_input->setText(QString::fromStdString(std::string(info.url.c_str())));

    m_backend_verified = networkMatch;
    m_backend_status->setState(networkMatch ? ValidationMark::State::Valid
                                            : ValidationMark::State::Invalid);
    onUpdateCreateButton();
}

void CreateAccount::applyNetworkDefaults() {
    switch (static_cast<Network>(m_network_combo->currentData().toInt())) {
    case Network::Regtest:
        applyRegtestDefaults();
        break;
    case Network::Bitcoin:
        // Prefilled but left untested: only a passing test resolves the electrum
        // scheme, and the create button gates on it anyway.
        m_blindbit_input->setText(MAINNET_BLINDBIT_URL);
        m_electrum_input->setText(MAINNET_ELECTRUM_URL);
        break;
    default:
        break;
    }
}

void CreateAccount::applyRegtestDefaults() {
    auto defaults = AppController::get()->regtestDefaults();
    if (!defaults.has_value()) {
        return;
    }

    m_blindbit_input->setText(defaults.value().blindbit_url);
    m_electrum_input->setText(defaults.value().electrum_url);
    m_backend_verified = true;
    m_electrum_verified = true;
    m_backend_status->setState(ValidationMark::State::Valid);
    m_electrum_status->setState(ValidationMark::State::Valid);
    onUpdateCreateButton();
}

void CreateAccount::invalidateBackendTest() {
    m_backend_verified = false;
    m_backend_status->setState(ValidationMark::State::None);
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
    m_electrum_status->setState(ValidationMark::State::None);

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
        m_electrum_status->setState(ValidationMark::State::Invalid);
        onUpdateCreateButton();
        auto rawError = QString::fromStdString(std::string(result.error.c_str()));
        auto message = mapBackendErrorSummary(rawError) + "\n\n" + formatBackendErrorDetails(rawError);
        AppController::execModal(new qontrol::Modal(TR("settings-electrum-test-failed"), message));
        return;
    }

    // Before setting verified: setText fires textChanged -> invalidateElectrumTest.
    m_electrum_input->setText(QString::fromStdString(std::string(result.url.c_str())));

    m_electrum_verified = true;
    m_electrum_status->setState(ValidationMark::State::Valid);
    onUpdateCreateButton();
}

void CreateAccount::invalidateElectrumTest() {
    m_electrum_verified = false;
    m_electrum_status->setState(ValidationMark::State::None);
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

    m_create_btn->setEnabled(m_backend_verified && m_electrum_verified);
}

auto CreateAccount::generateMnemonic() -> QString { // NOLINT
    auto mnemonic = ::generate_mnemonic();
    return QString::fromStdString(std::string(mnemonic));
}

} // namespace modal

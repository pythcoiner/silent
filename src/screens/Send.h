#pragma once

#include <Qontrol>
#include <cstdint>
#include <optional>
#include <qcheckbox.h>
#include <qhash.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include <qstringlist.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;
namespace modal {
class ConfirmSend;
}

namespace screen {
class Send;

class InputW {
public:
    InputW(const RustCoin &coin);
    auto widget() -> QWidget *;
    static const int OUTPOINT_WIDTH = 180;
    static const int LABEL_WIDTH = 420;
    static const int VALUE_WIDTH = 150;

private:
    QWidget *m_widget = nullptr;
};

class OutputW {
public:
    OutputW(Send *screen, int id);
    auto widget() -> QWidget *;
    auto setDeletable(bool deletable) -> void;
    auto enableMax(bool max) -> void;
    auto isMax() -> bool;
    auto address() -> QString;
    auto amount() -> std::optional<uint64_t>;
    auto label() -> QString;
    auto updateAddressValidation() -> void;
    auto updateAmountValidation() -> void;
    auto setAmountVisible(bool visible) -> void;
    auto clearAmount() -> void;

private:
    QLineEdit *m_address = nullptr;
    QLineEdit *m_label = nullptr;
    QLineEdit *m_amount = nullptr;
    QPushButton *m_delete_btn = nullptr;
    QWidget *m_delete_spacer = nullptr;
    QCheckBox *m_max = nullptr;
    QLabel *m_max_label = nullptr;
    QLabel *m_address_indicator = nullptr;
    QLabel *m_amount_indicator = nullptr;
    QWidget *m_amount_spacer = nullptr;
    QWidget *m_widget = nullptr;
};

class Send : public qontrol::Screen {
    Q_OBJECT
public:
    Send(AccountController *ctrl);

signals:
    void signReady(TxResult result);
    void broadcastReady(TxResult result);

public slots:
    auto outputSetMax(int id) -> void;
    auto deleteOutput(int id) -> void;
    auto addOutput() -> void;
    auto onOutputDeleteClicked() -> void;
    auto onOutputMaxToggled() -> void;
    auto clearOutputs() -> void;
    auto clearInputs() -> void;
    auto onFeeToggled() -> void;
    auto setBroadcastable(bool broadcastable) -> void;
    auto onCoinToggled() -> void;
    auto onAutoSelectionToggled() -> void;
    auto updateInputsTotal() -> void;
    auto updateOutputValidations() -> void;
    auto updateFeeValidation() -> void;
    auto updateSelectedCoinsFromSimulation() -> void;
    auto setSpendable(bool spendable) -> void;
    auto process() -> void;
    auto sendTransaction() -> void;
    auto onSendConfirmed() -> void;
    auto onSignResult(TxResult result) -> void;
    auto onBroadcastResult(TxResult result) -> void;
    auto onCoinsUpdated(CoinState state) -> void;

protected:
    auto init() -> void override;
    auto doConnect() -> void override;
    auto view() -> void override;
    auto outputsView() -> QWidget *;
    auto inputsView() -> QWidget *;
    auto txTemplate() -> std::optional<TransactionTemplate>;
    auto output() -> QWidget *;
    auto updateCoinCheckboxes() -> void;
    auto updateInputsTitle() -> void;

private:
    AccountController *m_controller = nullptr;
    int m_output_id = 0;
    QHash<int, OutputW *> m_outputs;
    qontrol::Column *m_outputs_column = nullptr;
    qontrol::Column *m_inputs_column = nullptr;

    QWidget *m_main_widget = nullptr;
    QWidget *m_outputs_frame = nullptr;
    QWidget *m_inputs_frame = nullptr;

    qontrol::widgets::ToggleSwitch *m_fee_toggle = nullptr;
    QLineEdit *m_fee_value = nullptr;
    QLabel *m_fee_label = nullptr;
    QLabel *m_fee_indicator = nullptr;
    qontrol::Row *m_fee_row = nullptr;

    QLabel *m_warning_label = nullptr;
    QLabel *m_inputs_title = nullptr;

    QPushButton *m_add_output_btn = nullptr;
    QScrollArea *m_coins_scroll = nullptr;

    qontrol::Row *m_inputs_total_row = nullptr;
    QLineEdit *m_inputs_total = nullptr;
    qontrol::Row *m_inputs_min_row = nullptr;
    QLineEdit *m_inputs_min = nullptr;
    QCheckBox *m_auto_coin_selection = nullptr;

    QPushButton *m_clear_outputs_btn = nullptr;
    QPushButton *m_clear_inputs_btn = nullptr;
    QPushButton *m_send_btn = nullptr;

    QLabel *m_fee_estimate_label = nullptr;

    bool m_broadcastable = false;
    std::optional<rust::Box<PsbtResult>> m_psbt_result = std::nullopt;
    std::optional<TransactionTemplate> m_tx_template = std::nullopt;
    QString m_signed_tx_hex;
    QList<RustCoin> m_selected_coins;
    QStringList m_auto_selected_outpoints;
    QHash<QString, QCheckBox *> m_coin_checkboxes;
    CoinState m_last_coin_state{};
    modal::ConfirmSend *m_confirm_modal = nullptr;
};

} // namespace screen

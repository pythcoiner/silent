#pragma once

#include "theme/Palette.h"
#include <Qontrol>
#include <cstdint>
#include <optional>
#include <qhash.h>
#include <qscrollarea.h>

namespace theme {
class Button;
}

namespace theme {
class Checkbox;
}

namespace theme {
class Display;
}

namespace theme {
class Input;
}

namespace theme {
class Label;
}

namespace theme {
class Toggle;
}

class QCheckBox;
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
    static const int TYPE_WIDTH = resolve(Size::XXS);
    static const int OUTPOINT_WIDTH = resolve(Size::M);
    static const int LABEL_WIDTH = resolve(Size::XL);
    static const int VALUE_WIDTH = resolve(Size::S);

private:
    QWidget *m_widget = nullptr;
};

class OutputW {
public:
    OutputW(Send *screen, int id);
    auto widget() -> QWidget *;
    void setDeletable(bool deletable);
    void enableMax(bool max);
    auto isMax() -> bool;
    auto address() -> QString;
    auto amount() -> std::optional<uint64_t>;
    auto label() -> QString;
    void updateAddressValidation();
    void updateAmountValidation();
    void setMaxMode(bool max);
    void setMaxAmount(uint64_t sats);

private:
    theme::Input *m_address_input = nullptr;
    theme::Input *m_label_input = nullptr;
    theme::Input *m_amount_input = nullptr;
    theme::Label *m_address_indicator = nullptr;
    theme::Label *m_amount_indicator = nullptr;
    theme::Button *m_delete_btn = nullptr;
    QWidget *m_delete_spacer = nullptr;
    theme::Checkbox *m_max = nullptr;
    theme::Label *m_max_label = nullptr;
    QWidget *m_widget = nullptr;
};

class Send : public qontrol::Screen {
    Q_OBJECT
public:
    Send(AccountController *ctrl);

signals:
    void validationReady(PsbtValidation result);
    void signReady(TxResult result);
    void broadcastReady(TxResult result);

public slots:
    void onOutputSetMax(int id);
    void onDeleteOutput(int id);
    void onAddOutput();
    void onOutputDeleteClicked();
    void onOutputMaxToggled();
    void onClearOutputs();
    void onClearInputs();
    void onFeeToggled();
    void onSetBroadcastable(bool broadcastable);
    void onCoinToggled();
    void onAutoSelectionToggled();
    void onUpdateInputsTotal();
    void onUpdateOutputValidations();
    void onUpdateFeeValidation();
    void onUpdateSelectedCoinsFromSimulation();
    void onSetSpendable(bool spendable);
    void onProcess();
    void onSendTransaction();
    void onValidationResult(PsbtValidation result);
    void onSendConfirmed();
    void onSignResult(TxResult result);
    void onBroadcastResult(TxResult result);
    void onCoinsUpdated(CoinState state);

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    auto outputsView() -> QWidget *;
    auto inputsView() -> QWidget *;
    auto txTemplate() -> std::optional<TransactionTemplate>;
    auto output() -> QWidget *;
    void updateCoinCheckboxes();
    void updateInputsTitle();

private:
    AccountController *m_controller = nullptr;
    int m_output_id = 0;
    QHash<int, OutputW *> m_outputs;
    qontrol::Column *m_outputs_column = nullptr;
    qontrol::Column *m_inputs_column = nullptr;

    QWidget *m_main_widget = nullptr;
    QWidget *m_outputs_frame = nullptr;
    QWidget *m_inputs_frame = nullptr;

    theme::Toggle *m_fee_toggle = nullptr;
    theme::Input *m_fee_value_input = nullptr;
    theme::Label *m_fee_label = nullptr;
    theme::Label *m_fee_indicator = nullptr;

    theme::Label *m_warning_label = nullptr;
    theme::Label *m_inputs_title_label = nullptr;

    theme::Button *m_add_output_btn = nullptr;
    QScrollArea *m_coins_scroll = nullptr;

    qontrol::Row *m_inputs_total_input_row = nullptr;
    theme::Display *m_inputs_total_input = nullptr;
    qontrol::Row *m_inputs_min_input_row = nullptr;
    theme::Display *m_inputs_min_input = nullptr;
    theme::Checkbox *m_auto_coin_selection = nullptr;

    theme::Button *m_clear_outputs_btn = nullptr;
    theme::Button *m_clear_inputs_btn = nullptr;
    theme::Button *m_send_btn = nullptr;

    theme::Label *m_fee_estimate_label = nullptr;

    bool m_broadcastable = false;
    std::optional<rust::Box<PsbtResult>> m_psbt_result = std::nullopt;
    std::optional<TransactionTemplate> m_tx_template = std::nullopt;
    QString m_signed_tx_hex;
    QList<RustCoin> m_selected_coins{};
    QStringList m_auto_selected_outpoints;
    QHash<QString, theme::Checkbox *> m_coin_checkboxes;
    CoinState m_last_coin_state{};
    modal::ConfirmSend *m_confirm_modal = nullptr;
};

} // namespace screen

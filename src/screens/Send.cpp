#include "Send.h"
#include "AccountController.h"
#include "AppController.h"
#include "modals/ConfirmSend.h"
#include "theme/Button.h"
#include "theme/Checkbox.h"
#include "theme/Display.h"
#include "theme/Icon.h"
#include "theme/Input.h"
#include "theme/Label.h"
#include "theme/Palette.h"
#include "theme/Theme.h"
#include "theme/Toggle.h"
#include "utils.h"
#include <QApplication>
#include <QDebug>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QThread>
#include <Qontrol>
#include <algorithm>
#include <common.h>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <qcheckbox.h>
#include <qcontainerfwd.h>
#include <string>

namespace screen {

using theme::Button;
using theme::ButtonRole;
using theme::Checkbox;
using theme::Display;
using theme::DisplayRole;
using theme::Input;
using theme::InputRole;
using theme::Label;
using theme::LabelRole;
using theme::Toggle;

static void setValidationIndicator(Label *indicator, const QString &text, bool valid) {
    if (text.isEmpty()) {
        indicator->setText("");
        indicator->setStyleSheet("");
    } else {
        if (valid) {
            indicator->setText(QString::fromUtf8("\xe2\x9c\x93"));
            indicator->setStyleSheet("QLabel { color: green; }");
        } else {
            indicator->setText(QString::fromUtf8("\xe2\x9c\x97"));
            indicator->setStyleSheet("QLabel { color: red; }");
        }
    }
}

InputW::InputW(const RustCoin &coin) {
    auto *typeLabel =
        new Label(QString::fromUtf8(coin.account_type.data(), coin.account_type.size()));

    auto *outpointLabel =
        new Label(QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size()), LabelRole::Mono);

    auto *valueLabel = new Label(toBitcoin(coin.value), LabelRole::Mono);
    valueLabel->setAlignment(Qt::AlignRight);

    auto *labelLabel = new Label(QString::fromUtf8(coin.label.data(), coin.label.size()));

    auto *row = (new qontrol::Row)
                    ->push(typeLabel)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(outpointLabel)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(valueLabel)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(labelLabel)
                    ->pushSpacer();

    m_widget = row;
}

auto InputW::widget() -> QWidget * {
    return m_widget;
}

OutputW::OutputW(Send *screen, int id) {
    m_address_input = new Input(InputRole::Mono);
    m_address_input->setWidth(Size::XL);
    m_address_input->setPlaceholderText("Address");

    m_address_indicator = new Label(LabelRole::Caption);
    m_address_indicator->setFixedWidth(20);
    m_address_indicator->setAlignment(Qt::AlignCenter);

    m_delete_btn = new Button(ButtonRole::InlineIcon);
    m_delete_btn->setIcon(icon::close());

    m_delete_spacer = new QWidget;
    m_delete_spacer->setFixedWidth(resolve(Spacing::XS));

    m_amount_input = new Input(InputRole::Mono);
    m_amount_input->setWidth(Size::S);
    m_amount_input->setPlaceholderText("0.002 BTC");

    m_amount_indicator = new Label(LabelRole::Caption);
    m_amount_indicator->setFixedWidth(20);
    m_amount_indicator->setAlignment(Qt::AlignCenter);

    m_label_input = new Input;
    m_label_input->setWidth(Size::XL);
    m_label_input->setPlaceholderText("Label");

    m_max = new Checkbox;
    QObject::connect(m_max, &Checkbox::toggled, screen, &Send::process, qontrol::UNIQUE);

    m_max_label = new Label("MAX", LabelRole::CheckboxLabel);

    // Connect to process() which handles validation updates
    QObject::connect(m_address_input, &QLineEdit::textChanged, screen, &Send::process,
                     qontrol::UNIQUE);
    QObject::connect(m_amount_input, &QLineEdit::textChanged, screen, &Send::process,
                     qontrol::UNIQUE);

    auto *addrRow = (new qontrol::Row)
                        ->push(m_delete_btn)
                        ->push(m_delete_spacer)
                        ->push(m_address_input)
                        ->push(m_address_indicator)
                        ->push(m_amount_input)
                        ->push(m_amount_indicator)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(m_max)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(m_max_label)
                        ->pushSpacer();

    auto *labelRow = (new qontrol::Row)->push(m_label_input)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(addrRow)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(labelRow)
                    ->pushSpacer(resolve(Spacing::S));

    m_delete_btn->setProperty("outputId", id);
    QObject::connect(m_delete_btn, &QPushButton::clicked, screen, &Send::onOutputDeleteClicked,
                     qontrol::UNIQUE);

    m_max->setProperty("outputId", id);
    QObject::connect(m_max, &Checkbox::checkStateChanged, screen, &Send::onOutputMaxToggled,
                     qontrol::UNIQUE);

    m_widget = col;
}

auto OutputW::widget() -> QWidget * {
    return m_widget;
}

void OutputW::setDeletable(bool deletable) {
    m_delete_btn->setVisible(deletable);
    m_delete_spacer->setVisible(deletable);
}

void OutputW::enableMax(bool max) {
    m_max->setChecked(false);
    m_max->setVisible(max);
    m_max_label->setVisible(max);
}

void OutputW::setMaxMode(bool max) {
    m_amount_input->setEnabled(!max);
    if (max) {
        m_amount_input->setText("MAX");
        setValidationIndicator(m_amount_indicator, "", false);
    } else {
        if (m_amount_input->text() == "MAX") {
            m_amount_input->setText("");
        }
        m_amount_input->setPlaceholderText("0.002 BTC");
    }
}

void OutputW::setMaxAmount(uint64_t sats) {
    m_amount_input->setText(toBitcoin(sats, false));
}

auto OutputW::isMax() -> bool {
    return m_max->isChecked();
}

void Send::onFeeToggled() {
    qDebug() << "Send::onFeeToggled()";
    if (m_fee_toggle->isChecked()) {
        m_fee_label->setText("sats/vb");
        // sats/vb: 3 decimal places max (milli-sats precision)
        auto *validator = new QDoubleValidator(0.001, 1000000.0, 3, m_fee_value_input);
        validator->setNotation(QDoubleValidator::StandardNotation);
        m_fee_value_input->setValidator(validator);
    } else {
        m_fee_label->setText("sats");
        // sats: integers only
        m_fee_value_input->setValidator(new QIntValidator(1, 100000000, m_fee_value_input));
    }
    process();
}

Send::Send(AccountController *ctrl) {
    qDebug() << "Send::Send()";
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->addOutput();
    this->view();
    this->onFeeToggled();
    this->setBroadcastable(false);
}

void Send::init() {
    qDebug() << "Send::init()";
    m_outputs_column = (new qontrol::Column);
    m_inputs_column = (new qontrol::Column);

    m_add_output_btn = new Button("+ Add an Output");
    m_send_btn = new Button("Send", ButtonRole::Primary);
    m_clear_outputs_btn = new Button("Clear");

    m_fee_estimate_label = new Label();
    m_fee_estimate_label->setVisible(false);

    // Fee toggle: OFF = sats (absolute), ON = sats/vb (rate)
    m_fee_toggle = new Toggle;
    m_fee_toggle->setChecked(true); // Default to sats/vb mode

    m_fee_value_input = new Input(InputRole::Mono);
    m_fee_value_input->setWidth(Size::XS);
    m_fee_value_input->setText("1");

    m_fee_indicator = new Label(LabelRole::Caption);
    m_fee_indicator->setFixedWidth(20);
    m_fee_indicator->setAlignment(Qt::AlignCenter);

    m_fee_label = new Label("sats/vb");
    m_fee_label->setFixedWidth(m_fee_label->fontMetrics().horizontalAdvance("sats/vb") + 5);

    m_warning_label = new Label();
    m_warning_label->setVisible(false);

    // Calculate label width to align with coin value column
    // checkbox + spacer + type + spacer + outpoint + spacer + label = total label width
    // Inputs total row
    auto *totalLabel = new Label("Total selected:", LabelRole::InfoLabel);
    totalLabel->setAlignment(Qt::AlignRight);
    m_inputs_total_input = new Display(DisplayRole::Amount);
    m_inputs_total_input->setWidth(Size::S);
    m_inputs_total_input_row = (new qontrol::Row)
                                   ->push(totalLabel)
                                   ->pushSpacer(resolve(Spacing::XS))
                                   ->push(m_inputs_total_input)
                                   ->pushSpacer();
    m_inputs_total_input_row->setVisible(false);

    // Inputs minimum row
    auto *minLabel = new Label("Amount to select:", LabelRole::InfoLabel);
    minLabel->setAlignment(Qt::AlignRight);
    m_inputs_min_input = new Display(DisplayRole::Amount);
    m_inputs_min_input->setWidth(Size::S);
    m_inputs_min_input_row = (new qontrol::Row)
                                 ->push(minLabel)
                                 ->pushSpacer(resolve(Spacing::XS))
                                 ->push(m_inputs_min_input)
                                 ->pushSpacer();
    m_inputs_min_input_row->setVisible(false);

    m_auto_coin_selection = new Checkbox("Auto coin selection");
    m_auto_coin_selection->setChecked(true);

    m_clear_inputs_btn = new Button("Clear");

    m_inputs_title_label = new Label("Select Inputs", LabelRole::Heading);
    // We avoid the height of the row chnage when "Clear" button show/hidden
    auto minHeight = Theme::get()->buttonPalette().defaultBtn.height + (2 * resolve(Padding::L));
    m_inputs_title_label->setMinimumHeight(minHeight);
}

void Send::doConnect() {
    qDebug() << "Send::doConnect()";
    connect(m_add_output_btn, &QPushButton::clicked, this, &Send::addOutput, qontrol::UNIQUE);
    connect(m_send_btn, &QPushButton::clicked, this, &Send::sendTransaction, qontrol::UNIQUE);
    connect(m_clear_outputs_btn, &QPushButton::clicked, this, &Send::clearOutputs, qontrol::UNIQUE);
    connect(m_clear_inputs_btn, &QPushButton::clicked, this, &Send::clearInputs, qontrol::UNIQUE);
    connect(m_auto_coin_selection, &Checkbox::toggled, this, &Send::onAutoSelectionToggled,
            qontrol::UNIQUE);
    connect(m_controller, &AccountController::updateCoins, this, &Send::onCoinsUpdated,
            qontrol::UNIQUE);
    connect(m_fee_toggle, &QCheckBox::toggled, this, &Send::onFeeToggled, qontrol::UNIQUE);
    connect(m_fee_value_input, &QLineEdit::textChanged, this, &Send::process, qontrol::UNIQUE);
    connect(this, &Send::validationReady, this, &Send::onValidationResult, qontrol::UNIQUE);
    connect(this, &Send::signReady, this, &Send::onSignResult, qontrol::UNIQUE);
    connect(this, &Send::broadcastReady, this, &Send::onBroadcastResult, qontrol::UNIQUE);
}

void Send::view() {
    qDebug() << "Send::view()";
    // Save focus state before rebuild (QPointer auto-nulls if widget is deleted)
    QPointer<QWidget> focusedWidget = QApplication::focusWidget();
    int cursorPos = 0;
    if (auto *lineEdit = qobject_cast<QLineEdit *>(focusedWidget.data())) {
        cursorPos = lineEdit->cursorPosition();
    }

    auto *oldOutputs = m_outputs_frame;
    m_outputs_frame = frame(outputsView());
    delete oldOutputs;

    auto *oldInputs = m_inputs_frame;
    m_inputs_frame = frame(inputsView());
    delete oldInputs;

    auto *col = (new qontrol::Column)
                    ->push(m_inputs_frame)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_outputs_frame);

    auto *oldWidget = m_main_widget;
    m_main_widget = margin(col, resolve(Spacing::S));
    delete oldWidget;
    delete this->layout();
    this->setLayout(m_main_widget->layout());

    // Restore focus state after rebuild (QPointer is null if widget was deleted)
    if (!focusedWidget.isNull()) {
        focusedWidget->setFocus();
        if (auto *lineEdit = qobject_cast<QLineEdit *>(focusedWidget.data())) {
            lineEdit->setCursorPosition(cursorPos);
        }
    }
}

auto Send::outputsView() -> QWidget * {
    qDebug() << "Send::outputsView()";
    auto *oldColumn = m_outputs_column;

    m_outputs_column = new qontrol::Column;

    auto keys = QList<int>();
    for (auto id : m_outputs.keys()) {
        keys.push_back(id);
    }
    std::ranges::sort(keys);
    for (auto id : keys) {
        auto *output = m_outputs.value(id);
        m_outputs_column->push(output->widget());
    }
    delete oldColumn;

    auto *lastRow = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(m_clear_outputs_btn)
                        ->pushSpacer(resolve(Size::XS))
                        ->push(m_send_btn)
                        ->pushSpacer();

    auto *warningRow = (new qontrol::Row)->push(m_warning_label)->pushSpacer();

    auto *title = new Label("Recipients", LabelRole::Heading);

    auto *titleRow = (new qontrol::Row)->pushSpacer(resolve(Padding::M))->push(title)->pushSpacer();

    auto *feeRow = (new qontrol::Row)
                       ->push(m_fee_toggle)
                       ->pushSpacer(resolve(Spacing::S))
                       ->push(m_fee_value_input)
                       ->push(m_fee_indicator)
                       ->pushSpacer(resolve(Spacing::S))
                       ->push(m_fee_label)
                       ->pushSpacer(resolve(Spacing::S))
                       ->push(m_fee_estimate_label)
                       ->pushSpacer(resolve(Size::S))
                       ->push(m_add_output_btn)
                       ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->push(titleRow)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(m_outputs_column)
                    ->push(feeRow)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(warningRow)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(lastRow)
                    ->pushSpacer();

    return col;
}

auto Send::inputsView() -> QWidget * {
    qDebug() << "Send::inputsView()";
    auto availableCoins = m_controller->getCoins();

    // Unparent persistent rows
    m_inputs_total_input_row->setParent(nullptr);
    m_inputs_min_input_row->setParent(nullptr);

    // Clear old checkbox references
    m_coin_checkboxes.clear();

    bool autoMode = m_auto_coin_selection->isChecked();

    auto *coinsColumn = new qontrol::Column;
    for (const auto &coin : availableCoins) {
        auto *checkbox = new Checkbox();
        QString outpoint = QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size());
        checkbox->setProperty("outpoint", outpoint);
        checkbox->setEnabled(!autoMode);

        // Store reference for later updates
        m_coin_checkboxes.insert(outpoint, checkbox);

        // Determine selection state based on mode
        bool isSelected = false;
        if (autoMode) {
            // In auto mode, use the auto-selected outpoints from last simulation
            isSelected = m_auto_selected_outpoints.contains(outpoint);
        } else {
            // In manual mode, use m_selected_coins
            for (const auto &selected : m_selected_coins) {
                if (selected.outpoint == coin.outpoint) {
                    isSelected = true;
                    break;
                }
            }
        }
        checkbox->setChecked(isSelected);
        connect(checkbox, &Checkbox::checkStateChanged, this, &Send::onCoinToggled, qontrol::UNIQUE);

        QString typeStr = QString::fromUtf8(coin.account_type.data(), coin.account_type.size());
        auto *typeField = new Display(typeStr);
        typeField->setWidth(Size::XS);

        auto *outpointField = new Display(shortenOutpoint(outpoint), DisplayRole::Outpoint);
        outpointField->setWidth(Size::M);

        QString label = QString::fromUtf8(coin.label.data(), coin.label.size());
        auto *labelField = new Display(label);
        labelField->setWidth(Size::XL);

        auto *valueField = new Display(toBitcoin(coin.value), DisplayRole::Amount);
        valueField->setWidth(Size::S);

        auto *row = (new qontrol::Row)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(checkbox)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(typeField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(outpointField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(labelField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(valueField)
                        ->pushSpacer();
        coinsColumn->pushSpacer(resolve(Spacing::XS))->push(row);
    }

    m_coins_scroll = new QScrollArea;
    m_coins_scroll->setWidget(coinsColumn);
    m_coins_scroll->setFixedHeight(resolve(Size::M));
    m_coins_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_auto_coin_selection->setParent(nullptr);
    m_inputs_title_label->setParent(nullptr);
    m_clear_inputs_btn->setParent(nullptr);

    // Only show Clear button when in manual mode
    m_clear_inputs_btn->setVisible(!m_auto_coin_selection->isChecked());

    auto *titleRow = (new qontrol::Row)
                         ->pushSpacer(resolve(Padding::M))
                         ->push(m_inputs_title_label)
                         ->pushSpacer()
                         ->push(m_clear_inputs_btn)
                         ->pushSpacer(resolve(Spacing::S))
                         ->push(m_auto_coin_selection)
                         ->pushSpacer(resolve(Padding::M));

    // Update total and minimum displays
    updateInputsTotal();

    auto *col = (new qontrol::Column)
                    ->push(titleRow)
                    ->pushSpacer(resolve(Spacing::S))
                    ->push(m_coins_scroll)
                    ->pushSpacer(resolve(Spacing::XS))
                    ->push(m_inputs_min_input_row)
                    ->push(m_inputs_total_input_row)
                    ->pushSpacer();

    return col;
}

void Send::addOutput() {
    qDebug() << "Send::addOutput()";
    auto *output = new OutputW(this, m_output_id);
    if (m_outputs.empty()) {
        output->setDeletable(false);
    } else {
        for (auto &out : m_outputs) {
            out->setDeletable(true);
        }
    }
    for (auto *outp : m_outputs) {
        if (outp->isMax()) {
            output->enableMax(false);
            break;
        }
    }
    m_outputs.insert(m_output_id, output);
    m_outputs_column->push(output->widget());
    m_output_id++;
    process();
    view();
}

void Send::deleteOutput(int id) {
    qDebug() << "Send::deleteOutput()" << id;
    auto *output = m_outputs.take(id);
    if (output->isMax()) {
        for (auto *outp : m_outputs) {
            outp->enableMax(true);
        }
    }
    delete output->widget();
    delete output;
    if (m_outputs.size() == 1) {
        auto *outp = m_outputs.value(m_outputs.keys().first());
        outp->setDeletable(false);
        if (!outp->isMax()) {
            outp->enableMax(true);
        }
    }
    process();
    view();
}

void Send::outputSetMax(int id) {
    qDebug() << "Send::outputSetMax()" << id;
    if (m_outputs.value(id)->isMax()) {
        for (auto &key : m_outputs.keys()) {
            if (key != id) {
                m_outputs.value(key)->enableMax(false);
            }
        }
    } else {
        for (auto &key : m_outputs.keys()) {
            m_outputs.value(key)->enableMax(true);
        }
    }
    process();
}

void Send::setBroadcastable(bool broadcastable) {
    qDebug() << "Send::setBroadcastable()" << broadcastable;
    m_broadcastable = broadcastable;
    m_send_btn->setEnabled(broadcastable);
}

void Send::clearOutputs() {
    qDebug() << "Send::clearOutputs()";
    for (auto *outp : m_outputs) {
        delete outp;
    }
    m_outputs.clear();
    addOutput();
    process();
    view();
}

void Send::clearInputs() {
    qDebug() << "Send::clearInputs()";
    m_selected_coins.clear();
    for (auto *cb : m_coin_checkboxes.values()) {
        cb->setChecked(false);
    }
    process();
    view();
}

void Send::onCoinToggled() {
    qDebug() << "Send::onCoinToggled()";
    auto *checkbox = qobject_cast<Checkbox *>(sender());
    if (checkbox == nullptr) {
        return;
    }

    QString outpoint = checkbox->property("outpoint").toString();
    auto availableCoins = m_controller->getCoins();

    if (checkbox->isChecked()) {
        // Add to m_selected_coins if not already present
        for (const auto &coin : availableCoins) {
            if (QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size()) == outpoint) {
                bool alreadySelected = false;
                for (const auto &selected : m_selected_coins) {
                    if (selected.outpoint == coin.outpoint) {
                        alreadySelected = true;
                        break;
                    }
                }
                if (!alreadySelected) {
                    m_selected_coins.append(coin);
                }
                break;
            }
        }
    } else {
        // Remove from m_selected_coins
        for (int i = 0; i < m_selected_coins.size(); ++i) {
            if (QString::fromUtf8(m_selected_coins[i].outpoint.data(),
                                  m_selected_coins[i].outpoint.size()) == outpoint) {
                m_selected_coins.removeAt(i);
                break;
            }
        }
    }
    process();
}

void Send::onAutoSelectionToggled() {
    qDebug() << "Send::onAutoSelectionToggled()";
    if (!m_auto_coin_selection->isChecked()) {
        // Switching to manual: populate m_selected_coins from last auto selection
        updateSelectedCoinsFromSimulation();
    }
    process();
    view();
}

void Send::onCoinsUpdated(CoinState state) {
    qDebug() << "Send::onCoinsUpdated() confirmed:" << state.confirmed_count
             << "unconfirmed:" << state.unconfirmed_count;
    // Only rebuild if coins actually changed
    bool changed = (state.confirmed_count != m_last_coin_state.confirmed_count ||
                    state.unconfirmed_count != m_last_coin_state.unconfirmed_count);

    if (!changed) {
        return;
    }

    m_last_coin_state = state;

    // Rebuild the inputs view to show new/removed coins
    view();

    // If auto coin selection is enabled AND outputs are valid, re-run simulation
    if (m_auto_coin_selection->isChecked()) {
        auto txTemp = txTemplate();
        if (txTemp.has_value()) {
            process();
        }
    }
}

void Send::updateSelectedCoinsFromSimulation() {
    qDebug() << "Send::updateSelectedCoinsFromSimulation()";
    m_selected_coins.clear();
    auto availableCoins = m_controller->getCoins();
    for (const auto &coin : availableCoins) {
        QString op = QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size());
        if (m_auto_selected_outpoints.contains(op)) {
            m_selected_coins.append(coin);
        }
    }
}

void Send::updateInputsTotal() {
    qDebug() << "Send::updateInputsTotal()";
    // Calculate total selected based on mode
    uint64_t totalSelected = 0;

    if (m_auto_coin_selection->isChecked()) {
        // In auto mode, calculate from auto-selected outpoints
        auto availableCoins = m_controller->getCoins();
        for (const auto &coin : availableCoins) {
            QString op = QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size());
            if (m_auto_selected_outpoints.contains(op)) {
                totalSelected += coin.value;
            }
        }
    } else {
        // In manual mode, calculate from m_selected_coins
        for (const auto &coin : m_selected_coins) {
            totalSelected += coin.value;
        }
    }

    if (totalSelected > 0) {
        double fValue = static_cast<double>(totalSelected) / SATS;
        m_inputs_total_input->setText(QString::number(fValue, 'f', 8) + " BTC");
        m_inputs_total_input_row->setVisible(true);
    } else {
        m_inputs_total_input_row->setVisible(false);
    }

    // Calculate minimum required from outputs
    uint64_t minRequired = 0;
    for (auto *out : m_outputs) {
        if (!out->isMax()) {
            auto amt = out->amount();
            if (amt.has_value()) {
                minRequired += amt.value();
            }
        }
    }

    if (minRequired > 0) {
        double fValue = static_cast<double>(minRequired) / SATS;
        m_inputs_min_input->setText(QString::number(fValue, 'f', 8) + " BTC");
        m_inputs_min_input_row->setVisible(true);
    } else {
        m_inputs_min_input_row->setVisible(false);
    }
}

void Send::updateCoinCheckboxes() {
    qDebug() << "Send::updateCoinCheckboxes()";
    bool autoMode = m_auto_coin_selection->isChecked();

    for (auto it = m_coin_checkboxes.begin(); it != m_coin_checkboxes.end(); ++it) {
        QString outpoint = it.key();
        Checkbox *checkbox = it.value();

        bool isSelected = false;
        if (autoMode) {
            isSelected = m_auto_selected_outpoints.contains(outpoint);
        } else {
            for (const auto &coin : m_selected_coins) {
                if (QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size()) == outpoint) {
                    isSelected = true;
                    break;
                }
            }
        }
        checkbox->setChecked(isSelected);
    }
    updateInputsTitle();
}

void Send::updateInputsTitle() {
    qDebug() << "Send::updateInputsTitle()";
    int count = 0;
    if (m_auto_coin_selection->isChecked()) {
        count = m_auto_selected_outpoints.size();
    } else {
        count = m_selected_coins.size();
    }

    if (count == 1) {
        m_inputs_title_label->setText(QString("Inputs (1 coin selected)"));
    } else if (count > 1) {
        m_inputs_title_label->setText(QString("Inputs (%1 coins selected)").arg(count));
    } else {
        m_inputs_title_label->setText("Inputs");
    }
}

auto OutputW::address() -> QString {
    return m_address_input->text();
}

auto OutputW::amount() -> std::optional<uint64_t> {
    auto amountStr = m_amount_input->text();
    bool ok = false;
    auto amountBtc = amountStr.toDouble(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return static_cast<uint64_t>(amountBtc * SATS);
}

auto OutputW::label() -> QString {
    return m_label_input->text();
}

void OutputW::updateAddressValidation() {
    QString addr = m_address_input->text();
    if (addr.isEmpty()) {
        setValidationIndicator(m_address_indicator, addr, false);
    } else {
        auto result = ::validate_address(rust::String(addr.toStdString()));
        setValidationIndicator(m_address_indicator, addr, result.empty());
    }
}

void OutputW::updateAmountValidation() {
    QString text = m_amount_input->text();
    if (text.isEmpty()) {
        setValidationIndicator(m_amount_indicator, text, false);
    } else {
        bool ok = false;
        double val = text.toDouble(&ok);
        setValidationIndicator(m_amount_indicator, text, ok && val > 0);
    }
}

auto Send::txTemplate() -> std::optional<TransactionTemplate> {
    qDebug() << "Send::txTemplate()";
    auto txTemplate = TransactionTemplate();
    bool ok = false;

    // Fee handling: toggle ON = sats/vb (rate), toggle OFF = sats (absolute)
    auto feeValue = m_fee_value_input->text().toDouble(&ok);
    if (!ok || feeValue <= 0) {
        return std::nullopt;
    }

    if (m_fee_toggle->isChecked()) {
        // Fee rate mode (sats/vb)
        txTemplate.fee_rate = feeValue;
        txTemplate.fee = 0;
    } else {
        // Absolute fee mode (sats)
        txTemplate.fee = static_cast<uint64_t>(feeValue);
        txTemplate.fee_rate = 1.0; // Fallback, not used when fee > 0
    }

    // Outputs
    for (auto *out : m_outputs) {
        auto output = Output();
        output.address = rust::String(out->address().toStdString());

        if (out->isMax()) {
            output.max = true;
            output.amount = 0; // Amount is ignored when max is true
        } else {
            auto amount = out->amount();
            if (!amount.has_value()) {
                return std::nullopt;
            }
            output.amount = amount.value();
            output.max = false;
        }

        output.label = rust::String(out->label().toStdString());
        txTemplate.outputs.push_back(output);
    }

    // Add selected coins as input outpoints only when manual selection is active
    if (!m_auto_coin_selection->isChecked()) {
        for (const auto &coin : m_selected_coins) {
            txTemplate.input_outpoints.push_back(rust::String(std::string(coin.outpoint)));
        }
    }
    // When auto is checked, leave input_outpoints empty -> Rust auto-selects

    return txTemplate;
}

void Send::setSpendable(bool spendable) {
    qDebug() << "Send::setSpendable()" << spendable;
    m_send_btn->setEnabled(spendable);
}

void Send::updateOutputValidations() {
    qDebug() << "Send::updateOutputValidations()";
    for (auto *out : m_outputs) {
        out->updateAddressValidation();
        out->updateAmountValidation();
    }
}

void Send::updateFeeValidation() {
    qDebug() << "Send::updateFeeValidation()";
    QString text = m_fee_value_input->text();
    if (text.isEmpty()) {
        setValidationIndicator(m_fee_indicator, text, false);
    } else {
        bool ok = false;
        double val = text.toDouble(&ok);
        setValidationIndicator(m_fee_indicator, text, ok && val > 0);
    }
}

void Send::process() {
    qDebug() << "Send::process()";
    updateOutputValidations();
    updateFeeValidation();
    updateInputsTotal();

    auto txTemp = txTemplate();
    if (!txTemp.has_value()) {
        setSpendable(false);
        setBroadcastable(false);
        m_fee_estimate_label->setVisible(false);
        // Clear auto selection when outputs are invalid
        if (m_auto_coin_selection->isChecked() && !m_auto_selected_outpoints.isEmpty()) {
            m_auto_selected_outpoints.clear();
            updateCoinCheckboxes();
            updateInputsTotal();
        }
        return;
    }

    auto simu = m_controller->simulateTx(txTemp.value());

    if (simu.error.empty()) {
        if (simu.is_valid) {
            setSpendable(true);
            m_warning_label->setVisible(false);

            // Display estimated fee
            m_fee_estimate_label->setText(QString("Fee: %1").arg(toBitcoin(simu.fee)));
            m_fee_estimate_label->setVisible(true);

            // Fill in estimated max amount for MAX outputs
            for (auto *out : m_outputs) {
                if (out->isMax()) {
                    uint64_t nonMaxTotal = 0;
                    for (auto *other : m_outputs) {
                        if (!other->isMax()) {
                            auto amt = other->amount();
                            if (amt.has_value()) {
                                nonMaxTotal += amt.value();
                            }
                        }
                    }
                    uint64_t maxAmount =
                        simu.output_total > nonMaxTotal ? simu.output_total - nonMaxTotal : 0;
                    out->setMaxAmount(maxAmount);
                    break;
                }
            }

            // Store auto-selected outpoints for display when auto mode is active
            if (m_auto_coin_selection->isChecked()) {
                QStringList newSelection;
                for (const auto &op : simu.selected_outpoints) {
                    QString opStr = QString::fromUtf8(op.data(), op.size());
                    newSelection.append(opStr);
                }
                m_auto_selected_outpoints = newSelection;
                updateCoinCheckboxes();
                updateInputsTotal();
            }
        } else {
            setSpendable(false);
            m_fee_estimate_label->setVisible(false);
            m_warning_label->setText("Transaction is not valid");
            m_warning_label->setVisible(true);
        }
    } else {
        setSpendable(false);
        setBroadcastable(false);
        m_fee_estimate_label->setVisible(false);
        m_warning_label->setText(simu.error.c_str());
        m_warning_label->setVisible(true);
    }
}

void Send::sendTransaction() {
    qDebug() << "Send::sendTransaction()";
    auto txTemp = txTemplate();
    if (!txTemp.has_value()) {
        AppController::execModal(new qontrol::Modal("Error", "Invalid transaction template."));
        return;
    }

    auto &account = m_controller->getAccount();
    if (!account.has_value()) {
        AppController::execModal(new qontrol::Modal("Error", "No account loaded."));
        return;
    }

    // Prepare transaction on main thread (pure computation)
    auto psbt = account.value()->prepare_transaction(txTemp.value());
    if (!psbt->is_ok()) {
        auto error = QString::fromStdString(std::string(psbt->get_psbt_error().c_str()));
        AppController::execModal(new qontrol::Modal("Prepare Failed", error));
        return;
    }

    // Store psbt and tx template for use in validation / onSendConfirmed / logging
    m_psbt_result = std::make_optional(std::move(psbt));
    m_tx_template = txTemp;

    // Run PSBT validation on background thread before showing confirm modal
    m_send_btn->setEnabled(false);
    auto *thread = QThread::create([this]() {
        auto result =
            m_controller->getAccount().value()->validate_before_sign(*m_psbt_result.value());
        emit validationReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void Send::onValidationResult(PsbtValidation result) {
    qDebug() << "Send::onValidationResult() is_ok:" << result.is_ok
             << "is_valid:" << result.is_valid;

    m_send_btn->setEnabled(true);

    if (!result.is_ok) {
        // Connection/protocol error — warn but don't block
        auto error = QString::fromStdString(std::string(result.error.c_str()));
        AppController::execModal(new qontrol::Modal("Validation Error", error));
        // Fall through to show ConfirmSend anyway
    } else if (result.spent_input_count > 0) {
        // Spent inputs — block the transaction
        auto issues = QString::fromStdString(std::string(result.issues.c_str()));
        AppController::execModal(new qontrol::Modal("Error", "Input(s) already spent:\n" + issues));
        m_psbt_result = std::nullopt;
        m_tx_template = std::nullopt;
        return;
    } else if (result.reused_output_count > 0) {
        // Address reuse — warn and ask user via accept/reject modal
        auto issues = QString::fromStdString(std::string(result.issues.c_str()));
        auto *modal = new qontrol::Modal();
        modal->setWindowTitle("Address Reuse Warning");
        auto *label = new Label(issues + "\n\nProceed anyway?");
        label->setWordWrap(true);
        auto *proceedBtn = new Button("Proceed");
        auto *cancelBtn = new Button("Cancel");
        connect(proceedBtn, &QPushButton::clicked, modal, &QDialog::accept);
        connect(cancelBtn, &QPushButton::clicked, modal, &QDialog::reject);
        auto *btnRow = (new qontrol::Row)
                           ->pushSpacer()
                           ->push(cancelBtn)
                           ->pushSpacer()
                           ->push(proceedBtn)
                           ->pushSpacer();
        auto *col = (new qontrol::Column)
                        ->push(label)
                        ->pushSpacer(resolve(Spacing::M))
                        ->push(btnRow)
                        ->pushSpacer();
        modal->setMainWidget(margin(col, resolve(Spacing::S)));
        int dialogResult = modal->exec();
        delete modal;
        if (dialogResult != QDialog::Accepted) {
            m_psbt_result = std::nullopt;
            m_tx_template = std::nullopt;
            return;
        }
    }

    // Continue with normal ConfirmSend flow
    if (!m_psbt_result.has_value() || !m_tx_template.has_value()) {
        return;
    }

    // Build recipient summary for confirmation modal
    QStringList recipients;
    for (auto *out : m_outputs) {
        QString addr = out->address();
        if (out->isMax()) {
            recipients.append(addr + "  (max)");
        } else {
            auto amount = out->amount();
            if (amount.has_value()) {
                recipients.append(addr + "  " + toBitcoin(amount.value()));
            }
        }
    }

    auto txidPreview =
        QString::fromStdString(std::string(m_psbt_result.value()->get_txid_preview().c_str()));
    uint64_t fee = 0;

    // Use the fee from the last simulation if available
    auto simu = m_controller->simulateTx(m_tx_template.value());
    if (simu.is_valid) {
        fee = simu.fee;
    }

    m_confirm_modal = new modal::ConfirmSend(recipients, fee, txidPreview);
    connect(m_confirm_modal, &modal::ConfirmSend::confirmed, this, &Send::onSendConfirmed,
            qontrol::UNIQUE);
    AppController::execModal(m_confirm_modal);
    m_confirm_modal = nullptr;
}

void Send::onSendConfirmed() {
    qDebug() << "Send::onSendConfirmed()";
    auto &account = m_controller->getAccount();
    if (!account.has_value() || !m_psbt_result.has_value()) {
        if (m_confirm_modal != nullptr) {
            m_confirm_modal->setResult(false, "Transaction state lost.");
        }
        return;
    }

    // Sign on background thread (future-proofed for hardware signing devices)
    auto *thread = QThread::create([this]() {
        auto result = m_controller->getAccount().value()->sign_transaction(*m_psbt_result.value());
        emit signReady(result);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Send::onSignResult(TxResult result) {
    qDebug() << "Send::onSignResult() ok:" << result.is_ok;
    // Clear psbt - no longer needed
    m_psbt_result = std::nullopt;

    if (!result.is_ok) {
        auto error = QString::fromStdString(std::string(result.error.c_str()));
        m_tx_template = std::nullopt;
        if (m_confirm_modal != nullptr) {
            m_confirm_modal->setResult(false, "Signing Failed: " + error);
        }
        return;
    }

    m_signed_tx_hex = QString::fromStdString(std::string(result.value.c_str()));

    // Broadcast on background thread (network I/O)
    auto signedHex = std::string(result.value.c_str());
    auto *thread = QThread::create([this, signedHex]() {
        auto broadcastResult =
            m_controller->getAccount().value()->broadcast_transaction(rust::String(signedHex));
        emit broadcastReady(broadcastResult);
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

void Send::onBroadcastResult(TxResult result) {
    qDebug() << "Send::onBroadcastResult() ok:" << result.is_ok;

    if (!result.is_ok && m_tx_template.has_value() && !m_signed_tx_hex.isEmpty()) {
        auto &account = m_controller->getAccount();
        if (account.has_value()) {
            account.value()->log_failed_broadcast(m_tx_template.value(),
                                                  m_signed_tx_hex.toStdString());
        }
    }

    m_signed_tx_hex.clear();
    m_tx_template = std::nullopt;

    if (m_confirm_modal != nullptr) {
        if (result.is_ok) {
            auto txid = QString::fromStdString(std::string(result.value.c_str()));
            m_confirm_modal->setResult(true, txid);
        } else {
            auto error = QString::fromStdString(std::string(result.error.c_str()));
            m_confirm_modal->setResult(false, error);
        }
    }

    // Refresh coin state
    m_controller->pollCoins();

    if (result.is_ok) {
        clearOutputs();
        clearInputs();
    }
}

void Send::onOutputDeleteClicked() {
    qDebug() << "Send::onOutputDeleteClicked()";
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn != nullptr) {
        int id = btn->property("outputId").toInt();
        deleteOutput(id);
    }
}

void Send::onOutputMaxToggled() {
    qDebug() << "Send::onOutputMaxToggled()";
    auto *checkbox = qobject_cast<Checkbox *>(sender());
    if (checkbox != nullptr) {
        int id = checkbox->property("outputId").toInt();
        auto *output = m_outputs.value(id);
        if (output != nullptr) {
            output->setMaxMode(checkbox->isChecked());
        }
        outputSetMax(id);
    }
}

} // namespace screen

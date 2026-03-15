#include "Send.h"
#include "AccountController.h"
#include "AppController.h"
#include "modals/ConfirmSend.h"
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
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <string>

namespace screen {

static void setValidationIndicator(QLabel *indicator, const QString &text, bool valid) {
    if (text.isEmpty()) {
        indicator->setText("");
        indicator->setStyleSheet("");
    } else {
        if (valid) {
            indicator->setText(QString::fromUtf8("✓"));
            indicator->setStyleSheet("QLabel { color: green; }");
        } else {
            indicator->setText(QString::fromUtf8("✗"));
            indicator->setStyleSheet("QLabel { color: red; }");
        }
    }
}

InputW::InputW(const RustCoin &coin) {
    auto *outpointLabel = new QLabel(QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size()));
    outpointLabel->setFixedWidth(OUTPOINT_WIDTH);

    auto *valueLabel = new QLabel(toBitcoin(coin.value));
    valueLabel->setFixedWidth(LABEL_WIDTH);
    valueLabel->setAlignment(Qt::AlignRight);

    auto *labelLabel = new QLabel(QString::fromUtf8(coin.label.data(), coin.label.size()));
    labelLabel->setFixedWidth(VALUE_WIDTH);

    auto *row = (new qontrol::Row)
                    ->push(outpointLabel)
                    ->pushSpacer(H_SPACER)
                    ->push(valueLabel)
                    ->pushSpacer(H_SPACER)
                    ->push(labelLabel)
                    ->pushSpacer();

    m_widget = row;
}

auto InputW::widget() -> QWidget * {
    return m_widget;
}

OutputW::OutputW(Send *screen, int id) {
    m_address = new QLineEdit;
    m_address->setFixedWidth(300);
    m_address->setPlaceholderText("Silent Payment Address");

    m_address_indicator = new QLabel();
    m_address_indicator->setFixedWidth(20);
    m_address_indicator->setAlignment(Qt::AlignCenter);

    m_delete = new QPushButton();
    QIcon closeIcon = m_delete->style()->standardIcon(QStyle::SP_DialogCloseButton);
    m_delete->setIcon(closeIcon);
    m_delete->setFixedWidth(m_address->minimumSizeHint().height());
    m_delete->setFixedHeight(m_address->minimumSizeHint().height());

    m_delete_spacer = new QWidget;
    m_delete_spacer->setFixedWidth(V_SPACER);

    m_amount = new QLineEdit;
    m_amount->setFixedWidth(95);
    m_amount->setPlaceholderText("0.002 BTC");

    m_amount_indicator = new QLabel();
    m_amount_indicator->setFixedWidth(20);
    m_amount_indicator->setAlignment(Qt::AlignCenter);

    m_amount_spacer = new QWidget;
    m_amount_spacer->setFixedWidth(95 + 20);  // amount + indicator width
    m_amount_spacer->setVisible(false);

    m_label = new QLineEdit;
    m_label->setFixedWidth(2 * INPUT_WIDTH);
    m_label->setPlaceholderText("Label");

    m_max = new QCheckBox;
    m_max->setStyleSheet(R"(
      QCheckBox::indicator {
        width: 28px;
        height: 28px;
      }
    )");
    QObject::connect(m_max, &QCheckBox::toggled, screen, &Send::process, qontrol::UNIQUE);

    m_max_label = new QLabel("MAX");
    QFont f = m_max_label->font();
    f.setPointSize(f.pointSize() + 4);
    m_max_label->setFont(f);

    // Connect to process() which handles validation updates
    QObject::connect(m_address, &QLineEdit::textChanged, screen, &Send::process);
    QObject::connect(m_amount, &QLineEdit::textChanged, screen, &Send::process);

    auto *addrRow = (new qontrol::Row)
                        ->push(m_delete)
                        ->push(m_delete_spacer)
                        ->push(m_address)
                        ->push(m_address_indicator)
                        ->pushSpacer(H_SPACER)
                        ->push(m_amount)
                        ->push(m_amount_indicator)
                        ->push(m_amount_spacer)
                        ->pushSpacer(H_SPACER)
                        ->push(m_max)
                        ->pushSpacer(H_SPACER)
                        ->push(m_max_label)
                        ->pushSpacer();

    auto *labelRow = (new qontrol::Row)->push(m_label)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(V_SPACER)
                    ->push(addrRow)
                    ->pushSpacer(V_SPACER)
                    ->push(labelRow)
                    ->pushSpacer(2 * V_SPACER);

    m_delete->setProperty("outputId", id);
    QObject::connect(m_delete, &QPushButton::clicked, screen, &Send::onOutputDeleteClicked,
                     qontrol::UNIQUE);

    m_max->setProperty("outputId", id);
    QObject::connect(m_max, &QCheckBox::checkStateChanged, screen, &Send::onOutputMaxToggled,
                     qontrol::UNIQUE);

    m_widget = col;
}

auto OutputW::widget() -> QWidget * {
    return m_widget;
}

auto OutputW::setDeletable(bool deletable) -> void {
    m_delete->setVisible(deletable);
    m_delete_spacer->setVisible(deletable);
}

auto OutputW::enableMax(bool max) -> void {
    m_max->setChecked(false);
    m_max->setVisible(max);
    m_max_label->setVisible(max);
    setAmountVisible(true);
}

auto OutputW::setAmountVisible(bool visible) -> void {
    m_amount->setVisible(visible);
    m_amount_indicator->setVisible(visible);
    m_amount_spacer->setVisible(!visible);
}

auto OutputW::clearAmount() -> void {
    m_amount->clear();
}

auto OutputW::isMax() -> bool {
    return m_max->isChecked();
}

auto Send::onFeeToggled() -> void {
    qDebug() << "Send::onFeeToggled()";
    if (m_fee_toggle->isChecked()) {
        m_fee_label->setText("sats/vb");
        // sats/vb: 3 decimal places max (milli-sats precision)
        auto *validator = new QDoubleValidator(0.001, 1000000.0, 3, m_fee_value);
        validator->setNotation(QDoubleValidator::StandardNotation);
        m_fee_value->setValidator(validator);
    } else {
        m_fee_label->setText("sats");
        // sats: integers only
        m_fee_value->setValidator(new QIntValidator(1, 100000000, m_fee_value));
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

auto Send::init() -> void {
    qDebug() << "Send::init()";
    m_outputs_column = (new qontrol::Column);
    m_inputs_column = (new qontrol::Column);

    m_add_output_btn = new QPushButton("+ Add an Output");
    m_send_button = new QPushButton("Send");
    m_clear_outputs_btn = new QPushButton("Clear");

    m_fee_estimate_label = new QLabel();
    m_fee_estimate_label->setVisible(false);

    // Fee toggle: OFF = sats (absolute), ON = sats/vb (rate)
    m_fee_toggle = new qontrol::widgets::ToggleSwitch;
    m_fee_toggle->setChecked(true); // Default to sats/vb mode
    m_fee_toggle->setFixedSize(40, 20);

    m_fee_value = new QLineEdit;
    m_fee_value->setFixedWidth(100);
    m_fee_value->setText("1");

    m_fee_indicator = new QLabel;
    m_fee_indicator->setFixedWidth(20);
    m_fee_indicator->setAlignment(Qt::AlignCenter);

    m_fee_label = new QLabel("sats/vb");
    m_fee_label->setFixedWidth(m_fee_label->fontMetrics().horizontalAdvance("sats/vb") + 5);

    m_fee_row = (new qontrol::Row)
                    ->push(m_fee_toggle)
                    ->pushSpacer(V_SPACER)
                    ->push(m_fee_value)
                    ->push(m_fee_indicator)
                    ->pushSpacer(V_SPACER)
                    ->push(m_fee_label)
                    ->pushSpacer();

    m_warning_label = new QLabel();
    m_warning_label->setVisible(false);

    // Calculate label width to align with coin value column
    // checkbox + spacer + outpoint + spacer + label = total label width
    int labelWidth = 20 + H_SPACER + InputW::OUTPOINT_WIDTH + H_SPACER + InputW::LABEL_WIDTH;

    // Inputs total row
    auto *totalLabel = new QLabel("Total selected:");
    totalLabel->setFixedWidth(labelWidth);
    totalLabel->setAlignment(Qt::AlignRight);
    m_inputs_total = new QLineEdit();
    m_inputs_total->setFixedWidth(InputW::VALUE_WIDTH);
    m_inputs_total->setAlignment(Qt::AlignRight);
    m_inputs_total->setEnabled(false);
    m_inputs_total_row = (new qontrol::Row)
                             ->push(totalLabel)
                             ->pushSpacer(H_SPACER)
                             ->push(m_inputs_total)
                             ->pushSpacer();
    m_inputs_total_row->setVisible(false);

    // Inputs minimum row
    auto *minLabel = new QLabel("Amount to select:");
    minLabel->setFixedWidth(labelWidth);
    minLabel->setAlignment(Qt::AlignRight);
    m_inputs_min = new QLineEdit();
    m_inputs_min->setFixedWidth(InputW::VALUE_WIDTH);
    m_inputs_min->setAlignment(Qt::AlignRight);
    m_inputs_min->setEnabled(false);
    m_inputs_min_row =
        (new qontrol::Row)->push(minLabel)->pushSpacer(H_SPACER)->push(m_inputs_min)->pushSpacer();
    m_inputs_min_row->setVisible(false);

    m_auto_coin_selection = new QCheckBox("Auto coin selection");
    m_auto_coin_selection->setChecked(true);

    m_clear_inputs_btn = new QPushButton("Clear");

    m_inputs_title = new QLabel("Select Inputs");
    auto titleFont = m_inputs_title->font();
    titleFont.setPointSize(15);
    m_inputs_title->setFont(titleFont);
}

auto Send::doConnect() -> void {
    qDebug() << "Send::doConnect()";
    connect(m_add_output_btn, &QPushButton::clicked, this, &Send::addOutput, qontrol::UNIQUE);
    connect(m_send_button, &QPushButton::clicked, this, &Send::sendTransaction, qontrol::UNIQUE);
    connect(m_clear_outputs_btn, &QPushButton::clicked, this, &Send::clearOutputs, qontrol::UNIQUE);
    connect(m_clear_inputs_btn, &QPushButton::clicked, this, &Send::clearInputs, qontrol::UNIQUE);
    connect(m_auto_coin_selection, &QCheckBox::toggled, this, &Send::onAutoSelectionToggled,
            qontrol::UNIQUE);
    connect(m_controller, &AccountController::updateCoins, this, &Send::onCoinsUpdated,
            qontrol::UNIQUE);
    connect(m_fee_toggle, &QCheckBox::toggled, this, &Send::onFeeToggled, qontrol::UNIQUE);
    connect(m_fee_value, &QLineEdit::textChanged, this, &Send::process, qontrol::UNIQUE);
    connect(this, &Send::signReady, this, &Send::onSignResult, qontrol::UNIQUE);
    connect(this, &Send::broadcastReady, this, &Send::onBroadcastResult, qontrol::UNIQUE);
}

auto Send::view() -> void {
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

    auto *col = (new qontrol::Column)->push(m_inputs_frame)->pushSpacer(20)->push(m_outputs_frame);

    auto *oldWidget = m_main_widget;
    m_main_widget = margin(col, 10);
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

    auto *addOutputRow = (new qontrol::Row)->pushSpacer()->push(m_add_output_btn)->pushSpacer();

    auto *lastRow = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(m_clear_outputs_btn)
                        ->pushSpacer()
                        ->push(m_send_button)
                        ->pushSpacer();

    m_fee_row->setParent(nullptr);
    m_fee_estimate_label->setParent(nullptr);
    auto *feeRow = (new qontrol::Row)
                       ->merge(m_fee_row)
                       ->pushSpacer(H_SPACER)
                       ->push(m_fee_estimate_label)
                       ->pushSpacer();

    auto *warningRow = (new qontrol::Row)->push(m_warning_label)->pushSpacer();

    auto *title = new QLabel("Recipients");
    auto font = title->font();
    font.setPointSize(15);
    title->setFont(font);

    auto *titleRow = (new qontrol::Row)->pushSpacer(15)->push(title)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->push(titleRow)
                    ->pushSpacer(20)
                    ->push(m_outputs_column)
                    ->push(addOutputRow)
                    ->pushSpacer(20)
                    ->push(feeRow)
                    ->pushSpacer(20)
                    ->push(warningRow)
                    ->pushSpacer(20)
                    ->push(lastRow)
                    ->pushSpacer();

    return col;
}

auto Send::inputsView() -> QWidget * {
    qDebug() << "Send::inputsView()";
    auto availableCoins = m_controller->getCoins();

    // Unparent persistent rows
    m_inputs_total_row->setParent(nullptr);
    m_inputs_min_row->setParent(nullptr);

    // Clear old checkbox references
    m_coin_checkboxes.clear();

    bool autoMode = m_auto_coin_selection->isChecked();

    auto *coinsColumn = new qontrol::Column;
    for (const auto &coin : availableCoins) {
        auto *checkbox = new QCheckBox();
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
        connect(checkbox, &QCheckBox::checkStateChanged, this, &Send::onCoinToggled,
                qontrol::UNIQUE);

        auto *outpointField = new QLineEdit(shortenOutpoint(outpoint));
        outpointField->setFixedWidth(InputW::OUTPOINT_WIDTH);
        outpointField->setEnabled(false);

        QString label = QString::fromUtf8(coin.label.data(), coin.label.size());
        auto *labelField = new QLineEdit(label);
        labelField->setFixedWidth(InputW::LABEL_WIDTH);
        labelField->setEnabled(false);

        auto *valueField = new QLineEdit(toBitcoin(coin.value));
        valueField->setFixedWidth(InputW::VALUE_WIDTH);
        valueField->setAlignment(Qt::AlignRight);
        valueField->setEnabled(false);

        auto *row = (new qontrol::Row)
                        ->push(checkbox)
                        ->pushSpacer(H_SPACER)
                        ->push(outpointField)
                        ->pushSpacer(H_SPACER)
                        ->push(labelField)
                        ->pushSpacer(H_SPACER)
                        ->push(valueField)
                        ->pushSpacer();
        coinsColumn->pushSpacer(V_SPACER)->push(row);
    }

    m_coins_scroll = new QScrollArea;
    m_coins_scroll->setWidget(coinsColumn);
    m_coins_scroll->setFixedHeight(200);
    m_coins_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_auto_coin_selection->setParent(nullptr);
    m_inputs_title->setParent(nullptr);
    m_clear_inputs_btn->setParent(nullptr);

    // Only show Clear button when in manual mode
    m_clear_inputs_btn->setVisible(!m_auto_coin_selection->isChecked());

    auto *titleRow = (new qontrol::Row)
                         ->pushSpacer(15)
                         ->push(m_inputs_title)
                         ->pushSpacer()
                         ->push(m_clear_inputs_btn)
                         ->pushSpacer(10)
                         ->push(m_auto_coin_selection)
                         ->pushSpacer(15);

    // Update total and minimum displays
    updateInputsTotal();

    auto *col = (new qontrol::Column)
                    ->push(titleRow)
                    ->pushSpacer(10)
                    ->push(m_coins_scroll)
                    ->pushSpacer(V_SPACER)
                    ->push(m_inputs_min_row)
                    ->push(m_inputs_total_row)
                    ->pushSpacer();

    return col;
}

auto Send::addOutput() -> void {
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

auto Send::deleteOutput(int id) -> void {
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

auto Send::outputSetMax(int id) -> void {
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

auto Send::setBroadcastable(bool broadcastable) -> void {
    qDebug() << "Send::setBroadcastable()" << broadcastable;
    m_broadcastable = broadcastable;
    m_send_button->setEnabled(broadcastable);
}

auto Send::clearOutputs() -> void {
    qDebug() << "Send::clearOutputs()";
    for (auto *outp : m_outputs) {
        delete outp;
    }
    m_outputs.clear();
    addOutput();
    process();
    view();
}

auto Send::clearInputs() -> void {
    qDebug() << "Send::clearInputs()";
    m_selected_coins.clear();
    for (auto *cb : m_coin_checkboxes.values()) {
        cb->setChecked(false);
    }
    process();
    view();
}

auto Send::onCoinToggled() -> void {
    qDebug() << "Send::onCoinToggled()";
    auto *checkbox = qobject_cast<QCheckBox *>(sender());
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

auto Send::onAutoSelectionToggled() -> void {
    qDebug() << "Send::onAutoSelectionToggled()";
    if (!m_auto_coin_selection->isChecked()) {
        // Switching to manual: populate m_selected_coins from last auto selection
        updateSelectedCoinsFromSimulation();
    }
    process();
    view();
}

auto Send::onCoinsUpdated(CoinState state) -> void {
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

auto Send::updateSelectedCoinsFromSimulation() -> void {
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

auto Send::updateInputsTotal() -> void {
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
        m_inputs_total->setText(QString::number(fValue, 'f', 8) + " BTC");
        m_inputs_total_row->setVisible(true);
    } else {
        m_inputs_total_row->setVisible(false);
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
        m_inputs_min->setText(QString::number(fValue, 'f', 8) + " BTC");
        m_inputs_min_row->setVisible(true);
    } else {
        m_inputs_min_row->setVisible(false);
    }
}

auto Send::updateCoinCheckboxes() -> void {
    qDebug() << "Send::updateCoinCheckboxes()";
    bool autoMode = m_auto_coin_selection->isChecked();

    for (auto it = m_coin_checkboxes.begin(); it != m_coin_checkboxes.end(); ++it) {
        QString outpoint = it.key();
        QCheckBox *checkbox = it.value();

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

auto Send::updateInputsTitle() -> void {
    qDebug() << "Send::updateInputsTitle()";
    int count = 0;
    if (m_auto_coin_selection->isChecked()) {
        count = m_auto_selected_outpoints.size();
    } else {
        count = m_selected_coins.size();
    }

    if (count == 1) {
        m_inputs_title->setText(QString("Inputs (1 coin selected)"));
    } else if (count > 1) {
        m_inputs_title->setText(QString("Inputs (%1 coins selected)").arg(count));
    } else {
        m_inputs_title->setText("Inputs");
    }
}

auto OutputW::address() -> QString {
    return m_address->text();
}

auto OutputW::amount() -> std::optional<uint64_t> {
    auto amountStr = m_amount->text();
    bool ok = false;
    auto amountBtc = amountStr.toDouble(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return static_cast<uint64_t>(amountBtc * SATS);
}

auto OutputW::label() -> QString {
    return m_label->text();
}

auto OutputW::updateAddressValidation() -> void {
    QString addr = m_address->text();
    if (addr.isEmpty()) {
        setValidationIndicator(m_address_indicator, "", false);
    } else {
        auto result = ::validate_address(rust::String(addr.toStdString()));
        setValidationIndicator(m_address_indicator, addr, result.empty());
    }
}

auto OutputW::updateAmountValidation() -> void {
    QString text = m_amount->text();
    if (text.isEmpty()) {
        setValidationIndicator(m_amount_indicator, "", false);
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
    auto feeValue = m_fee_value->text().toDouble(&ok);
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

auto Send::setSpendable(bool spendable) -> void {
    qDebug() << "Send::setSpendable()" << spendable;
    m_send_button->setEnabled(spendable);
}

auto Send::updateOutputValidations() -> void {
    qDebug() << "Send::updateOutputValidations()";
    for (auto *out : m_outputs) {
        out->updateAddressValidation();
        out->updateAmountValidation();
    }
}

auto Send::updateFeeValidation() -> void {
    qDebug() << "Send::updateFeeValidation()";
    QString text = m_fee_value->text();
    if (text.isEmpty()) {
        setValidationIndicator(m_fee_indicator, "", false);
    } else {
        bool ok = false;
        double val = text.toDouble(&ok);
        setValidationIndicator(m_fee_indicator, text, ok && val > 0);
    }
}

auto Send::process() -> void {
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

auto Send::sendTransaction() -> void {
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

    auto txidPreview = QString::fromStdString(std::string(psbt->get_txid_preview().c_str()));
    auto fee = m_fee_estimate_label->isVisible()
                   ? static_cast<uint64_t>(m_fee_value->text().toDouble() > 0 ? m_fee_value->text().toDouble() : 0)
                   : 0;

    // Use the fee from the last simulation if available
    auto simu = m_controller->simulateTx(txTemp.value());
    if (simu.is_valid) {
        fee = simu.fee;
    }

    // Store psbt for use in onSendConfirmed
    m_psbt_result = std::make_optional(std::move(psbt));

    auto *modal = new modal::ConfirmSend(recipients, fee, txidPreview);
    connect(modal, &modal::ConfirmSend::confirmed, this, &Send::onSendConfirmed, qontrol::UNIQUE);
    AppController::execModal(modal);
}

auto Send::onSendConfirmed() -> void {
    qDebug() << "Send::onSendConfirmed()";
    auto &account = m_controller->getAccount();
    if (!account.has_value() || !m_psbt_result.has_value()) {
        AppController::execModal(new qontrol::Modal("Error", "Transaction state lost."));
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

auto Send::onSignResult(TxResult result) -> void {
    qDebug() << "Send::onSignResult() ok:" << result.is_ok;
    // Clear psbt - no longer needed
    m_psbt_result = std::nullopt;

    if (!result.is_ok) {
        auto error = QString::fromStdString(std::string(result.error.c_str()));
        AppController::execModal(new qontrol::Modal("Signing Failed", error));
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

auto Send::onBroadcastResult(TxResult result) -> void {
    qDebug() << "Send::onBroadcastResult() ok:" << result.is_ok;
    m_signed_tx_hex.clear();

    if (!result.is_ok) {
        auto error = QString::fromStdString(std::string(result.error.c_str()));
        AppController::execModal(new qontrol::Modal("Broadcast Failed", error));
        return;
    }

    auto txid = QString::fromStdString(std::string(result.value.c_str()));
    AppController::execModal(new qontrol::Modal("Transaction Sent", "Txid: " + txid));

    // Refresh coin state
    m_controller->pollCoins();
}

auto Send::onOutputDeleteClicked() -> void {
    qDebug() << "Send::onOutputDeleteClicked()";
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn != nullptr) {
        int id = btn->property("outputId").toInt();
        deleteOutput(id);
    }
}

auto Send::onOutputMaxToggled() -> void {
    qDebug() << "Send::onOutputMaxToggled()";
    auto *checkbox = qobject_cast<QCheckBox *>(sender());
    if (checkbox != nullptr) {
        int id = checkbox->property("outputId").toInt();
        auto *output = m_outputs.value(id);
        if (output != nullptr) {
            bool isMax = checkbox->isChecked();
            output->setAmountVisible(!isMax);
            if (isMax) {
                output->clearAmount();
            }
        }
        outputSetMax(id);
    }
}

} // namespace screen

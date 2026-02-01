#include "Send.h"
#include "AccountController.h"
#include "AppController.h"
#include "common.h"
#include "screens/modals/SelectCoins.h"
#include <Qontrol>
#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcontainerfwd.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qpushbutton.h>
#include <qradiobutton.h>

namespace screen {

OutputW::OutputW(Send *screen, int id) {
    m_address = new QLineEdit;
    m_address->setFixedWidth(300);
    m_address->setPlaceholderText("Silent Payment Address");
    QObject::connect(m_address, &QLineEdit::editingFinished, screen,
                     &Send::process);

    m_delete = new QPushButton();
    QIcon closeIcon = m_delete->style()->standardIcon(
        QStyle::SP_DialogCloseButton);
    m_delete->setIcon(closeIcon);
    m_delete->setFixedWidth(m_address->minimumSizeHint().height());
    m_delete->setFixedHeight(m_address->minimumSizeHint().height());

    m_delete_spacer = new QWidget;
    m_delete_spacer->setFixedWidth(V_SPACER);

    m_amount = new QLineEdit;
    m_amount->setFixedWidth(95);
    m_amount->setPlaceholderText("0.002 BTC");
    QObject::connect(m_amount, &QLineEdit::editingFinished, screen,
                     &Send::process);

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
    QObject::connect(m_max, &QCheckBox::toggled, screen, &Send::process,
                     qontrol::UNIQUE);

    m_max_label = new QLabel("MAX");
    QFont f = m_max_label->font();
    f.setPointSize(f.pointSize() + 4);
    m_max_label->setFont(f);

    auto *addrRow = (new qontrol::Row)
                        ->push(m_delete)
                        ->push(m_delete_spacer)
                        ->push(m_address)
                        ->pushSpacer(H_SPACER)
                        ->push(m_amount)
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

    QObject::connect(m_delete, &QPushButton::clicked, screen,
                     [screen, id]() { screen->deleteOutput(id); });
    QObject::connect(m_max, &QCheckBox::checkStateChanged, screen,
                     [screen, id]() { screen->outputSetMax(id); });

    m_widget = col;
}

auto OutputW::widget() -> QWidget * {
    return m_widget;
}

void OutputW::setDeletable(bool deletable) {
    m_delete->setVisible(deletable);
    m_delete_spacer->setVisible(deletable);
}

void OutputW::enableMax(bool max) {
    m_max->setChecked(false);
    m_max->setVisible(max);
    m_max_label->setVisible(max);
}

auto OutputW::isMax() -> bool {
    return m_max->isChecked();
}

RadioElement::RadioElement(Send *parent, const QString &label) {
    m_button = new QRadioButton(parent);
    QObject::connect(m_button, &QAbstractButton::toggled, parent,
                     &Send::process);
    m_value = new QLineEdit;
    m_value->setFixedWidth(100);
    QObject::connect(m_value, &QLineEdit::editingFinished, parent,
                     &Send::process);
    m_label = new QLabel(label);

    auto *row = (new qontrol::Row)
                    ->push(m_button)
                    ->pushSpacer(V_SPACER)
                    ->push(m_value)
                    ->pushSpacer(V_SPACER)
                    ->push(m_label)
                    ->pushSpacer(4 * V_SPACER);
    m_widget = row;
}

auto RadioElement::button() -> QAbstractButton * {
    return m_button;
}

void RadioElement::update() {
    m_value->setEnabled(m_button->isChecked());
}

auto RadioElement::widget() -> qontrol::Row * {
    return m_widget;
}

void Send::updateRadio() {
    m_fee_sats_vb->update();
    process();
}

Send::Send(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->addOutput();
    this->view();
    this->updateRadio();
    this->setBroadcastable(false);
}

void Send::init() {
    m_outputs_column = (new qontrol::Column);

    m_add_output_btn = new QPushButton("+ Add an Output");
    connect(m_add_output_btn, &QPushButton::clicked, this, &Send::addOutput);

    m_select_coins_btn = new QPushButton("Select Coins");
    connect(m_select_coins_btn, &QPushButton::clicked, this, &Send::addCoins);

    m_simulate_btn = new QPushButton("Simulate");
    connect(m_simulate_btn, &QPushButton::clicked, this, &Send::simulateTransaction);

    m_send_button = new QPushButton("Send");
    connect(m_send_button, &QPushButton::clicked, this, &Send::sendTransaction);

    m_clear_outputs_btn = new QPushButton("Clear");
    connect(m_clear_outputs_btn, &QPushButton::clicked, this,
            &Send::clearOutputs, qontrol::UNIQUE);

    m_fee_sats_vb = new RadioElement(this, "sats/vb");
    m_fee_group = new QButtonGroup;
    m_fee_group->addButton(m_fee_sats_vb->button());

    m_warning_label = new QLabel();
    m_warning_label->setVisible(false);

    m_fee_sats_vb->button()->setChecked(true);
}

void Send::doConnect() {
}

void Send::view() {
    auto *oldOutputs = m_outputs_frame;
    m_outputs_frame = frame(outputsView());
    delete oldOutputs;

    auto *row = (new qontrol::Row)
                    ->push(m_outputs_frame);

    auto *oldWidget = m_main_widget;
    m_main_widget = margin(row, 10);
    delete oldWidget;
    delete this->layout();
    this->setLayout(m_main_widget->layout());
}

auto Send::outputsView() -> QWidget * {
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

    auto *addOutputRow = (new qontrol::Row)
                             ->pushSpacer()
                             ->push(m_add_output_btn)
                             ->pushSpacer()
                             ->push(m_select_coins_btn)
                             ->pushSpacer();

    auto *lastRow = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(m_clear_outputs_btn)
                        ->pushSpacer()
                        ->push(m_simulate_btn)
                        ->push(m_send_button)
                        ->pushSpacer();

    auto *feeRow = (new qontrol::Row)
                       ->merge(m_fee_sats_vb->widget())
                       ->pushSpacer();

    auto *warningRow = (new qontrol::Row)->push(m_warning_label)->pushSpacer();

    auto *title = new QLabel("Send Transaction");
    auto font = title->font();
    font.setPointSize(15);
    title->setFont(font);

    auto *titleRow = (new qontrol::Row)
                         ->pushSpacer(15)
                         ->push(title)
                         ->pushSpacer();

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

void Send::addOutput() {
    auto *output = new OutputW(this, m_output_id);
    if (m_outputs.size() == 0) {
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
    m_broadcastable = broadcastable;
    m_send_button->setEnabled(broadcastable);
}

void Send::clearOutputs() {
    for (auto *outp : m_outputs) {
        delete outp;
    }
    m_outputs.clear();
    addOutput();
    process();
    view();
}

void Send::addCoins() {
    qDebug() << "Send::addCoins() - Opening SelectCoins modal";

    // Get available coins from AccountController
    auto availableCoins = m_controller->getCoins();

    // Convert rust::Vec to QList
    QList<RustCoin> coinsList;
    for (const auto &coin : availableCoins) {
        coinsList.append(coin);
    }

    if (coinsList.isEmpty()) {
        auto *modal = new qontrol::Modal("No Coins", "No spendable coins available for selection");
        AppController::execModal(modal);
        return;
    }

    // Create and show SelectCoins modal
    auto *selectCoinsModal = new modal::SelectCoins(coinsList);
    connect(selectCoinsModal, &modal::SelectCoins::coinsSelected, this, &Send::onCoinsSelected, qontrol::UNIQUE);
    AppController::execModal(selectCoinsModal);
}

void Send::onCoinsSelected(const QList<RustCoin> &coins) {
    qDebug() << "Send::onCoinsSelected() - " << coins.size() << " coins selected";

    // TODO: Store selected coins and use them for transaction building
    // For now, just log the selected coins
    uint64_t totalValue = 0;
    for (const auto &coin : coins) {
        totalValue += coin.value;
        qDebug() << "  Selected coin:" << QString(coin.outpoint.c_str())
                 << "Value:" << coin.value;
    }

    auto *modal = new qontrol::Modal("Coins Selected",
        QString("Selected %1 coin(s) with total value: %2 BTC")
            .arg(coins.size())
            .arg(toBitcoin(totalValue)));
    AppController::execModal(modal);

    // Re-process transaction with new coin selection
    process();
}

auto RadioElement::text() -> QString {
    return m_value->text();
}

auto OutputW::address() -> QString {
    return m_address->text();
}

auto OutputW::amount() -> std::optional<uint64_t> {
    auto amountStr = m_amount->text();
    bool ok = false;
    auto amountBtc = amountStr.toDouble(&ok);
    if (!ok) {
        qDebug() << "Output::amount() m_amount value is not a valid double";
        return std::nullopt;
    }
    return static_cast<uint64_t>(amountBtc * SATS);
}

auto OutputW::label() -> QString {
    return m_label->text();
}

auto Send::txTemplate() -> std::optional<TransactionTemplate> {
    auto txTemplate = TransactionTemplate();
    bool ok = false;

    // Fee rate
    if (m_fee_sats_vb->checked()) {
        auto fee = m_fee_sats_vb->text().toDouble(&ok);
        if (!ok || fee <= 0) {
            qDebug() << "Send::txTemplate() m_fee_sats_vb is not a valid double";
            return std::nullopt;
        }
        txTemplate.fee_rate = fee;
    } else {
        return std::nullopt;
    }

    // Outputs
    for (auto *out : m_outputs) {
        auto output = Output();
        output.address = rust::String(out->address().toStdString());

        if (out->isMax()) {
            output.max = true;
            output.amount = 0;  // Amount is ignored when max is true
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

    return txTemplate;
}

void RadioElement::setEnabled(bool enabled) {
    m_button->setEnabled(enabled);
    m_value->setEnabled(enabled);
}

auto RadioElement::checked() -> bool {
    return m_button->isChecked();
}

void Send::setSpendable(bool spendable) {
    m_simulate_btn->setEnabled(spendable);
}

void Send::process() {
    qDebug() << "Send::process()";
    auto txTemp = txTemplate();
    if (!txTemp.has_value()) {
        setSpendable(false);
        setBroadcastable(false);
        return;
    }

    auto simu = m_controller->simulateTx(txTemp.value());
    if (simu.error.empty()) {
        if (simu.is_valid) {
            setSpendable(true);
            m_warning_label->setVisible(false);
        } else {
            setSpendable(false);
            m_warning_label->setText("Transaction is not valid");
            m_warning_label->setVisible(true);
        }
    } else {
        setSpendable(false);
        setBroadcastable(false);
        m_warning_label->setText(simu.error.c_str());
        m_warning_label->setVisible(true);
    }
}

void Send::simulateTransaction() {
    qDebug() << "Send::simulateTransaction()";
    auto txTemp = txTemplate();
    if (!txTemp.has_value()) {
        auto *modal = new qontrol::Modal("Error", "Invalid transaction template");
        AppController::execModal(modal);
        return;
    }

    auto simu = m_controller->simulateTx(txTemp.value());
    if (simu.error.empty() && simu.is_valid) {
        QString msg = QString("Fee: %1 BTC\nWeight: %2\nInputs: %3\nInput Total: %4 BTC\nOutput Total: %5 BTC")
            .arg(toBitcoin(simu.fee))
            .arg(simu.weight)
            .arg(simu.input_count)
            .arg(toBitcoin(simu.input_total))
            .arg(toBitcoin(simu.output_total));
        auto *modal = new qontrol::Modal("Transaction Simulation", msg);
        AppController::execModal(modal);
        setBroadcastable(true);
    } else {
        QString error = QString("Simulation failed: %1").arg(QString(simu.error.c_str()));
        auto *modal = new qontrol::Modal("Error", error);
        AppController::execModal(modal);
        setBroadcastable(false);
    }
}

void Send::sendTransaction() {
    qDebug() << "Send::sendTransaction()";
    // This would prepare, sign, and broadcast the transaction
    auto *modal = new qontrol::Modal("Not Implemented", "Transaction signing and broadcasting not yet implemented");
    AppController::execModal(modal);
}

} // namespace screen

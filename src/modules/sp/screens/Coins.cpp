#include "Coins.h"
#include "AccountController.h"
#include "i18n/Tr.h"
#include "theme/Button.h"
#include "theme/Display.h"
#include "theme/Icon.h"
#include "theme/Input.h"
#include "theme/Theme.h"
#include "theme/Label.h"
#include "screens/utils.h"
#include <QKeyEvent>
#include <Qontrol>
#include <common.h>
#include <cstdint>
#include <optional>
#include <utility>

namespace screen {

using theme::Button;
using theme::ButtonRole;
using theme::Display;
using theme::DisplayRole;
using theme::Input;
using theme::Label;
using theme::LabelRole;

Coins::Coins(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->doConnect();
    this->view();
}

void Coins::init() {
    // Match Display padding (6px) + border (1px) = 7px left indent
    int hPad = 7;

    m_h_type = new Label(TR("coins-type"), LabelRole::InfoLabel);
    m_h_type->setFixedWidth(TYPE_W);
    m_h_type->setContentsMargins(hPad, 0, 0, 0);
    m_h_height = new Label(TR("coins-height"), LabelRole::InfoLabel);
    m_h_height->setFixedWidth(HEIGHT_W);
    m_h_height->setContentsMargins(hPad, 0, 0, 0);
    m_h_outpoint = new Label(TR("coins-outpoint"), LabelRole::InfoLabel);
    m_h_outpoint->setFixedWidth(OUTPOINT_W);
    m_h_outpoint->setContentsMargins(hPad, 0, 0, 0);
    m_h_label = new Label(TR("coins-label"), LabelRole::InfoLabel);
    m_h_label->setFixedWidth(LABEL_W);
    m_h_label->setContentsMargins(hPad, 0, 0, 0);
    m_h_edit = new QWidget;
    m_h_edit->setFixedWidth(Theme::get()->buttonPalette().inlineIcon.width);
    m_h_value = new Label(TR("coins-value"), LabelRole::InfoLabel);
    m_h_value->setFixedWidth(VALUE_W);
    m_h_value->setContentsMargins(hPad, 0, 0, 0);
}

void Coins::onRecvPayload(const CoinState &state) {
    m_state = state;
    this->view();
    emit coinsUpdated();
}

void Coins::doConnect() {
    auto *ctrl = m_controller;
    connect(ctrl, &AccountController::updateCoins, this, &Coins::onRecvPayload, qontrol::UNIQUE);
}

auto balanceRow(const QString &label_str, uint64_t balance, uint64_t coins_count) -> QWidget * {
    auto *label = new Label(label_str, LabelRole::InfoLabel);

    auto priceStr = toBitcoin(balance);
    auto *price = new Label(priceStr, LabelRole::Mono);

    auto coinStr = coinsCount(coins_count);
    auto *coins = new Label(coinStr);

    auto *row = (new qontrol::Row)->push(label)->push(price)->push(coins)->pushSpacer();

    return row;
}

void Coins::view() {
    auto *oldCR = m_confirmed_row;
    m_confirmed_row = balanceRow(TR("coins-confirmed"), m_state.confirmed_balance, m_state.confirmed_count);
    delete oldCR;

    auto *oldUR = m_unconfirmed_row;
    m_unconfirmed_row =
        balanceRow(TR("coins-unconfirmed"), m_state.unconfirmed_balance, m_state.unconfirmed_count);
    delete oldUR;

    // Get coins from controller's account
    if (m_controller != nullptr) {
        m_coins = m_controller->getCoins();
    } else {
        m_coins = rust::Vec<RustCoin>();
    }

    // Clear tracking
    m_label_inputs.clear();
    m_edit_buttons.clear();

    // Unparent persistent header labels
    m_h_type->setParent(nullptr);
    m_h_height->setParent(nullptr);
    m_h_outpoint->setParent(nullptr);
    m_h_label->setParent(nullptr);
    m_h_edit->setParent(nullptr);
    m_h_value->setParent(nullptr);

    auto *headerRow = (new qontrol::Row)
                          ->push(m_h_type)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_height)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_outpoint)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_label)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_edit)
                          ->pushSpacer(resolve(Spacing::XS))
                          ->push(m_h_value)
                          ->pushSpacer();

    // Column containing header + coin rows
    auto *coinsColumn = new qontrol::Column;
    coinsColumn->push(headerRow);

    for (const auto &coin : m_coins) {
        QString typeStr = QString::fromUtf8(coin.account_type.data(), coin.account_type.size());
        auto *typeField = new Display(typeStr);
        typeField->setFixedWidth(TYPE_W);

        QString heightStr;
        if (!coin.spent && coin.height > 0) {
            heightStr = QString::number(coin.height);
        } else if (coin.spent) {
            heightStr = TR("coins-spent");
        } else {
            heightStr = TR("coins-unconfirmed-no-colon");
        }
        auto *heightField = new Display(heightStr);
        heightField->setFixedWidth(HEIGHT_W);

        QString outpoint = QString::fromUtf8(coin.outpoint.data(), coin.outpoint.size());
        auto *outpointField = new Display(shortenOutpoint(outpoint), DisplayRole::Outpoint);
        outpointField->setFixedWidth(OUTPOINT_W);

        QString label = QString::fromUtf8(coin.label.data(), coin.label.size());
        auto *labelField = new Display(label);
        labelField->setFixedWidth(LABEL_W);
        labelField->setProperty("outpoint", outpoint);

        auto *editBtn = new Button(ButtonRole::InlineIcon);
        editBtn->setIcon(icon::pencil());
        editBtn->setProperty("outpoint", outpoint);
        connect(editBtn, &Button::clicked, this, &Coins::onEditLabelClicked, qontrol::UNIQUE);
        m_edit_buttons.append(editBtn);

        auto *valueField = new Display(toBitcoin(coin.value), DisplayRole::Amount);
        valueField->setFixedWidth(VALUE_W);

        auto *row = (new qontrol::Row)
                        ->push(typeField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(heightField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(outpointField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(labelField)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(editBtn)
                        ->pushSpacer(resolve(Spacing::XS))
                        ->push(valueField)
                        ->pushSpacer();

        coinsColumn->pushSpacer(resolve(Spacing::XS))->push(row);
    }
    coinsColumn->pushSpacer();

    auto *scroll = new QScrollArea;
    scroll->setWidget(coinsColumn);
    scroll->setWidgetResizable(true);
    scroll->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    delete m_scroll;
    m_scroll = scroll;

    auto *mainLayout = (new qontrol::Column(this))
                           ->push(m_confirmed_row)
                           ->pushSpacer(resolve(Spacing::XS))
                           ->push(m_unconfirmed_row)
                           ->pushSpacer(resolve(Spacing::M))
                           ->push(scroll);

    auto *boxed = margin(mainLayout);
    delete m_main_widget;
    m_main_widget = boxed;

    auto *old = this->layout();
    delete old;
    this->setLayout(boxed->layout());
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto Coins::getCoins() -> std::optional<QList<RustCoin>> {
    auto coins = QList<RustCoin>();
    for (const auto &coin : m_coins) {
        coins.append(coin);
    }
    if (coins.isEmpty()) {
        return std::nullopt;
    }
    return std::make_optional(coins);
}

void Coins::onEditLabelClicked() {
    auto *btn = qobject_cast<Button *>(sender());
    if (btn == nullptr) {
        return;
    }

    QString outpoint = btn->property("outpoint").toString();
    if (outpoint.isEmpty()) {
        return;
    }

    // Already editing this coin
    if (m_label_inputs.contains(outpoint)) {
        return;
    }

    // Find the label Display in the same row
    auto *row = btn->parentWidget();
    if (row == nullptr) {
        return;
    }

    Display *labelDisplay = nullptr;
    for (auto *child : row->findChildren<Display *>()) {
        if (child->property("outpoint").toString() == outpoint) {
            labelDisplay = child;
            break;
        }
    }
    if (labelDisplay == nullptr) {
        return;
    }

    // Input spans label + button (spacers remain in layout)
    int btnW = Theme::get()->buttonPalette().inlineIcon.width;
    int inputW = LABEL_W + btnW;

    auto *input = new Input(labelDisplay->text());
    input->setFixedWidth(inputW);
    input->setProperty("outpoint", outpoint);
    input->installEventFilter(this);
    connect(input, &Input::editingFinished, this, &Coins::onLabelEditFinished, qontrol::UNIQUE);

    // Replace Display with Input in the layout
    auto *layout = row->layout();
    if (layout == nullptr) {
        delete input;
        return;
    }

    int idx = layout->indexOf(labelDisplay);
    if (idx < 0) {
        delete input;
        return;
    }

    layout->replaceWidget(labelDisplay, input);
    labelDisplay->hide();
    btn->hide();

    m_label_inputs.insert(outpoint, input);
    setEditButtonsEnabled(false);
    input->setFocus();
    input->selectAll();
}

void Coins::onLabelEditFinished() {
    auto *input = qobject_cast<Input *>(sender());
    if (input == nullptr) {
        return;
    }

    QString outpoint = input->property("outpoint").toString();
    if (outpoint.isEmpty()) {
        return;
    }

    QString newLabel = input->text();

    qDebug() << "Coins::onLabelEditFinished() - Updating label for" << outpoint << "to" << newLabel;

    // Update the coin label through the controller
    if (m_controller != nullptr) {
        m_controller->updateCoinLabel(outpoint, newLabel);
    }

    // Remove from tracking (view() will rebuild everything)
    m_label_inputs.remove(outpoint);
}

void Coins::setEditButtonsEnabled(bool enabled) {
    for (auto *btn : m_edit_buttons) {
        btn->setEnabled(enabled);
    }
}

auto Coins::eventFilter(QObject *obj, QEvent *event) -> bool {
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            auto *input = qobject_cast<Input *>(obj);
            if (input != nullptr) {
                QString outpoint = input->property("outpoint").toString();
                m_label_inputs.remove(outpoint);
                setEditButtonsEnabled(true);
                // Rebuild view to restore Display
                this->view();
                return true;
            }
        }
    }
    return Screen::eventFilter(obj, event);
}

} // namespace screen

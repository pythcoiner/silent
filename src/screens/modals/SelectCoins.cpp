#include "SelectCoins.h"
#include "screens/common.h"
#include <Qontrol>
#include <optional>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcontainerfwd.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include <algorithm>

namespace modal {

void CoinWidget::updateLabel() {
    auto stdStr = m_label->text().toStdString();
    m_coin.label = rust::String(stdStr);
}

auto CoinWidget::isChecked() -> bool {
    return m_checkbox->isChecked();
}

auto CoinWidget::coin() -> RustCoin {
    return m_coin;
}

void SelectCoins::onOk() {
    auto selectedCoins = QList<RustCoin>();
    for (auto id : m_coins.keys()) {
        auto *cw = m_coins.value(id);
        if (cw->isChecked()) {
            selectedCoins.append(cw->coin());
        }
    }

    emit coinsSelected(selectedCoins);
    accept();
}

void SelectCoins::onAbort() {
    reject();
}

void SelectCoins::init(const QList<RustCoin> &coins) {
    setWindowTitle("Select coin(s)");
    setFixedSize(700, 400);

    m_label_filter = new QLineEdit();
    m_label_filter->setPlaceholderText("label filter...");
    connect(m_label_filter, &QLineEdit::textEdited, this,
            &SelectCoins::applyFilter, qontrol::UNIQUE);

    m_value_up = new QPushButton();
    auto up = m_value_up->style()->standardIcon(QStyle::SP_ArrowUp);
    m_value_up->setIcon(up);
    m_value_up->setFixedWidth(60);
    m_value_up->setCheckable(true);

    m_value_down = new QPushButton();
    auto down = m_value_down->style()->standardIcon(QStyle::SP_ArrowDown);
    m_value_down->setIcon(down);
    m_value_down->setFixedWidth(60);
    m_value_down->setCheckable(true);

    m_total = new QLineEdit();
    m_total->setFixedWidth(m_amount_width);
    m_total->setEnabled(false);

    m_total_label = new QLabel("Total selected: ");
    m_total_label->setFixedWidth(m_label_width);
    m_total_label->setAlignment(Qt::AlignRight);

    connect(m_value_up, &QPushButton::toggled, [this]() {
        if (this->m_value_up->isChecked()) {
            this->m_value_down->setChecked(false);
        }
        this->view();
    });
    connect(m_value_down, &QPushButton::toggled, [this]() {
        if (this->m_value_down->isChecked()) {
            this->m_value_up->setChecked(false);
        }
        this->view();
    });

    m_abort = new QPushButton("Cancel");
    m_abort->setFixedWidth(150);
    connect(m_abort, &QPushButton::clicked, this, &SelectCoins::onAbort,
            qontrol::UNIQUE);

    m_ok = new QPushButton("Ok");
    m_ok->setFixedWidth(150);
    m_ok->setEnabled(false);
    connect(m_ok, &QPushButton::clicked, this, &SelectCoins::onOk,
            qontrol::UNIQUE);

    int id = 0;
    for (const auto &coin : coins) {
        auto *cw = new CoinWidget(coin, this);
        m_coins.insert(id, cw);
        id++;
    }

    if (m_coins.size() == 1) {
        auto k = m_coins.keys().first();
        m_coins.find(k).value()->setChecked(true);
    }
}

void SelectCoins::view() {
    // Reset parents to avoid deletion
    for (auto *cw : getCoins()) {
        cw->amount()->setParent(nullptr);
        cw->checkbox()->setParent(nullptr);
        cw->outpoint()->setParent(nullptr);
        cw->label()->setParent(nullptr);
    }
    m_total->setParent(nullptr);
    m_total_label->setParent(nullptr);

    auto filtered = filter(getCoins());
    auto sorted = sort(filtered);

    auto *oldWidget = m_widget;

    int arrowBtnWidth = m_value_up->sizeHint().height();
    m_value_up->setFixedWidth(arrowBtnWidth);
    m_value_down->setFixedWidth(arrowBtnWidth);
    m_label_filter->setFixedWidth(m_label_width);
    int spacer = 25 + m_outpoint_width + H_SPACER;

    int valueSpacer = m_amount_width - (2 * arrowBtnWidth);
    auto *firstRow = (new qontrol::Row)
                         ->pushSpacer(spacer)
                         ->push(m_label_filter)
                         ->pushSpacer(H_SPACER)
                         ->push(m_value_up)
                         ->pushSpacer(valueSpacer)
                         ->push(m_value_down)
                         ->pushSpacer();

    auto *coinsCol = new qontrol::Column;
    for (auto *cw : sorted) {
        cw->outpoint()->setFixedWidth(200);
        cw->label()->setFixedWidth(200);
        cw->amount()->setFixedWidth(m_amount_width);
        auto *row = (new qontrol::Row)
                        ->push(cw->checkbox())
                        ->pushSpacer(H_SPACER)
                        ->push(cw->outpoint())
                        ->pushSpacer(H_SPACER)
                        ->push(cw->label())
                        ->pushSpacer(H_SPACER)
                        ->push(cw->amount())
                        ->pushSpacer();
        coinsCol->pushSpacer(V_SPACER)->push(row);
    }
    auto *scroll = new QScrollArea;
    scroll->setWidget(coinsCol);
    scroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    uint64_t selectedAmount = 0;

    for (auto *cw : getCoins()) {
        if (cw->isChecked()) {
            selectedAmount += cw->coin().value;
        }
    }

    std::optional<QWidget *> totalRow = std::nullopt;
    if (selectedAmount > 0) {
        double fValue = selectedAmount;
        fValue /= SATS;
        auto btcValue = QString::number(fValue) + " BTC";
        m_total->setText(btcValue);
        auto *total = (new qontrol::Row)
                          ->pushSpacer(spacer)
                          ->push(m_total_label)
                          ->pushSpacer(H_SPACER)
                          ->push(m_total)
                          ->pushSpacer();
        totalRow = std::make_optional(total);
        m_total->setVisible(true);
        m_total_label->setVisible(true);
    } else {
        m_total->setVisible(false);
        m_total_label->setVisible(false);
    }

    auto *lastRow = (new qontrol::Row)
                        ->pushSpacer()
                        ->push(m_abort)
                        ->pushSpacer()
                        ->push(m_ok)
                        ->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->push(firstRow)
                    ->pushSpacer(10)
                    ->push(scroll)
                    ->pushSpacer(5)
                    ->push(totalRow)
                    ->pushSpacer(20)
                    ->push(lastRow)
                    ->pushSpacer();

    m_widget = margin(col);

    delete oldWidget;
    setMainWidget(m_widget);
}

auto SelectCoins::getCoins() -> QList<CoinWidget *> {
    auto coins = QList<CoinWidget *>();
    for (auto *coin : m_coins) {
        coins.append(coin);
    }
    return coins;
}

auto CoinWidget::checkbox() -> QCheckBox * {
    return m_checkbox;
}

auto CoinWidget::label() -> QLineEdit * {
    return m_label;
}

auto CoinWidget::amount() -> QLineEdit * {
    return m_value;
}

auto CoinWidget::outpoint() -> QLineEdit * {
    return m_outpoint;
}

CoinWidget::CoinWidget(const RustCoin &coin, SelectCoins *modal) {
    m_coin = coin;
    m_checkbox = new QCheckBox();
    connect(m_checkbox, &QCheckBox::stateChanged, modal,
            [modal]() { modal->checked(); }, qontrol::UNIQUE);

    m_outpoint = new QLineEdit();
    auto op = m_coin.outpoint;
    m_outpoint->setText(QString(op.c_str()));
    m_outpoint->setEnabled(false);

    m_label = new QLineEdit();
    auto label = m_coin.label;
    m_label->setText(QString(label.c_str()));
    m_label->setEnabled(false); // Disable editing to prevent non-persisted changes

    m_value = new QLineEdit();
    double fValue = m_coin.value;
    fValue /= SATS;
    auto btcValue = QString::number(fValue) + " BTC";
    m_value->setText(btcValue);
    m_value->setEnabled(false);
}

auto SelectCoins::sort(const QList<CoinWidget *> &coins)
    -> QList<CoinWidget *> {
    QList<CoinWidget *> sortedCoins = coins;

    bool sortAscendingAmount = m_value_up->isChecked();
    bool sortDescendingAmount = m_value_down->isChecked();

    if (sortAscendingAmount) {
        std::ranges::sort(sortedCoins, [](CoinWidget *a, CoinWidget *b) {
            return a->coin().value < b->coin().value;
        });
    } else if (sortDescendingAmount) {
        std::ranges::sort(sortedCoins, [](CoinWidget *a, CoinWidget *b) {
            return a->coin().value > b->coin().value;
        });
    } else {
        std::ranges::sort(sortedCoins, [](CoinWidget *a, CoinWidget *b) {
            return a->coin().outpoint < b->coin().outpoint;
        });
    }
    return sortedCoins;
}

auto SelectCoins::filter(const QList<CoinWidget *> &coins)
    -> QList<CoinWidget *> {
    if (!m_label_filter->text().isEmpty()) {
        auto filtered = QList<CoinWidget *>();
        for (auto *coin : coins) {
            auto label = QString(coin->coin().label.c_str());
            if (label.contains(m_label_filter->text())) {
                filtered.append(coin);
            }
        }
        return filtered;
    }
    return coins;
}

void SelectCoins::applyFilter() {
    int cursorPos = m_label_filter->cursorPosition();

    view();

    m_label_filter->setFocus();
    m_label_filter->setCursorPosition(cursorPos);
}

SelectCoins::SelectCoins(const QList<RustCoin> &coins) {
    init(coins);
    view();
}

auto CoinWidget::setCheckable(bool checkable) {
    m_checkbox->setCheckable(checkable);
}

void SelectCoins::checked() {
    int checked = 0;
    for (auto *cw : m_coins) {
        if (cw->isChecked()) {
            checked++;
        }
    }

    if (checked > 0) {
        m_ok->setEnabled(true);
    } else {
        m_ok->setEnabled(false);
    }
    view();
}

void CoinWidget::setChecked(bool checked) {
    m_checkbox->setChecked(checked);
}

} // namespace modal

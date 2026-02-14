#pragma once

#include <Qontrol>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qhash.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qobject.h>
#include <qpushbutton.h>
#include <qstyle.h>
#include <qtmetamacros.h>
#include <silent.h>

namespace modal {

class SelectCoins;

class CoinWidget : public QObject {
    Q_OBJECT
public:
    CoinWidget(const RustCoin &coin, SelectCoins *modal);
    auto isChecked() -> bool;
    auto setCheckable(bool checkable) -> void;
    auto setChecked(bool checked) -> void;
    auto coin() -> RustCoin;
    auto checkbox() -> QCheckBox *;
    auto outpoint() -> QLineEdit *;
    auto label() -> QLineEdit *;
    auto amount() -> QLineEdit *;

public slots:
    auto updateLabel() -> void;

private:
    RustCoin m_coin;
    QCheckBox *m_checkbox = nullptr;
    QLineEdit *m_outpoint = nullptr;
    QLineEdit *m_label = nullptr;
    QLineEdit *m_value = nullptr;
};

class SelectCoins : public qontrol::Modal {
    Q_OBJECT
public:
    SelectCoins(const QList<RustCoin> &coins);

    auto init(const QList<RustCoin> &coins) -> void;
    auto doConnect() -> void;
    auto view() -> void;
    auto filter(const QList<CoinWidget *> &coins) -> QList<CoinWidget *>;
    auto sort(const QList<CoinWidget *> &coins) -> QList<CoinWidget *>;
    auto getCoins() -> QList<CoinWidget *>;
    auto applyFilter() -> void;

signals:
    void coinsSelected(QList<RustCoin> coins);

public slots:
    auto checked() -> void;
    auto onAbort() -> void;
    auto onOk() -> void;
    auto onSortAscendingToggled() -> void;
    auto onSortDescendingToggled() -> void;

private:
    int m_amount_width = 150;
    int m_label_width = 200;
    int m_outpoint_width = 200;

    QWidget *m_widget = nullptr;
    QLineEdit *m_label_filter = nullptr;
    QLineEdit *m_total = nullptr;
    QLabel *m_total_label = nullptr;
    QPushButton *m_value_up = nullptr;
    QPushButton *m_value_down = nullptr;
    QButtonGroup *m_value_group = nullptr;
    QPushButton *m_abort = nullptr;
    QPushButton *m_ok = nullptr;
    QHash<int, CoinWidget *> m_coins;
};

} // namespace modal

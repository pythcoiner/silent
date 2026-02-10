#pragma once

#include <Qontrol>
#include <cstdint>
#include <optional>
#include <qabstractbutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qtmetamacros.h>
#include <qwidget.h>
#include <silent.h>

class AccountController;

namespace screen {
class Send;

class InputW {
public:
    InputW(const RustCoin &coin);
    auto widget() -> QWidget *;

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

private:
    QLineEdit *m_address = nullptr;
    QLineEdit *m_label = nullptr;
    QLineEdit *m_amount = nullptr;
    QPushButton *m_delete = nullptr;
    QWidget *m_delete_spacer = nullptr;
    QCheckBox *m_max = nullptr;
    QLabel *m_max_label = nullptr;
    QWidget *m_widget = nullptr;
};

class RadioElement {
public:
    RadioElement(Send *parent, const QString &label);
    auto widget() -> qontrol::Row *;
    void update();
    auto button() -> QAbstractButton *;
    auto text() -> QString;
    void setEnabled(bool enabled);
    auto checked() -> bool;

private:
    QRadioButton *m_button = nullptr;
    QLineEdit *m_value = nullptr;
    QLabel *m_label = nullptr;
    qontrol::Row *m_widget;
};

class Send : public qontrol::Screen {
    Q_OBJECT
public:
    Send(AccountController *ctrl);

signals:

public slots:
    void outputSetMax(int id);
    void deleteOutput(int id);
    void addOutput();
    void clearOutputs();
    void updateRadio();
    void setBroadcastable(bool broadcastable);
    void addCoins();
    void onCoinsSelected(const QList<RustCoin> &coins);
    void setSpendable(bool spendable);
    void process();
    void simulateTransaction();
    void sendTransaction();

protected:
    void init() override;
    void doConnect() override;
    void view() override;
    auto outputsView() -> QWidget *;
    auto inputsView() -> QWidget *;
    auto txTemplate() -> std::optional<TransactionTemplate>;

private:
    AccountController *m_controller = nullptr;
    int m_output_id = 0;
    QHash<int, OutputW *> m_outputs;
    qontrol::Column *m_outputs_column = nullptr;
    qontrol::Column *m_inputs_column = nullptr;

    QWidget *m_main_widget = nullptr;
    QWidget *m_outputs_frame = nullptr;
    QWidget *m_inputs_frame = nullptr;

    RadioElement *m_fee_sats_vb = nullptr;
    QButtonGroup *m_fee_group = nullptr;

    QLabel *m_warning_label = nullptr;

    QPushButton *m_add_output_btn = nullptr;
    QPushButton *m_select_coins_btn = nullptr;

    QPushButton *m_clear_outputs_btn = nullptr;
    QPushButton *m_simulate_btn = nullptr;
    QPushButton *m_send_button = nullptr;

    bool m_broadcastable = false;
    QList<RustCoin> m_selected_coins;

    auto output() -> QWidget *;
};

} // namespace screen

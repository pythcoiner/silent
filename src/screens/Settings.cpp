#include "Settings.h"
#include "AccountController.h"
#include "common.h"
#include <Qontrol>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qpushbutton.h>

namespace screen {

Settings::Settings(AccountController *ctrl) {
    m_controller = ctrl;
    this->init();
    this->view();
    this->doConnect();
}

void Settings::init() {
}

void Settings::doConnect() {
    connect(m_btn_save, &QPushButton::clicked, this, &Settings::actionSave, qontrol::UNIQUE);
}

void Settings::actionSave() {
    qDebug() << "Settings::actionSave()";
    // TODO: Save settings to config
    auto *modal = new qontrol::Modal("Settings", "Settings saved (not yet implemented)");
    // Note: Would need AppController to exec modal, but we don't have access here
    // This is a placeholder for now
    emit configSaved();
}

void Settings::view() {
    if (m_view_init)
        return;

    auto *url_label = new QLabel("BlindBit URL:");
    url_label->setFixedWidth(LABEL_WIDTH);

    m_blindbit_url = new QLineEdit;
    m_blindbit_url->setFixedWidth(3 * INPUT_WIDTH);
    m_blindbit_url->setPlaceholderText("https://blindbit.example.com");

    auto *url_row = (new qontrol::Row)
                        ->push(url_label)
                        ->push(m_blindbit_url)
                        ->pushSpacer();

    auto *network_label = new QLabel("Network:");
    network_label->setFixedWidth(LABEL_WIDTH);

    m_network_selector = new QComboBox;
    m_network_selector->addItem("Regtest", static_cast<int>(Network::Regtest));
    m_network_selector->addItem("Signet", static_cast<int>(Network::Signet));
    m_network_selector->addItem("Testnet", static_cast<int>(Network::Testnet));
    m_network_selector->addItem("Bitcoin", static_cast<int>(Network::Bitcoin));
    m_network_selector->setFixedWidth(INPUT_WIDTH);

    auto *network_row = (new qontrol::Row)
                            ->push(network_label)
                            ->push(m_network_selector)
                            ->pushSpacer();

    m_btn_save = new QPushButton("Save");

    auto *saveRow = (new qontrol::Row)->pushSpacer()->push(m_btn_save)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer(50)
                    ->push(url_row)
                    ->pushSpacer(V_SPACER)
                    ->push(network_row)
                    ->pushSpacer(30)
                    ->push(saveRow)
                    ->pushSpacer();

    auto *row = (new qontrol::Row)->pushSpacer()->push(col)->pushSpacer()->pushSpacer();

    m_main_widget = margin(row);
    this->setLayout(m_main_widget->layout());
    m_view_init = true;
}

} // namespace screen

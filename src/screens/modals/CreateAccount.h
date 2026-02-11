#pragma once

#include <Qontrol>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QTextEdit>
#include <QPushButton>
#include <silent.h>

class CreateAccount : public qontrol::Modal {
    Q_OBJECT

public:
    explicit CreateAccount(QWidget *parent = nullptr);

signals:
    void createAccount(const QString &name, const QString &mnemonic,
                      Network network, const QString &blindbit_url);

private slots:
    void onGenerate();
    void onCreate();
    void onModeChanged();
    void onNetworkChanged();
    void onTestBackend();

private:
    void initUI();
    void invalidateBackendTest();
    void updateCreateButton();
    void onBackendInfoReady(BackendInfo info);
    auto generateMnemonic() -> QString;

    QLineEdit *m_name_input = nullptr;
    QRadioButton *m_generate_radio = nullptr;
    QRadioButton *m_restore_radio = nullptr;
    QTextEdit *m_mnemonic_input = nullptr;
    QPushButton *m_generate_btn = nullptr;
    QComboBox *m_network_combo = nullptr;
    QLineEdit *m_blindbit_input = nullptr;
    QPushButton *m_test_btn = nullptr;
    QPushButton *m_create_btn = nullptr;
    QPushButton *m_cancel_btn = nullptr;
    bool m_backend_verified = false;
};

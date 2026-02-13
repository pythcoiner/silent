#pragma once

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <Qontrol>
#include <silent.h>

class CreateAccount : public qontrol::Modal {
    Q_OBJECT

public:
    explicit CreateAccount(QWidget *parent = nullptr);

signals:
    void createAccount(const QString &name, const QString &mnemonic, Network network,
                       const QString &blindbit_url);

private slots:
    auto onGenerate() -> void;
    auto onCreate() -> void;
    auto onNetworkChanged() -> void;
    auto onTestBackend() -> void;
    auto onUpdateCreateButton() -> void;

private:
    auto init() -> void;
    auto doConnect() -> void;
    auto view() -> void;
    auto invalidateBackendTest() -> void;
    auto onBackendInfoReady(BackendInfo info) -> void;
    auto generateMnemonic() -> QString;

    QLineEdit *m_name_input = nullptr;
    QTextEdit *m_mnemonic_input = nullptr;
    QPushButton *m_generate_btn = nullptr;
    QComboBox *m_network_combo = nullptr;
    QLineEdit *m_blindbit_input = nullptr;
    QPushButton *m_test_btn = nullptr;
    QPushButton *m_create_btn = nullptr;
    QPushButton *m_cancel_btn = nullptr;
    bool m_backend_verified = false;
};

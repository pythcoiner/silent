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
                       const QString &blindbit_url, const QString &p2p_node);

private slots:
    auto onGenerate() -> void;
    auto onCreate() -> void;
    auto onNetworkChanged() -> void;
    auto onTestBackend() -> void;
    auto onTestP2p() -> void;
    auto onUpdateCreateButton() -> void;

private:
    auto init() -> void;
    auto doConnect() -> void;
    auto view() -> void;
    auto applyRegtestDefaults() -> void;
    auto invalidateBackendTest() -> void;
    auto onBackendInfoReady(BackendInfo info) -> void;
    auto invalidateP2pTest() -> void;
    auto onP2pTestReady(ConnectionResult result) -> void;
    auto generateMnemonic() -> QString;

    QLineEdit *m_name_input = nullptr;
    QTextEdit *m_mnemonic_input = nullptr;
    QPushButton *m_generate_btn = nullptr;
    QComboBox *m_network_combo = nullptr;
    QLineEdit *m_blindbit_input = nullptr;
    QLineEdit *m_p2p_input = nullptr;
    QPushButton *m_test_btn = nullptr;
    QPushButton *m_test_p2p_btn = nullptr;
    QPushButton *m_create_btn = nullptr;
    QPushButton *m_cancel_btn = nullptr;
    bool m_backend_verified = false;
    bool m_p2p_verified = false;
};

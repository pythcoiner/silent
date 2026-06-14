#pragma once

namespace catalog {
class Button;
}

namespace catalog {
class Input;
}

namespace catalog {
class TextEdit;
}

namespace catalog {
class ComboBox;
}

#include <Qontrol>
#include <silent.h>

namespace modal {

class CreateAccount : public qontrol::Modal {
    Q_OBJECT

public:
    explicit CreateAccount(QWidget *parent = nullptr);

signals:
    void createAccount(const QString &name, const QString &mnemonic, Network network,
                       const QString &blindbit_url, const QString &p2p_node,
                       const QString &electrum_url);
    void backendInfoReady(BackendInfo info);
    void p2pTestReady(ConnectionResult result);
    void electrumTestReady(ConnectionResult result);

public slots:
    void onGenerate();
    void onCreate();
    void onNetworkChanged();
    void onTestBackend();
    void onTestP2p();
    void onTestElectrum();
    void onUpdateCreateButton();
    void onBackendInfoReady(BackendInfo info);
    void onP2pTestReady(ConnectionResult result);
    void onElectrumTestReady(ConnectionResult result);

protected:
    void init();
    void doConnect();
    void view();
    void applyRegtestDefaults();
    void invalidateBackendTest();
    void invalidateP2pTest();
    void invalidateElectrumTest();
    auto generateMnemonic() -> QString;

private:
    catalog::Input *m_name_input = nullptr;
    catalog::TextEdit *m_mnemonic_input = nullptr;
    catalog::Button *m_generate_btn = nullptr;
    catalog::ComboBox *m_network_combo = nullptr;
    catalog::Input *m_blindbit_input = nullptr;
    catalog::Input *m_p2p_input = nullptr;
    catalog::Button *m_test_btn = nullptr;
    catalog::Button *m_test_p2p_btn = nullptr;
    catalog::Input *m_electrum_input = nullptr;
    catalog::Button *m_test_electrum_btn = nullptr;
    catalog::Button *m_create_btn = nullptr;
    catalog::Button *m_cancel_btn = nullptr;
    bool m_backend_verified = false;
    bool m_p2p_verified = false;
    bool m_electrum_verified = false;
};

} // namespace modal

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

namespace catalog {
class ValidationMark;
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
                       const QString &blindbit_url, const QString &electrum_url);
    void backendInfoReady(BackendInfo info);
    void electrumTestReady(ConnectionResult result);

public slots:
    void onGenerate();
    void onCreate();
    void onNetworkChanged();
    void onTestBackend();
    void onTestElectrum();
    void onUpdateCreateButton();
    void onBackendInfoReady(BackendInfo info);
    void onElectrumTestReady(ConnectionResult result);

protected:
    void init();
    void doConnect();
    void view();
    void applyRegtestDefaults();
    void invalidateBackendTest();
    void invalidateElectrumTest();
    auto generateMnemonic() -> QString;

private:
    catalog::Input *m_name_input = nullptr;
    catalog::TextEdit *m_mnemonic_input = nullptr;
    catalog::Button *m_generate_btn = nullptr;
    catalog::ComboBox *m_network_combo = nullptr;
    catalog::Input *m_blindbit_input = nullptr;
    catalog::Button *m_test_btn = nullptr;
    catalog::ValidationMark *m_backend_status = nullptr;
    catalog::Input *m_electrum_input = nullptr;
    catalog::Button *m_test_electrum_btn = nullptr;
    catalog::ValidationMark *m_electrum_status = nullptr;
    catalog::Button *m_create_btn = nullptr;
    catalog::Button *m_cancel_btn = nullptr;
    bool m_backend_verified = false;
    bool m_electrum_verified = false;
};

} // namespace modal

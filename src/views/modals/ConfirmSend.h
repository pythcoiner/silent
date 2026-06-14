#pragma once

#include <Qontrol>
#include <cstdint>
#include <qevent.h>
#include <qstring.h>

namespace catalog {
class Button;
}

namespace catalog {
class Label;
}

namespace catalog {
class ModalStatus;
}

#include <qstringlist.h>

namespace modal {

class ConfirmSend : public qontrol::Modal {
    Q_OBJECT

public:
    ConfirmSend(const QStringList &recipients, uint64_t fee, const QString &txid_preview);

signals:
    void signRequested();
    void broadcastRequested();

public slots:
    void onConfirmClicked();
    void onBroadcastClicked();
    void onSetSigning();
    void onSetSigned();
    void onSetBroadcasting();
    void onSetResult(bool ok, const QString &message);

protected:
    void init();
    void doConnect();
    void view();
    void changeEvent(QEvent *event) override;
    void retranslateUi();

private:
    QStringList m_recipients;
    uint64_t m_fee = 0;
    QString m_txid_preview;
    QWidget *m_summary_widget = nullptr;
    catalog::Label *m_recipients_label = nullptr;
    catalog::Label *m_fee_label = nullptr;
    catalog::Label *m_txid_label = nullptr;
    catalog::Button *m_cancel_btn = nullptr;
    catalog::Button *m_confirm_btn = nullptr;
    catalog::Button *m_broadcast_btn = nullptr;
    catalog::ModalStatus *m_status = nullptr;
    catalog::Button *m_ok_btn = nullptr;
};

} // namespace modal

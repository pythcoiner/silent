#pragma once

#include <Qontrol>
#include <cstdint>
#include <qevent.h>
#include <qstring.h>

namespace theme {
class Button;
}

namespace theme {
class Label;
}

#include <qstringlist.h>

namespace modal {

class ConfirmSend : public qontrol::Modal {
    Q_OBJECT

public:
    ConfirmSend(const QStringList &recipients, uint64_t fee, const QString &txid_preview);

signals:
    void confirmed();

public slots:
    void onConfirmClicked();
    void setBroadcasting();
    void setResult(bool ok, const QString &message);

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
    theme::Label *m_details_label = nullptr;
    theme::Button *m_cancel_btn = nullptr;
    theme::Button *m_confirm_btn = nullptr;
    theme::Label *m_status_label = nullptr;
    theme::Button *m_ok_btn = nullptr;
};

} // namespace modal

#pragma once

#include <Qontrol>
#include <cstdint>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qstring.h>
#include <qstringlist.h>

namespace modal {

class ConfirmSend : public qontrol::Modal {
    Q_OBJECT

public:
    ConfirmSend(const QStringList &recipients, uint64_t fee, const QString &txid_preview);

signals:
    void confirmed();

public slots:
    auto onConfirmClicked() -> void;

protected:
    auto init() -> void;
    auto doConnect() -> void;
    auto view() -> void;

private:
    QStringList m_recipients;
    uint64_t m_fee = 0;
    QString m_txid_preview;
    QLabel *m_details_label = nullptr;
    QPushButton *m_cancel_btn = nullptr;
    QPushButton *m_confirm_btn = nullptr;
};

} // namespace modal

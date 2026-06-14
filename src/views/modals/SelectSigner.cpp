#include "SelectSigner.h"

#include "../utils.h"
#include "i18n/Tr.h"
#include "catalog/Button.h"
#include "theme/Icon.h"
#include "catalog/display/Label.h"
#include "catalog/SelectRow.h"

#include <QVariant>
#include <common.h>

namespace modal {

using catalog::Button;
using catalog::ButtonRole;
using catalog::Label;
using catalog::LabelRole;

namespace {

struct SignerOption {
    QString name;
    QString detail;
    QString fingerprint;
    QString kind;
};

#ifdef SILENT_MOCK_UI
const QList<SignerOption> C_SIGNERS = {
    {"Ledger Nano S Plus", "USB signer", "#a4f9c821", "usb"},
    {"Coldcard Mk4", "USB signer", "#7e02da41", "usb"},
    {"BitBox02", "USB signer", "#b1740c3e", "usb"},
    {"SD Card Airgap Device", "Airgap signer", "", "sd"},
};
#else
// FIXME: populate from the real signer registry once the backend exposes
// connected signers. Empty until then so no fake devices are shown.
const QList<SignerOption> C_SIGNERS = {};
#endif

} // namespace

SelectSigner::SelectSigner(const QString &title, bool include_sd, QWidget *parent)
    : m_title(title)
    , m_include_sd(include_sd) {
    Q_UNUSED(parent);
    setWindowTitle(title);
    resize(440, 320);
    init();
    doConnect();
    view();
}

auto SelectSigner::selectedName() const -> QString {
    return m_selected_name;
}

auto SelectSigner::selectedKind() const -> QString {
    return m_selected_kind;
}

void SelectSigner::init() {
    m_cancel_btn = new Button(TR("common-cancel"));
}

void SelectSigner::doConnect() {
    connect(m_cancel_btn, &QPushButton::clicked, this, &QDialog::reject, qontrol::UNIQUE);
}

void SelectSigner::onSignerClicked() {
    auto *button = qobject_cast<QPushButton *>(sender());
    if (button == nullptr) {
        return;
    }
    m_selected_name = button->property("signerName").toString();
    m_selected_kind = button->property("signerKind").toString();
    accept();
}

void SelectSigner::view() {
    auto *col = (new qontrol::Column)->push(new Label(m_title, LabelRole::Heading));
    col->pushSpacer(resolve(Spacing::M));
    for (const auto &signer : C_SIGNERS) {
        if (!m_include_sd && signer.kind == "sd") {
            continue;
        }
        auto *row = new catalog::SelectRow;
        // sd_card glyph is not vendored; folder is the closest storage stand-in.
        row->setIcon(signer.kind == "sd" ? icon::folder(IconColor::Accent)
                                         : icon::usb(IconColor::Accent));
        row->setTitle(signer.name);
        row->setMetadata(signer.fingerprint.isEmpty() ? signer.detail : signer.fingerprint + "  " + signer.detail);
        row->setProperty("signerName", signer.name);
        row->setProperty("signerKind", signer.kind);
        connect(row, &catalog::SelectRow::clicked, this, &SelectSigner::onSignerClicked, qontrol::UNIQUE);
        col->push(row)->pushSpacer(resolve(Spacing::S));
    }
    auto *cancelRow = (new qontrol::Row)->pushSpacer()->push(m_cancel_btn)->pushSpacer();
    col->push(cancelRow)->pushSpacer();
    setMainWidget(margin(col));
}

} // namespace modal

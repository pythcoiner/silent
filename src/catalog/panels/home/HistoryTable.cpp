#include "catalog/panels/home/HistoryTable.h"

#include "catalog/format.h"
#include "i18n/Tr.h"
#include "theme/Palette.h"

namespace catalog {

namespace {

const int DIR_W = resolve(Size::XS);
const int TXID_W = resolve(Size::S);
const int HEIGHT_W = resolve(Size::XS);
const int AMOUNT_W = resolve(Size::S);

constexpr int TXID_SHORTEN_MIN = 16; // below this length the txid is shown in full
constexpr int TXID_HEAD = 6;         // leading chars kept when shortening
constexpr int TXID_TAIL = 6;         // trailing chars kept when shortening

auto shortenTxid(const QString &txid) -> QString {
    if (txid.length() < TXID_SHORTEN_MIN) {
        return txid;
    }
    return txid.left(TXID_HEAD) + "..." + txid.right(TXID_TAIL);
}

} // namespace

HistoryTable::HistoryTable(QWidget *parent) : Table(parent) {
    addColumn({.title = TR("history-direction"), .width = DIR_W});
    addColumn({.title = TR("history-txid"), .width = TXID_W, .role = DisplayRole::Outpoint});
    addColumn({.title = TR("history-height"), .width = HEIGHT_W});
    addColumn({.title = TR("history-amount"),
               .width = AMOUNT_W,
               .alignment = Qt::AlignRight,
               .headerAlignment = Qt::AlignRight,
               .role = DisplayRole::Amount,
               .grow = true});
}

void HistoryTable::addEntry(const QString &direction, const QString &txid, uint32_t height,
                            uint64_t amount) {
    QString displayDir = (direction == "incoming") ? TR("history-incoming") : TR("history-outgoing");
    QString heightStr = (height > 0) ? QString::number(height) : TR("history-unconfirmed-no-colon");
    addRow(txid, {displayDir, shortenTxid(txid), heightStr, toBitcoin(amount)});
}

} // namespace catalog

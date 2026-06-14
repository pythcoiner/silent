#include "catalog/format.h"

#include <QLocale>

auto toBitcoin(uint64_t sats, bool with_unit) -> QString {
    double bitcoinValue = static_cast<double>(sats) / SATS;
    auto btcStr = QLocale().toString(bitcoinValue, 'f', 8);
    if (with_unit) {
        return btcStr + " BTC";
    }
    return btcStr;
}

#pragma once

#include <QList>
#include <QString>
#include <QtGlobal>

using ReqId = quint64;

namespace plugin {

struct Coin {
    QString outpoint;
    quint64 value{};
    quint32 height{};
    QString label;
    bool spent{};
    QString accountType;
};

struct Balance {
    quint64 confirmed{};
    quint64 unconfirmed{};
};

} // namespace plugin

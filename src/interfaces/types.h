#pragma once

#include <QList>
#include <QMetaType>
#include <QString>
#include <QtGlobal>
#include <optional>

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

Q_DECLARE_METATYPE(plugin::Coin)
Q_DECLARE_METATYPE(plugin::Balance)
Q_DECLARE_METATYPE(QList<plugin::Coin>)
Q_DECLARE_METATYPE(std::optional<ReqId>)

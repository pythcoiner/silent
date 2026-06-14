#include "SpPlugin.h"

QMap<QString, QString> SpPlugin::meta() const {
    return {{QStringLiteral("id"), QStringLiteral("sp")},
            {QStringLiteral("name"), QStringLiteral("Silent Payments")},
            {QStringLiteral("version"), QStringLiteral("1.0.0")}};
}

QList<IModule *> SpPlugin::modules() const {
    return {&m_module};
}

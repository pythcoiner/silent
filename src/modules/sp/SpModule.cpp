#include "SpModule.h"

#include "AppController.h"
#include "SpInstance.h"
#include "modals/CreateAccount.h"
#include "i18n/Tr.h"
#include "interfaces/host.h"
#include <QTimer>
#include <QStringList>
#include <optional>
#include <silent.h>
#include <string>

namespace {
constexpr auto K_SP_PLUGIN_ID = "sp";
const QString K_CREATE_INSTANCE_ID = QStringLiteral("sp.create");
} // namespace

SpModule::SpModule() : IModule(nullptr) {}

QMap<QString, QString> SpModule::meta() const {
    return {{QStringLiteral("id"), QStringLiteral("sp.module")},
            {QStringLiteral("name"), QStringLiteral("Silent Payments Accounts")},
            {QStringLiteral("version"), QStringLiteral("1.0.0")}};
}

ReqId SpModule::list() {
    ReqId reqId = nextReqId();
    QList<QPair<QString, QString>> instances;
    auto configs = ::list_configs();
    for (auto name : configs) {
        QString accountId = QString::fromStdString(std::string(name.c_str()));
        auto config = ::config_from_file(name);
        if (QString::fromStdString(std::string(config->get_plugin_id().c_str()))
            != QString::fromLatin1(K_SP_PLUGIN_ID)) {
            continue;
        }

        // FIXME: detect whether this account is already open (a tab in this app
        // or another process holding the persistence lock) and mark it not openable.
        instances.append({accountId, accountId});
    }

    instances.append({K_CREATE_INSTANCE_ID, TR("sp-create-account")});
    QTimer::singleShot(0, this, [this, reqId, instances]() { emit IModule::instances(reqId, instances); });
    return reqId;
}

ReqId SpModule::autoStart() {
    ReqId reqId = nextReqId();
    QTimer::singleShot(
        0, this, [this, reqId]() { emit IModule::autoStart(reqId, QStringList{}); });
    return reqId;
}

void SpModule::startInstance(const QString &id) {
    if (id == K_CREATE_INSTANCE_ID) {
        AppController::execModal(new modal::CreateAccount);
        return;
    }

    auto configs = ::list_configs();
    bool found = false;
    for (auto name : configs) {
        if (QString::fromStdString(std::string(name.c_str())) != id) {
            continue;
        }
        auto config = ::config_from_file(name);
        if (QString::fromStdString(std::string(config->get_plugin_id().c_str()))
            == QString::fromLatin1(K_SP_PLUGIN_ID)) {
            found = true;
            break;
        }
    }

    if (!found) {
        emit error(std::nullopt, TR("sp-instance-not-found").arg(id));
        return;
    }

    auto instance = std::make_unique<SpInstance>(id);
    Host::get()->registerInstance(id, instance.get());
    m_instances.push_back(std::move(instance));
}

ReqId SpModule::nextReqId() {
    ++m_next_req_id;
    if (m_next_req_id == 0) {
        ++m_next_req_id;
    }
    return m_next_req_id;
}

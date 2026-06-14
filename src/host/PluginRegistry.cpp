#include "PluginRegistry.h"
#include "Host.h"
#include "interfaces/instance.h"

#include <algorithm>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QStringList>
#include <Qt>
#include <common.h>
#include <silent.h>

namespace {
class MetadataOnlyPlugin final : public IPlugin {
public:
    explicit MetadataOnlyPlugin(QMap<QString, QString> meta) : m_meta(std::move(meta)) {}

    [[nodiscard]] QMap<QString, QString> meta() const override {
        return m_meta;
    }

    [[nodiscard]] QList<IModule *> modules() const override {
        return {};
    }

private:
    QMap<QString, QString> m_meta;
};

QMap<QString, QString> readSidecarMeta(const QString &path) {
    QFile sidecar(path);
    if (!sidecar.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Plugin sidecar open failed:" << path << sidecar.errorString();
        return {};
    }

    QJsonParseError parseError;
    auto doc = QJsonDocument::fromJson(sidecar.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Plugin sidecar parse failed:" << path << parseError.errorString();
        return {};
    }

    QMap<QString, QString> meta;
    auto object = doc.object();
    for (auto it = object.constBegin(); it != object.constEnd(); ++it) {
        if (it.value().isString()) {
            meta.insert(it.key(), it.value().toString());
            continue;
        }
        if (it.value().isDouble()) {
            meta.insert(it.key(), QString::number(it.value().toDouble()));
            continue;
        }
        if (it.value().isBool()) {
            meta.insert(it.key(), it.value().toBool() ? QStringLiteral("true") : QStringLiteral("false"));
            continue;
        }
        if (it.value().isArray()) {
            QStringList values;
            auto array = it.value().toArray();
            for (const auto &entry : array) {
                if (entry.isString()) {
                    values.append(entry.toString());
                }
            }
            meta.insert(it.key(), values.join(','));
        }
    }

    return meta;
}

} // namespace

PluginRegistry::PluginRegistry(QObject *parent) : QObject(parent) {
    auto *host = Host::get();
    if (host != nullptr && host->events() != nullptr) {
        connect(host->events(), &HostEvents::instanceRegistered, this,
                &PluginRegistry::onHostInstanceRegistered, Qt::UniqueConnection);
        connect(host->events(), &HostEvents::instanceRemoved, this,
                &PluginRegistry::onHostInstanceRemoved, Qt::UniqueConnection);
    }
    rescanDiscoveredPlugins();
    reloadEnabledPlugins();
}

PluginRegistry::~PluginRegistry() {
    for (auto it = m_external_plugins.begin(); it != m_external_plugins.end(); ++it) {
        disablePlugin(it.key());
        if (it->loader != nullptr) {
            delete it->loader;
            it->loader = nullptr;
        }
    }
    qDeleteAll(m_metadata_plugins);
    m_metadata_plugins.clear();
}

void PluginRegistry::rescanDiscoveredPlugins() {
    for (auto it = m_external_plugins.begin(); it != m_external_plugins.end(); ++it) {
        it->discovered = false;
    }

    QString pluginsRoot = QDir::homePath() + QStringLiteral("/.silent/plugins");
    QDir rootDir(pluginsRoot);
    if (!rootDir.exists()) {
        rebuildPluginList();
        emit enabledPluginsChanged();
        return;
    }

    auto pluginDirs = rootDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const auto &pluginDirInfo : pluginDirs) {
        QString pluginDir = pluginDirInfo.absoluteFilePath();
        QString pluginId = pluginDirInfo.fileName().trimmed();
        if (pluginId.isEmpty()) {
            continue;
        }

        QString sidecarPath = pluginDir + QStringLiteral("/%1.json").arg(pluginId);
        auto meta = readSidecarMeta(sidecarPath);
        if (meta.isEmpty()) {
            continue;
        }

        QString metaId = meta.value(QStringLiteral("id")).trimmed();
        if (metaId.isEmpty() || metaId != pluginId) {
            qWarning() << "Plugin sidecar id mismatch:" << sidecarPath;
            continue;
        }

        auto &record = m_external_plugins[pluginId];
        record.id = pluginId;
        record.name = meta.value(QStringLiteral("name")).trimmed();
        record.version = meta.value(QStringLiteral("version")).trimmed();
        record.meta = meta;
        record.i18nDir = pluginDir + QStringLiteral("/i18n");
        record.libraryPath = resolvePluginLibraryPath(pluginDir, meta);
        record.discovered = true;

        if (record.plugin == nullptr) {
            record.pluginObject = nullptr;
            record.modules.clear();
        }
    }

    for (auto it = m_external_plugins.constBegin(); it != m_external_plugins.constEnd(); ++it) {
        if (!it->discovered && it->plugin != nullptr) {
            qWarning() << "Plugin no longer discoverable on disk; keeping runtime state until explicit disable:"
                       << it.key();
        }
    }

    rebuildPluginList();
    emit enabledPluginsChanged();
}

void PluginRegistry::reloadEnabledPlugins() {
    QHash<QString, bool> desiredEnabled;
    auto enabled = app_enabled_plugins();
    for (auto pluginId : enabled) {
        desiredEnabled.insert(QString::fromStdString(std::string(pluginId.c_str())), true);
    }

    for (const auto &id : m_plugin_enabled.keys()) {
        desiredEnabled.insert(id, desiredEnabled.value(id, false));
    }
    for (const auto &id : m_builtin_plugin_ids) {
        desiredEnabled.insert(id, true);
    }

    bool changed = false;
    for (auto it = desiredEnabled.constBegin(); it != desiredEnabled.constEnd(); ++it) {
        bool prev = m_plugin_enabled.value(it.key(), false);
        if (prev != it.value()) {
            changed = true;
        }
        m_plugin_enabled.insert(it.key(), it.value());
    }

    for (const auto &id : m_builtin_plugin_ids) {
        m_plugin_enabled.insert(id, true);
    }

    for (auto it = m_external_plugins.constBegin(); it != m_external_plugins.constEnd(); ++it) {
        if (m_plugin_enabled.value(it.key(), false)) {
            if (!enablePlugin(it.key())) {
                // Keep runtime enabled state truthful when load fails.
                if (m_plugin_enabled.value(it.key(), false)) {
                    changed = true;
                }
                m_plugin_enabled.insert(it.key(), false);
                ::app_set_plugin_enabled(rust::String(it.key().toStdString()), false);
            }
            continue;
        }
        disablePlugin(it.key());
    }

    if (changed) {
        emit enabledPluginsChanged();
    }
}

void PluginRegistry::registerBuiltin(IPlugin *plugin) {
    if (plugin == nullptr || m_plugin_modules.contains(plugin)) {
        return;
    }

    m_plugins.append(plugin);
    QString id = pluginId(plugin);
    m_builtin_plugin_ids.insert(id);
    bool hadEntry = m_plugin_enabled.contains(id);
    bool wasEnabled = m_plugin_enabled.value(id, false);
    m_plugin_enabled.insert(id, true);
    syncPluginTranslations(id, true);
    if (hadEntry && !wasEnabled) {
        ::app_set_plugin_enabled(rust::String(id.toStdString()), true);
    }

    QList<IModule *> modules = plugin->modules();
    std::ranges::remove(modules, nullptr);
    m_plugin_modules.insert(plugin, modules);
    for (auto *module : modules) {
        m_builtin_modules.insert(module);
        activateModule(module, plugin);
    }

    rebuildPluginList();
}

QList<IPlugin *> PluginRegistry::plugins() const {
    return m_listed_plugins;
}

QList<IModule *> PluginRegistry::enabledModulesForLauncher() const {
    QList<IModule *> enabled;
    for (auto it = m_plugin_modules.constBegin(); it != m_plugin_modules.constEnd(); ++it) {
        if (!isPluginEnabled(pluginId(it.key()))) {
            continue;
        }
        for (auto *module : it.value()) {
            if (module != nullptr) {
                enabled.append(module);
            }
        }
    }
    return enabled;
}

QList<QPair<QString, QString>> PluginRegistry::launcherInstancesForModule(
    const IModule *module) const {
    auto *nonConstModule = const_cast<IModule *>(module);
    return m_launcher_instances.value(nonConstModule);
}

bool PluginRegistry::isCreatorInstanceId(const QString &id) {
    QString trimmed = id.trimmed();
    return trimmed == QStringLiteral("create") || trimmed.endsWith(QStringLiteral(".create"));
}

bool PluginRegistry::isBuiltinModule(IModule *module) const {
    return m_builtin_modules.contains(module);
}

QList<PluginRegistry::CreatorLauncher> PluginRegistry::externalCreatorLaunchers() const {
    QList<CreatorLauncher> creators;
    for (auto *module : enabledModulesForLauncher()) {
        if (module == nullptr || isBuiltinModule(module)) {
            continue;
        }
        for (const auto &instance : launcherInstancesForModule(module)) {
            if (isCreatorInstanceId(instance.first)) {
                creators.append({module, instance.first.trimmed(), instance.second.trimmed()});
            }
        }
    }
    return creators;
}

bool PluginRegistry::startLauncherInstance(IModule *module, const QString &instance_id) {
    return startOwnedInstance(module, instance_id);
}

void PluginRegistry::deleteInstance(IModule *module, const QString &instance_id) {
    QString trimmedId = instance_id.trimmed();
    if (trimmedId.isEmpty()) {
        return;
    }

    auto *host = Host::get();
    auto *hostImpl = dynamic_cast<HostImpl *>(host);
    if (host != nullptr && hostImpl != nullptr) {
        for (auto *instance : host->instances()) {
            if (instance != nullptr && instance->id() == trimmedId) {
                instance->stop();
                hostImpl->removeInstance(trimmedId);
                break;
            }
        }
    }
    m_instance_owner_by_id.remove(trimmedId);

    // The module owns its persistence: ask it to drop the stored state.
    if (module != nullptr) {
        module->deleteInstance(trimmedId);
    }

    emit enabledPluginsChanged();
}

bool PluginRegistry::isPluginEnabled(const QString &plugin_id) const {
    return m_plugin_enabled.value(plugin_id, false);
}

bool PluginRegistry::isPluginBuiltin(const QString &plugin_id) const {
    return m_builtin_plugin_ids.contains(plugin_id);
}

bool PluginRegistry::pluginHasBuiltinModules(const QString &plugin_id) const {
    auto *plugin = pluginById(plugin_id);
    if (plugin == nullptr) {
        return false;
    }

    auto modules = m_plugin_modules.value(plugin);
    // NOLINTNEXTLINE(readability-use-anyofallof)
    for (auto *module : modules) {
        if (m_builtin_modules.contains(module)) {
            return true;
        }
    }
    return false;
}

void PluginRegistry::setEnabled(const QString &plugin_id, bool enabled) {
    if (plugin_id.trimmed().isEmpty()) {
        return;
    }

    if (pluginHasBuiltinModules(plugin_id) && !enabled) {
        return;
    }

    bool previous = m_plugin_enabled.value(plugin_id, false);
    if (previous == enabled) {
        return;
    }

    if (enabled) {
        m_plugin_enabled.insert(plugin_id, true);
        if (!enablePlugin(plugin_id)) {
            m_plugin_enabled.insert(plugin_id, false);
            return;
        }
    } else {
        if (!disablePlugin(plugin_id)) {
            m_plugin_enabled.insert(plugin_id, true);
            return;
        }
        m_plugin_enabled.insert(plugin_id, false);
    }

    syncPluginTranslations(plugin_id, enabled);
    ::app_set_plugin_enabled(rust::String(plugin_id.toStdString()), enabled);
    emit enabledPluginsChanged();
}

void PluginRegistry::onModuleInstances(ReqId req_id, QList<QPair<QString, QString>> instances) {
    auto *module = qobject_cast<IModule *>(sender());
    if (module == nullptr) {
        return;
    }

    auto pendingIt = m_pending_list_requests.find(module);
    if (pendingIt == m_pending_list_requests.end() || !pendingIt->remove(req_id)) {
        if (req_id != 0 && m_module_owner.contains(module)) {
            m_sync_list_responses.insert(module, {req_id, std::move(instances)});
        }
        return;
    }
    if (pendingIt->isEmpty()) {
        m_pending_list_requests.erase(pendingIt);
    }

    if (!m_module_owner.contains(module)) {
        return;
    }
    m_launcher_instances.insert(module, instances);
}

void PluginRegistry::onModuleAutoStart(ReqId req_id, QStringList ids) {
    auto *module = qobject_cast<IModule *>(sender());
    if (module == nullptr) {
        return;
    }

    auto pendingIt = m_pending_auto_start_requests.find(module);
    if (pendingIt == m_pending_auto_start_requests.end() || !pendingIt->remove(req_id)) {
        if (req_id != 0 && m_module_owner.contains(module)) {
            m_sync_auto_start_responses.insert(module, {req_id, std::move(ids)});
        }
        return;
    }
    if (pendingIt->isEmpty()) {
        m_pending_auto_start_requests.erase(pendingIt);
    }

    if (!m_module_owner.contains(module)) {
        return;
    }

    for (const auto &id : ids) {
        startOwnedInstance(module, id);
    }
}

void PluginRegistry::onModuleError(std::optional<ReqId> req_id, const QString &message) {
    qWarning() << "Module error:" << message;
    if (!req_id.has_value()) {
        return;
    }

    auto *module = qobject_cast<IModule *>(sender());
    if (module == nullptr) {
        return;
    }

    if (auto listIt = m_pending_list_requests.find(module); listIt != m_pending_list_requests.end()) {
        listIt->remove(req_id.value());
        if (listIt->isEmpty()) {
            m_pending_list_requests.erase(listIt);
        }
    }

    if (auto autoIt = m_pending_auto_start_requests.find(module);
        autoIt != m_pending_auto_start_requests.end()) {
        autoIt->remove(req_id.value());
        if (autoIt->isEmpty()) {
            m_pending_auto_start_requests.erase(autoIt);
        }
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QString PluginRegistry::pluginId(IPlugin *plugin) const {
    if (plugin == nullptr) {
        return QString();
    }
    auto meta = plugin->meta();
    QString id = meta.value(QStringLiteral("id")).trimmed();
    if (!id.isEmpty()) {
        return id;
    }
    return QStringLiteral("builtin.%1").arg(reinterpret_cast<quintptr>(plugin), 0, 16);
}

QString PluginRegistry::pluginI18nDir(IPlugin *plugin) const {
    if (plugin == nullptr) {
        return {};
    }

    auto meta = plugin->meta();
    QString explicitDir = meta.value(QStringLiteral("i18n_dir")).trimmed();
    if (!explicitDir.isEmpty()) {
        return explicitDir;
    }

    QString id = pluginId(plugin);
    if (id.isEmpty()) {
        return {};
    }
    auto externalIt = m_external_plugins.find(id);
    if (externalIt != m_external_plugins.end() && !externalIt->i18nDir.isEmpty()) {
        return externalIt->i18nDir;
    }
    return QDir::homePath() + QStringLiteral("/.silent/plugins/%1/i18n").arg(id);
}

void PluginRegistry::syncPluginTranslations(const QString &plugin_id, bool enabled) {
    auto *plugin = pluginById(plugin_id);
    auto *host = Host::get();
    if (plugin == nullptr || host == nullptr || host->i18n() == nullptr) {
        return;
    }

    if (!enabled) {
        host->i18n()->unregisterPluginTranslations(plugin_id);
        return;
    }

    QString dir = pluginI18nDir(plugin);
    if (dir.isEmpty() || !QFileInfo::exists(dir) || !QFileInfo(dir).isDir()) {
        host->i18n()->unregisterPluginTranslations(plugin_id);
        return;
    }
    host->i18n()->registerPluginTranslations(plugin_id, dir);
}

IPlugin *PluginRegistry::pluginById(const QString &plugin_id) const {
    if (plugin_id.trimmed().isEmpty()) {
        return nullptr;
    }

    for (auto *plugin : m_plugins) {
        if (plugin != nullptr && this->pluginId(plugin) == plugin_id) {
            return plugin;
        }
    }
    auto externalIt = m_external_plugins.find(plugin_id);
    if (externalIt != m_external_plugins.end()) {
        return externalIt->plugin;
    }
    return nullptr;
}

bool PluginRegistry::enablePlugin(const QString &plugin_id) {
    if (m_builtin_plugin_ids.contains(plugin_id)) {
        syncPluginTranslations(plugin_id, true);
        return true;
    }

    if (!m_external_plugins.contains(plugin_id)) {
        qWarning() << "Enable skipped: plugin not discovered" << plugin_id;
        return false;
    }

    auto &record = m_external_plugins[plugin_id];
    if (!record.discovered) {
        qWarning() << "Enable skipped: plugin is not currently discovered" << plugin_id;
        return false;
    }
    if (record.plugin != nullptr) {
        syncPluginTranslations(plugin_id, true);
        return true;
    }
    if (record.loader != nullptr) {
        qWarning() << "Enable blocked: plugin still has active loader; disable/unload cleanup required"
                   << plugin_id;
        return false;
    }

    if (record.libraryPath.isEmpty()) {
        qWarning() << "Enable failed: plugin library missing" << plugin_id;
        return false;
    }

    record.loader = new QPluginLoader(record.libraryPath);
    if (!record.loader->load()) {
        qWarning() << "Plugin load failed:" << plugin_id << record.loader->errorString();
        delete record.loader;
        record.loader = nullptr;
        return false;
    }

    QObject *root = record.loader->instance();
    if (root == nullptr) {
        qWarning() << "Plugin instance failed:" << plugin_id << record.loader->errorString();
        record.loader->unload();
        delete record.loader;
        record.loader = nullptr;
        return false;
    }

    auto *plugin = qobject_cast<IPlugin *>(root);
    if (plugin == nullptr) {
        qWarning() << "Plugin interface cast failed:" << plugin_id << record.loader->errorString();
        record.loader->unload();
        delete record.loader;
        record.loader = nullptr;
        return false;
    }

    record.pluginObject = root;
    record.plugin = plugin;
    record.modules = plugin->modules();
    std::ranges::remove(record.modules, nullptr);

    m_plugins.append(plugin);
    m_plugin_modules.insert(plugin, record.modules);
    for (auto *module : record.modules) {
        activateModule(module, plugin);
    }
    rebuildPluginList();
    syncPluginTranslations(plugin_id, true);
    return true;
}

bool PluginRegistry::disablePlugin(const QString &plugin_id) {
    if (m_builtin_plugin_ids.contains(plugin_id)) {
        return false;
    }

    if (!m_external_plugins.contains(plugin_id)) {
        return false;
    }

    auto &record = m_external_plugins[plugin_id];
    if (record.plugin == nullptr) {
        bool unloaded = unloadPlugin(plugin_id);
        if (!unloaded) {
            qWarning() << "Plugin unload failed:" << plugin_id;
            return false;
        }
        // Disabling tears down/unloads only; discovery state is owned by
        // rescanDiscoveredPlugins(), so the plugin stays listed as discovered+disabled.
        syncPluginTranslations(plugin_id, false);
        rebuildPluginList();
        return true;
    }

    stopPluginInstances(plugin_id);
    if (hasLiveOwnedInstances(plugin_id)) {
        qWarning() << "Plugin disable aborted: live instances still owned by plugin:" << plugin_id;
        return false;
    }

    for (auto *module : record.modules) {
        deactivateModule(module);
    }

    bool unloaded = unloadPlugin(plugin_id);
    if (!unloaded) {
        qWarning() << "Plugin unload failed:" << plugin_id;
        // Roll back module wiring best-effort so runtime state stays internally consistent.
        for (auto *module : record.modules) {
            activateModule(module, record.plugin);
        }
        return false;
    }

    m_plugin_modules.remove(record.plugin);
    m_plugins.removeAll(record.plugin);
    record.modules.clear();
    record.plugin = nullptr;
    record.pluginObject = nullptr;
    // Disabling tears down/unloads only; discovery state is owned by
    // rescanDiscoveredPlugins(), so the plugin stays listed as discovered+disabled.

    syncPluginTranslations(plugin_id, false);
    rebuildPluginList();
    return true;
}

bool PluginRegistry::unloadPlugin(const QString &plugin_id) {
    if (!m_external_plugins.contains(plugin_id)) {
        return false;
    }

    auto &record = m_external_plugins[plugin_id];
    if (record.loader == nullptr) {
        return true;
    }

    bool ok = record.loader->unload();
    if (!ok) {
        qWarning() << "Plugin unload error:" << plugin_id << record.loader->errorString();
        return false;
    }

    delete record.loader;
    record.loader = nullptr;
    return true;
}

void PluginRegistry::activateModule(IModule *module, IPlugin *plugin) {
    if (module == nullptr) {
        return;
    }

    m_module_owner.insert(module, plugin);
    connect(module, &IModule::instances, this, &PluginRegistry::onModuleInstances,
            Qt::UniqueConnection);
    connect(module, QOverload<ReqId, QStringList>::of(&IModule::autoStart), this,
            &PluginRegistry::onModuleAutoStart, Qt::UniqueConnection);
    connect(module, &IModule::error, this, &PluginRegistry::onModuleError,
            Qt::UniqueConnection);

    ReqId autoReqId = module->autoStart();
    if (autoReqId != 0) {
        m_pending_auto_start_requests[module].insert(autoReqId);
        if (m_sync_auto_start_responses.contains(module)
            && m_sync_auto_start_responses.value(module).first == autoReqId) {
            auto response = m_sync_auto_start_responses.take(module);
            onModuleAutoStart(response.first, response.second);
        }
    }

    ReqId listReqId = module->list();
    if (listReqId != 0) {
        m_pending_list_requests[module].insert(listReqId);
        if (m_sync_list_responses.contains(module)
            && m_sync_list_responses.value(module).first == listReqId) {
            auto response = m_sync_list_responses.take(module);
            onModuleInstances(response.first, response.second);
        }
    }
}

void PluginRegistry::deactivateModule(IModule *module) {
    if (module == nullptr) {
        return;
    }

    disconnect(module, &IModule::instances, this, &PluginRegistry::onModuleInstances);
    disconnect(module, QOverload<ReqId, QStringList>::of(&IModule::autoStart), this,
               &PluginRegistry::onModuleAutoStart);
    disconnect(module, &IModule::error, this, &PluginRegistry::onModuleError);

    m_pending_list_requests.remove(module);
    m_pending_auto_start_requests.remove(module);
    m_sync_list_responses.remove(module);
    m_sync_auto_start_responses.remove(module);
    m_launcher_instances.remove(module);
    m_module_owner.remove(module);
}

bool PluginRegistry::startOwnedInstance(IModule *module, const QString &instance_id) {
    if (module == nullptr) {
        return false;
    }

    auto ownerIt = m_module_owner.find(module);
    if (ownerIt == m_module_owner.end() || ownerIt.value() == nullptr) {
        return false;
    }

    QString ownerPluginId = pluginId(ownerIt.value());
    QString trimmedInstanceId = instance_id.trimmed();
    if (ownerPluginId.isEmpty() || trimmedInstanceId.isEmpty()) {
        return false;
    }

    m_pending_start_owner_plugin_id = ownerPluginId;
    module->startInstance(trimmedInstanceId);
    m_pending_start_owner_plugin_id.clear();
    m_instance_owner_by_id.insert(trimmedInstanceId, ownerPluginId);
    return true;
}

void PluginRegistry::stopPluginInstances(const QString &plugin_id) {
    auto *host = Host::get();
    if (host == nullptr) {
        return;
    }

    auto *hostImpl = dynamic_cast<HostImpl *>(host);
    if (hostImpl == nullptr) {
        return;
    }

    QSet<QString> ownedIds;
    for (auto it = m_instance_owner_by_id.constBegin(); it != m_instance_owner_by_id.constEnd(); ++it) {
        if (it.value() == plugin_id) {
            ownedIds.insert(it.key());
        }
    }

    if (ownedIds.isEmpty()) {
        return;
    }

    auto liveInstances = host->instances();
    QHash<QString, IInstance *> liveById;
    liveById.reserve(liveInstances.size());
    for (auto *instance : liveInstances) {
        if (instance == nullptr) {
            continue;
        }
        liveById.insert(instance->id(), instance);
    }

    for (const auto &instanceId : ownedIds) {
        auto *instance = liveById.value(instanceId, nullptr);
        if (instance != nullptr) {
            instance->stop();
            hostImpl->removeInstance(instanceId);
        }

        bool stillLive = false;
        auto afterRemove = host->instances();
        for (auto *live : afterRemove) {
            if (live != nullptr && live->id() == instanceId) {
                stillLive = true;
                break;
            }
        }
        if (stillLive) {
            qWarning() << "Owned plugin instance still live after stop/remove:" << plugin_id
                       << instanceId;
            continue;
        }
        m_instance_owner_by_id.remove(instanceId);
    }
}

bool PluginRegistry::hasLiveOwnedInstances(const QString &plugin_id) const {
    auto *host = Host::get();
    if (host == nullptr) {
        return false;
    }

    auto liveInstances = host->instances();
    return std::ranges::any_of(liveInstances, [this, &plugin_id](const auto *instance) {
        return instance != nullptr && m_instance_owner_by_id.value(instance->id()) == plugin_id;
    });
}

void PluginRegistry::onHostInstanceRegistered(const QString &id) {
    QString instanceId = id.trimmed();
    if (instanceId.isEmpty()) {
        return;
    }
    if (m_pending_start_owner_plugin_id.isEmpty()) {
        return;
    }
    if (!m_instance_owner_by_id.contains(instanceId)) {
        m_instance_owner_by_id.insert(instanceId, m_pending_start_owner_plugin_id);
    }
}

void PluginRegistry::onHostInstanceRemoved(const QString &id) {
    m_instance_owner_by_id.remove(id.trimmed());
}

void PluginRegistry::rebuildPluginList() {
    qDeleteAll(m_metadata_plugins);
    m_metadata_plugins.clear();
    m_listed_plugins.clear();

    for (auto *plugin : m_plugins) {
        if (plugin != nullptr) {
            m_listed_plugins.append(plugin);
        }
    }

    for (auto it = m_external_plugins.constBegin(); it != m_external_plugins.constEnd(); ++it) {
        if (!it->discovered || it->plugin != nullptr) {
            continue;
        }

        auto meta = it->meta;
        if (!meta.contains(QStringLiteral("id"))) {
            meta.insert(QStringLiteral("id"), it.key());
        }
        if (!meta.contains(QStringLiteral("name")) && !it->name.isEmpty()) {
            meta.insert(QStringLiteral("name"), it->name);
        }
        if (!meta.contains(QStringLiteral("version")) && !it->version.isEmpty()) {
            meta.insert(QStringLiteral("version"), it->version);
        }

        auto *placeholder = new MetadataOnlyPlugin(meta);
        m_metadata_plugins.append(placeholder);
        m_listed_plugins.append(placeholder);
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QString PluginRegistry::resolvePluginLibraryPath(const QString &plugin_dir,
                                                 const QMap<QString, QString> &meta) const {
    QString explicitLibrary = meta.value(QStringLiteral("library")).trimmed();
    if (!explicitLibrary.isEmpty()) {
        QFileInfo explicitInfo(QDir(plugin_dir).absoluteFilePath(explicitLibrary));
        if (explicitInfo.exists() && explicitInfo.isFile()) {
            return explicitInfo.absoluteFilePath();
        }
    }

    QString pluginId = meta.value(QStringLiteral("id")).trimmed();
    if (!pluginId.isEmpty()) {
#ifdef Q_OS_MAC
        QString candidate = QDir(plugin_dir).absoluteFilePath(QStringLiteral("lib%1.dylib").arg(pluginId));
#elif defined(Q_OS_WIN)
        QString candidate = QDir(plugin_dir).absoluteFilePath(QStringLiteral("%1.dll").arg(pluginId));
#else
        QString candidate = QDir(plugin_dir).absoluteFilePath(QStringLiteral("lib%1.so").arg(pluginId));
#endif
        if (QFileInfo::exists(candidate) && QFileInfo(candidate).isFile()) {
            return candidate;
        }
    }

#ifdef Q_OS_MAC
    QStringList filters{QStringLiteral("*.dylib")};
#elif defined(Q_OS_WIN)
    QStringList filters{QStringLiteral("*.dll")};
#else
    QStringList filters{QStringLiteral("*.so")};
#endif

    QDir dir(plugin_dir);
    auto candidates = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    if (!candidates.isEmpty()) {
        return candidates.first().absoluteFilePath();
    }

    return {};
}

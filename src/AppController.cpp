#include "AppController.h"
#include "host/Host.h"
#include "host/PluginRegistry.h"
#include "interfaces/instance.h"
#include "interfaces/module.h"
#include "interfaces/theme.h"
#include "screens/modals/AppSettings.h"
#include "screens/modals/ConfirmDelete.h"
#include "theme/Theme.h"
#include <algorithm>
#include <common.h>
#include <qlogging.h>
#include <qthread.h>
#include <qtimer.h>

namespace {
constexpr int LIST_ACCOUNTS_TIMEOUT_MS = 3000;
}

AppController::AppController() = default;
AppController::~AppController() = default;

auto AppController::init() -> void {
    if (Controller::isInit()) {
        qFatal("Controller has already been initialized!");
    }
    Controller::init(new AppController);
}

auto AppController::get() -> AppController * {
    auto *ctrl = Controller::get();
    auto *controller = dynamic_cast<AppController *>(ctrl);
    return controller;
}

auto AppController::pluginRegistry() const -> PluginRegistry * {
    return m_plugin_registry.get();
}

auto AppController::registerBuiltinPlugin(std::unique_ptr<IPlugin> plugin) -> void {
    if (!plugin) {
        return;
    }

    if (!m_plugin_registry) {
        m_plugin_registry = std::make_unique<PluginRegistry>();
    }

    auto *raw = plugin.get();
    m_builtin_plugins.push_back(std::move(plugin));
    m_plugin_registry->registerBuiltin(raw);
}

auto AppController::initState() -> void {
    if (!m_plugin_registry) {
        m_plugin_registry = std::make_unique<PluginRegistry>();
    }

    m_host = Host::get();
    if (m_host != nullptr && m_host->events() != nullptr) {
        connect(m_host->events(), &HostEvents::instanceRegistered, this,
                &AppController::onHostInstanceRegistered, qontrol::UNIQUE);
        connect(m_host->events(), &HostEvents::instanceRemoved, this,
                &AppController::onHostInstanceRemoved, qontrol::UNIQUE);
        connect(m_host->events(), &HostEvents::settingSectionsChanged, this,
                &AppController::onHostSettingsSectionsChanged, qontrol::UNIQUE);
    }

    connect(this, &AppController::accountCreated, this, &AppController::onAccountCreated,
            qontrol::UNIQUE);
    connect(this, &AppController::regtestDefaultsReady, this,
            &AppController::onRegtestDefaultsReady, qontrol::UNIQUE);

    applyPersistedThemeAtStartup();
    listAccounts();

    auto *thread = QThread::create([this]() -> void {
        auto defaults = ::get_regtest_defaults();
        if (defaults.is_ok) {
            auto blindbit = QString::fromStdString(std::string(defaults.blindbit_url.c_str()));
            auto p2p = QString::fromStdString(std::string(defaults.p2p_node.c_str()));
            auto electrum = QString::fromStdString(std::string(defaults.electrum_url.c_str()));
            emit regtestDefaultsReady(blindbit, p2p, electrum);
        }
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

auto AppController::pluginIdForModule(const IModule *module) const -> QString {
    if (module == nullptr || m_plugin_registry == nullptr) {
        return {};
    }

    for (auto *plugin : m_plugin_registry->plugins()) {
        if (plugin == nullptr) {
            continue;
        }
        const auto modules = plugin->modules();
        bool ownsModule = std::ranges::find(modules, module) != modules.end();
        if (!ownsModule) {
            continue;
        }
        return plugin->meta().value(QStringLiteral("id")).trimmed();
    }

    return {};
}

auto AppController::moduleForPluginId(const QString &plugin_id) const -> IModule * {
    if (plugin_id.isEmpty() || m_plugin_registry == nullptr) {
        return nullptr;
    }

    const auto modules = m_plugin_registry->enabledModulesForLauncher();
    for (auto *module : modules) {
        if (module == nullptr) {
            continue;
        }
        if (pluginIdForModule(module) == plugin_id) {
            return module;
        }
    }

    return nullptr;
}

auto AppController::pluginIdForAccount(const QString &account_id) const -> QString {
    auto cached = m_account_plugin_ids.value(account_id);
    if (!cached.isEmpty()) {
        return cached;
    }

    auto config = ::config_from_file(rust::String(account_id.toStdString()));
    return QString::fromStdString(std::string(config->get_plugin_id().c_str()));
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto AppController::addAccount(const QString &name) -> void {
    if (isAccountOpen(name)) {
        qCritical() << "Account " << name << " already exists!";
        return;
    }

    QString pluginId = pluginIdForAccount(name);
    auto *module = moduleForPluginId(pluginId);
    if (module == nullptr) {
        qCritical() << "No enabled module found for plugin" << pluginId << "account" << name;
        return;
    }

    if (!m_plugin_registry->startLauncherInstance(module, name)) {
        qCritical() << "Failed to start account instance:" << name;
        return;
    }
    m_open_accounts.insert(name);
}

auto AppController::removeAccount(const QString &account) -> void {
    if (m_host != nullptr) {
        auto activeInstances = m_host->instances();
        for (auto *instance : activeInstances) {
            if (instance != nullptr && instance->id() == account) {
                instance->stop();
                if (auto *hostImpl = dynamic_cast<HostImpl *>(m_host); hostImpl != nullptr) {
                    hostImpl->removeInstance(account);
                }
                break;
            }
        }
    }

    m_open_accounts.remove(account);
    listAccounts();
}

auto AppController::finalizeListAccounts() -> void {
    QList<QString> accounts;
    for (const auto &id : m_config_order) {
        if (m_discovered_accounts.contains(id)) {
            accounts.append(id);
        }
    }

    if (accounts.isEmpty()) {
        emit accountList(m_config_order);
        return;
    }
    emit accountList(accounts);
}

auto AppController::listAccounts() -> void {
    ++m_list_accounts_epoch;
    auto listEpoch = m_list_accounts_epoch;

    m_config_order.clear();
    m_known_configs.clear();
    m_discovered_accounts.clear();
    m_pending_list_modules = 0;
    m_pending_list_requests.clear();
    for (const auto &connection : m_pending_list_connections) {
        disconnect(connection);
    }
    m_pending_list_connections.clear();

    auto pendingSyncResponses =
        std::make_shared<QHash<IModule *, QPair<ReqId, QList<QPair<QString, QString>>>>>();

    auto handleInstances = [this, listEpoch](IModule *module, const QString &plugin_id, ReqId req_id,
                                                   const QList<QPair<QString, QString>> &instances) {
        if (m_list_accounts_epoch != listEpoch) {
            return;
        }

        if (!m_pending_list_requests.contains(module)
            || m_pending_list_requests.value(module) != req_id) {
            return;
        }

        m_pending_list_requests.remove(module);
        if (m_pending_list_connections.contains(module)) {
            disconnect(m_pending_list_connections.take(module));
        }

        for (const auto &instance : instances) {
            const auto &accountId = instance.first;
            if (!m_known_configs.contains(accountId)) {
                continue;
            }
            m_discovered_accounts.insert(accountId);
            m_account_plugin_ids.insert(accountId, plugin_id);
        }

        --m_pending_list_modules;
        if (m_pending_list_modules == 0) {
            finalizeListAccounts();
        }
    };

    const auto raccounts = list_configs();
    for (auto acc : raccounts) {
        const auto account = QString::fromStdString(std::string(acc.c_str()));
        m_config_order.append(account);
        m_known_configs.insert(account);
    }

    if (m_plugin_registry == nullptr) {
        emit accountList(m_config_order);
        return;
    }

    const auto modules = m_plugin_registry->enabledModulesForLauncher();
    if (modules.isEmpty()) {
        emit accountList(m_config_order);
        return;
    }

    m_pending_list_modules = modules.size();
    for (auto *module : modules) {
        if (module == nullptr) {
            --m_pending_list_modules;
            if (m_pending_list_modules == 0) {
                finalizeListAccounts();
            }
            continue;
        }

        if (m_pending_list_connections.contains(module)) {
            disconnect(m_pending_list_connections.take(module));
        }

        QString pluginId = pluginIdForModule(module);
        const auto connection = connect(module, &IModule::instances, this,
                                        [this, module, pluginId, pendingSyncResponses, handleInstances](
                                            ReqId req_id, QList<QPair<QString, QString>> instances) {
                                            if (!m_pending_list_requests.contains(module)) {
                                                pendingSyncResponses->insert(module, {req_id, std::move(instances)});
                                                return;
                                            }
                                            handleInstances(module, pluginId, req_id, instances);
                                        });
        m_pending_list_connections.insert(module, connection);

        ReqId reqId = module->list();
        if (reqId == 0) {
            disconnect(m_pending_list_connections.take(module));
            --m_pending_list_modules;
            if (m_pending_list_modules == 0) {
                finalizeListAccounts();
            }
            continue;
        }
        m_pending_list_requests.insert(module, reqId);

        QTimer::singleShot(LIST_ACCOUNTS_TIMEOUT_MS, this, [this, listEpoch, module]() {
            if (m_list_accounts_epoch != listEpoch) {
                return;
            }

            if (!m_pending_list_requests.contains(module)) {
                return;
            }

            m_pending_list_requests.remove(module);
            if (m_pending_list_connections.contains(module)) {
                disconnect(m_pending_list_connections.take(module));
            }

            --m_pending_list_modules;
            if (m_pending_list_modules == 0) {
                finalizeListAccounts();
            }
        });

        if (pendingSyncResponses->contains(module)) {
            const auto response = pendingSyncResponses->take(module);
            handleInstances(module, pluginId, response.first, response.second);
        }
    }

    if (m_pending_list_modules == 0) {
        finalizeListAccounts();
    }
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto AppController::createAccount(const QString &name, const QString &mnemonic, Network network,
                                  const QString &blindbit_url, const QString &p2p_node,
                                  const QString &electrum_url) -> void {
    Q_UNUSED(name);
    Q_UNUSED(mnemonic);
    Q_UNUSED(network);
    Q_UNUSED(blindbit_url);
    Q_UNUSED(p2p_node);
    Q_UNUSED(electrum_url);
    qCritical() << "createAccount is deprecated in AppController and should be handled by module UI";
}

auto AppController::onAccountCreated(const QString &name) -> void {
    addAccount(name);
    listAccounts();
}

auto AppController::openAccount(const QString &name) -> void {
    addAccount(name);
    listAccounts();
}

// NOLINTNEXTLINE(readability-make-member-function-const)
auto AppController::deleteAccount(const QString &name) -> void {
    auto *modal = new modal::ConfirmDelete(name);
    connect(modal, &modal::ConfirmDelete::confirmed, this, &AppController::onDeleteConfirmed,
            qontrol::UNIQUE);
    AppController::execModal(modal);
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto AppController::openAppSettings() -> void {
    AppController::execModal(new modal::AppSettings());
}

auto AppController::onDeleteConfirmed(const QString &account) -> void {
    if (isAccountOpen(account)) {
        removeAccount(account);
    }
    if (!delete_config(rust::String(account.toStdString()))) {
        qCritical() << "Failed to delete account:" << account;
    }
    m_account_plugin_ids.remove(account);
    listAccounts();
}

auto AppController::onHostInstanceRemoved(const QString &id) -> void {
    m_open_accounts.remove(id);
    applyPersistedThemeAtStartup();
}

auto AppController::onHostInstanceRegistered(const QString &id) -> void {
    Q_UNUSED(id);
    applyPersistedThemeAtStartup();
}

auto AppController::onHostSettingsSectionsChanged() -> void {
    if (m_plugin_registry == nullptr) {
        return;
    }
    m_plugin_registry->reloadEnabledPlugins();
}

auto AppController::accounts() -> int {
    return m_open_accounts.size();
}

auto AppController::isAccountOpen(const QString &name) const -> bool {
    return m_open_accounts.contains(name);
}

auto AppController::onRegtestDefaultsReady(const QString &blindbit, const QString &p2p,
                                           const QString &electrum) -> void {
    m_regtest_defaults = RegtestDefaultsInfo{.blindbit_url = blindbit,
                                             .p2p_node = p2p,
                                             .electrum_url = electrum};
}

auto AppController::regtestDefaults() const -> std::optional<RegtestDefaultsInfo> {
    return m_regtest_defaults;
}

auto AppController::applyPersistedThemeAtStartup() -> void {
    QString activeTheme = QString::fromStdString(std::string(app_active_theme().c_str())).trimmed();
    if (activeTheme.isEmpty()) {
        return;
    }

    m_startup_active_theme = activeTheme;

    auto *theme = Theme::get();
    if (theme == nullptr) {
        return;
    }

    if (activeTheme.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0) {
        theme->setMode(ThemeMode::Dark);
        m_startup_custom_theme_applied = true;
        return;
    }

    if (activeTheme.compare(QStringLiteral("light"), Qt::CaseInsensitive) == 0) {
        theme->setMode(ThemeMode::Light);
        m_startup_custom_theme_applied = true;
        return;
    }

    if (!m_startup_custom_theme_applied) {
        requestPersistedThemePalette();
    }
}

auto AppController::requestPersistedThemePalette() -> void {
    m_startup_theme_providers.clear();
    m_startup_pending_theme_list.clear();
    m_startup_pending_palette.clear();

    if (m_host == nullptr) {
        return;
    }

    const auto instances = m_host->instances();
    for (auto *instance : instances) {
        if (instance == nullptr) {
            continue;
        }
        auto *provider = instance->theme();
        if (provider == nullptr || !provider->implemented()) {
            continue;
        }
        if (!m_startup_theme_providers.contains(provider)) {
            m_startup_theme_providers.append(provider);
            connect(provider, qOverload<ReqId, QStringList>(&IThemeProvider::themes), this,
                    &AppController::onStartupThemeNames, qontrol::UNIQUE);
            connect(provider, qOverload<ReqId, theme::Palette>(&IThemeProvider::palette), this,
                    &AppController::onStartupThemePalette, qontrol::UNIQUE);
        }
        ReqId reqId = provider->themes();
        if (reqId != 0) {
            m_startup_pending_theme_list.insert(reqId, provider);
        }
    }
}

auto AppController::onStartupThemeNames(ReqId req_id, const QStringList &themes) -> void {
    if (!m_startup_pending_theme_list.contains(req_id) || m_startup_custom_theme_applied
        || m_startup_active_theme.isEmpty()) {
        return;
    }

    auto *provider = m_startup_pending_theme_list.take(req_id);
    if (provider == nullptr) {
        return;
    }

    bool hasTheme = std::ranges::any_of(themes, [this](const QString &name) {
        return name.trimmed().compare(m_startup_active_theme, Qt::CaseInsensitive) == 0;
    });
    if (!hasTheme) {
        return;
    }

    ReqId paletteReqId = provider->palette(m_startup_active_theme);
    if (paletteReqId != 0) {
        m_startup_pending_palette.insert(paletteReqId, provider);
    }
}

auto AppController::onStartupThemePalette(ReqId req_id, theme::Palette palette) -> void {
    if (!m_startup_pending_palette.contains(req_id) || m_startup_custom_theme_applied) {
        return;
    }
    m_startup_pending_palette.remove(req_id);

    auto *theme = Theme::get();
    if (theme == nullptr) {
        return;
    }

    theme->setPalette(palette);
    theme->apply();
    m_startup_custom_theme_applied = true;
}

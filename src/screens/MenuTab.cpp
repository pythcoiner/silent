#include "MenuTab.h"
#include "AppController.h"
#include "i18n/Tr.h"
#include "host/Host.h"
#include "host/PluginRegistry.h"
#include "interfaces/module.h"
#include "screens/modals/ConfirmDelete.h"
#include "screens/modals/CustomLaunchers.h"
#include "theme/Button.h"
#include "theme/Icon.h"
#include "theme/Label.h"
#include "theme/Palette.h"
#include "theme/Theme.h"
#include <Qontrol>
#include <common.h>

using theme::Button;
using theme::ButtonRole;
using theme::Label;
using theme::LabelRole;

MenuTab::MenuTab(QWidget *parent) : QWidget(parent) {
    init();
    doConnect();
    view();
}

void MenuTab::init() {
    m_custom_btn = new Button(TR("menu-custom"), ButtonRole::TabOpen);
    // Match the create-account button width so the launcher buttons line up.
    m_custom_btn->setFixedWidth(Theme::get()->buttonPalette().tabCreate.width);
    m_custom_btn->setVisible(false);

    m_accounts_column = new qontrol::Column;

    m_host = Host::get();
    if (m_host != nullptr) {
        m_host_events = m_host->events();
    }
}

void MenuTab::doConnect() {
    auto *controller = AppController::get();
    connect(m_custom_btn, &QPushButton::clicked, this, &MenuTab::onCustomClicked, qontrol::UNIQUE);
    connect(controller, &AppController::accountList, this, &MenuTab::onAccountList, qontrol::UNIQUE);
    if (controller != nullptr && controller->pluginRegistry() != nullptr) {
        connect(controller->pluginRegistry(), &PluginRegistry::enabledPluginsChanged, this,
                &MenuTab::onPluginRegistryChanged, qontrol::UNIQUE);
    }
    if (m_host_events != nullptr) {
        connect(m_host_events, &HostEvents::instanceRegistered, this, &MenuTab::onHostInstanceChanged,
                qontrol::UNIQUE);
        connect(m_host_events, &HostEvents::instanceRemoved, this, &MenuTab::onHostInstanceChanged,
                qontrol::UNIQUE);
        connect(m_host_events, &HostEvents::settingSectionsChanged, this,
                &MenuTab::onHostSettingsChanged, qontrol::UNIQUE);
    }
    onRefreshLauncherItems();
}

void MenuTab::onItemClicked() {
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn == nullptr) {
        return;
    }
    auto id = btn->property("instanceId").toString();
    auto modulePtr = btn->property("modulePtr").toULongLong();
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    auto *module = reinterpret_cast<IModule *>(modulePtr);
    if (module == nullptr || id.isEmpty()) {
        return;
    }
    // Route through the registry so the started instance is attributed to its
    // owning plugin; a direct module->startInstance() leaves it unowned and
    // blocks the plugin from being disabled later.
    auto *controller = AppController::get();
    auto *registry = controller != nullptr ? controller->pluginRegistry() : nullptr;
    if (registry != nullptr) {
        registry->startLauncherInstance(module, id);
    } else {
        module->startInstance(id);
    }
}

void MenuTab::onTrashClicked() {
    auto *btn = qobject_cast<QPushButton *>(sender());
    if (btn == nullptr) {
        return;
    }
    auto name = btn->property("accountName").toString();
    if (!btn->property("external").toBool()) {
        AppController::get()->deleteAccount(name);
        return;
    }

    auto modulePtr = btn->property("modulePtr").toULongLong();
    // NOLINTNEXTLINE(performance-no-int-to-ptr)
    auto *module = reinterpret_cast<IModule *>(modulePtr);
    if (module == nullptr || name.isEmpty()) {
        return;
    }
    m_pending_delete_module = module;
    m_pending_delete_id = name;
    auto *modal = new modal::ConfirmDelete(name);
    connect(modal, &modal::ConfirmDelete::confirmed, this, &MenuTab::onExternalDeleteConfirmed,
            qontrol::UNIQUE);
    AppController::execModal(modal);
}

void MenuTab::onExternalDeleteConfirmed(const QString &name) {
    auto *controller = AppController::get();
    auto *registry = controller != nullptr ? controller->pluginRegistry() : nullptr;
    if (registry != nullptr && m_pending_delete_module != nullptr && name == m_pending_delete_id) {
        registry->deleteInstance(m_pending_delete_module, name);
    }
    m_pending_delete_module = nullptr;
    m_pending_delete_id.clear();
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void MenuTab::onCustomClicked() {
    AppController::execModal(new modal::CustomLaunchers());
}

void MenuTab::view() {
    auto *title = new Label(TR("main-window-title"), LabelRole::Title);
    title->setAlignment(Qt::AlignCenter);

    auto *titleRow = (new qontrol::Row)->pushSpacer()->push(title)->pushSpacer();
    auto *btnRow = (new qontrol::Row)->pushSpacer()->push(m_custom_btn)->pushSpacer();

    auto *col = (new qontrol::Column)
                    ->pushSpacer()
                    ->push(titleRow)
                    ->pushSpacer(resolve(Spacing::XL))
                    ->push(m_accounts_column)
                    ->pushSpacer(resolve(Spacing::M))
                    ->push(btnRow)
                    ->pushSpacer();

    setLayout(col->layout());
}

void MenuTab::clearRows() {
    m_accounts_column->clear();
    m_rows.clear();
}

void MenuTab::onAccountList(const QList<QString> &accounts) {
    m_sp_accounts.clear();
    for (const auto &account : accounts) {
        m_sp_accounts.insert(account);
    }
    onRefreshLauncherItems();
}

void MenuTab::ensureModuleConnections() {
    auto *controller = AppController::get();
    if (controller == nullptr || controller->pluginRegistry() == nullptr) {
        return;
    }

    auto modules = controller->pluginRegistry()->enabledModulesForLauncher();
    if (modules == m_modules) {
        return;
    }

    for (auto *module : m_modules) {
        if (module == nullptr) {
            continue;
        }
        disconnect(module, &IModule::instances, this, &MenuTab::onModuleInstances);
    }

    m_modules = modules;
    m_pending_list.clear();
    m_sync_list_responses.clear();
    m_items_by_module.clear();
    for (auto *module : m_modules) {
        if (module == nullptr) {
            continue;
        }
        connect(module, &IModule::instances, this, &MenuTab::onModuleInstances, qontrol::UNIQUE);
    }
}

void MenuTab::onRefreshLauncherItems() {
    ensureModuleConnections();
    for (auto *module : m_modules) {
        if (module == nullptr) {
            continue;
        }
        ReqId reqId = module->list();
        if (reqId == 0) {
            continue;
        }
        m_pending_list.insert(module, reqId);
        if (m_sync_list_responses.contains(module)
            && m_sync_list_responses.value(module).first == reqId) {
            auto response = m_sync_list_responses.take(module);
            onModuleInstances(response.first, response.second);
        }
    }
}

void MenuTab::onModuleInstances(ReqId req_id, QList<QPair<QString, QString>> instances) {
    auto *module = qobject_cast<IModule *>(sender());
    if (module == nullptr) {
        return;
    }
    if (!m_pending_list.contains(module) || m_pending_list.value(module) != req_id) {
        if (req_id != 0) {
            m_sync_list_responses.insert(module, {req_id, std::move(instances)});
        }
        return;
    }

    m_pending_list.remove(module);
    m_items_by_module.insert(module, instances);
    rebuildRows();
}

void MenuTab::onHostInstanceChanged(const QString &id) {
    Q_UNUSED(id);
    AppController::get()->listAccounts();
    onRefreshLauncherItems();
}

void MenuTab::onHostSettingsChanged() {
    onRefreshLauncherItems();
}

void MenuTab::onPluginRegistryChanged() {
    onRefreshLauncherItems();
}

void MenuTab::rebuildRows() {
    clearRows();
    m_entries.clear();

    auto *controller = AppController::get();
    auto *registry = controller != nullptr ? controller->pluginRegistry() : nullptr;

    QList<LauncherEntry> createEntries;
    QList<LauncherEntry> openEntries;

    for (auto it = m_items_by_module.constBegin(); it != m_items_by_module.constEnd(); ++it) {
        auto *module = it.key();
        for (const auto &item : it.value()) {
            QString id = item.first.trimmed();
            bool isCreator = PluginRegistry::isCreatorInstanceId(id);
            // External-plugin creators live in the Custom modal, not the launcher list.
            if (isCreator && registry != nullptr && !registry->isBuiltinModule(module)) {
                continue;
            }

            bool isBuiltin = registry != nullptr && registry->isBuiltinModule(module);

            LauncherEntry entry;
            entry.module = module;
            entry.id = id;
            entry.name = item.second.trimmed();
            if (entry.name.isEmpty()) {
                entry.name = entry.id;
            }
            entry.external = !isBuiltin;
            // Built-in (SP) accounts keep their existing delete path; every external
            // non-creator entry is a persisted instance the owning module can delete.
            entry.canDelete = isBuiltin ? m_sp_accounts.contains(entry.id) : !isCreator;

            if (isCreator) {
                createEntries.append(entry);
            } else {
                openEntries.append(entry);
            }
        }
    }

    m_entries = openEntries + createEntries;
    for (const auto &entry : m_entries) {
        auto role = PluginRegistry::isCreatorInstanceId(entry.id) ? ButtonRole::TabCreate
                                                                  : ButtonRole::TabOpen;
        auto *btn = new Button(entry.name, role);
        btn->setProperty("instanceId", entry.id);
        btn->setProperty("modulePtr", QVariant::fromValue(reinterpret_cast<qulonglong>(entry.module)));
        connect(btn, &QPushButton::clicked, this, &MenuTab::onItemClicked, qontrol::UNIQUE);

        auto *row = (new qontrol::Row)->pushSpacer()->push(btn);
        if (entry.canDelete) {
            auto *trashBtn = new Button(ButtonRole::Icon);
            trashBtn->setIcon(icon::trash());
            trashBtn->setToolTip(TR("menu-delete-wallet-tooltip"));
            trashBtn->setProperty("accountName", entry.id);
            trashBtn->setProperty("external", entry.external);
            trashBtn->setProperty("modulePtr",
                                  QVariant::fromValue(reinterpret_cast<qulonglong>(entry.module)));
            connect(trashBtn, &QPushButton::clicked, this, &MenuTab::onTrashClicked, qontrol::UNIQUE);
            row->pushSpacer(resolve(Spacing::XS))->push(trashBtn);
        }
        row->pushSpacer();

        if (AppController::get()->isAccountOpen(entry.id)) {
            row->setEnabled(false);
        }

        if (!m_rows.isEmpty()) {
            m_accounts_column->pushSpacer(resolve(Spacing::S));
        }

        m_accounts_column->push(row);
        m_rows.append(row);
    }

    if (m_custom_btn != nullptr) {
        m_custom_btn->setVisible(registry != nullptr
                                 && !registry->externalCreatorLaunchers().isEmpty());
    }
}

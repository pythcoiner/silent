#include "Host.h"

#include "AppController.h"
#include "i18n/I18nManager.h"
#include "MainWindow.h"
#include "interfaces/instance.h"

#include <QApplication>
#include <QDialog>
#include <QList>
#include <QThread>
#include <Qontrol>
#include <common.h>

namespace {
Host *gHostSingleton = nullptr;

void assertGuiThread() {
    if (qApp == nullptr) {
        return;
    }
    Q_ASSERT(QThread::currentThread() == qApp->thread());
}

class HostedWizardModal final : public qontrol::Modal {
public:
    explicit HostedWizardModal(QWidget *content) {
        setMainWidget(content);
    }
};

class HostI18nImpl final : public HostI18n {
public:
    auto registerPluginTranslations(const QString &plugin_id, const QString &dir) -> bool override {
        return i18n::I18nManager::get()->registerPluginTranslations(plugin_id, dir);
    }

    void unregisterPluginTranslations(const QString &plugin_id) override {
        i18n::I18nManager::get()->unregisterPluginTranslations(plugin_id);
    }
};
} // namespace

HostImpl::HostImpl() : QObject(nullptr), m_i18n(new HostI18nImpl()) {}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
MainWindow *HostImpl::mainWindow() const {
    return dynamic_cast<MainWindow *>(AppController::window());
}

// NOLINTNEXTLINE(misc-no-recursion)
void HostImpl::ensureMainWindowHooked() {
    if (m_main_window_hooked) {
        return;
    }
    auto *window = mainWindow();
    if (window == nullptr) {
        return;
    }
    connect(window, &MainWindow::hostedTabCloseRequested, this,
            &HostImpl::onHostedTabCloseRequested, qontrol::UNIQUE);
    m_main_window_hooked = true;
    runDevTabSmokeIfRequested();
}

void HostImpl::registerInstance(const QString &id, IInstance *instance) {
    assertGuiThread();
    if (instance == nullptr) {
        return;
    }
    m_instances.insert(id, instance);
    emit m_events.instanceRegistered(id);
}

QList<IInstance *> HostImpl::instances() const {
    assertGuiThread();
    return m_instances.values();
}

// NOLINTNEXTLINE(misc-no-recursion)
Host::TabId HostImpl::openTab(QWidget *content, IInstance *owner) {
    assertGuiThread();
    if (content == nullptr || owner == nullptr) {
        return 0;
    }
    ensureMainWindowHooked();
    auto *window = mainWindow();
    if (window == nullptr) {
        return 0;
    }

    if (m_tab_by_owner.contains(owner)) {
        // This owner already has a hosted tab; host does not take ownership of replacement content.
        // Dispose the extra widget to avoid ambiguous ownership/lifecycle leaks.
        content->deleteLater();
        return m_tab_by_owner.value(owner);
    }

    const TabId tab_id = ++m_next_tab_id;
    window->addTab(content, QString());
    m_tabs.insert(tab_id, TabEntry{.id = tab_id, .content = content, .owner = owner});
    m_tab_by_owner.insert(owner, tab_id);
    return tab_id;
}

void HostImpl::setTabTitle(TabId tab, const QString &title) {
    assertGuiThread();
    if (!m_tabs.contains(tab)) {
        return;
    }
    auto *window = mainWindow();
    if (window == nullptr) {
        return;
    }
    auto &entry = m_tabs[tab];
    window->setTabTitle(entry.content, title);
}

void HostImpl::closeTab(TabId tab) {
    assertGuiThread();
    if (!m_tabs.contains(tab)) {
        return;
    }
    auto entry = m_tabs.take(tab);
    m_tab_by_owner.remove(entry.owner);
    auto *window = mainWindow();
    if (window != nullptr) {
        window->removeTab(entry.content);
    } else if (entry.content != nullptr) {
        entry.content->deleteLater();
    }
}

Host::SectionId HostImpl::addSettingSection(
    QWidget *content, IInstance *owner, const QString &title) {
    assertGuiThread();
    if (content == nullptr || owner == nullptr) {
        return 0;
    }

    const SectionId section_id = ++m_next_section_id;
    m_sections.insert(section_id,
                      SectionEntry{
                          .id = section_id,
                          .content = content,
                          .owner = owner,
                          .title = title.trimmed(),
                      });
    emit m_events.settingSectionsChanged();
    return section_id;
}

void HostImpl::delSettingSection(SectionId section) {
    assertGuiThread();
    if (!m_sections.contains(section)) {
        return;
    }
    auto entry = m_sections.take(section);
    if (entry.content != nullptr) {
        entry.content->deleteLater();
    }
    emit m_events.settingSectionsChanged();
}

Host::WizardId HostImpl::openWizard(QWidget *content, IInstance *owner) {
    assertGuiThread();
    if (content == nullptr || owner == nullptr) {
        return 0;
    }

    const WizardId wizard_id = ++m_next_wizard_id;
    auto *modal = new HostedWizardModal(content);
    m_wizards.insert(wizard_id, WizardEntry{.id = wizard_id,
                                           .content = content,
                                           .modal = modal,
                                           .owner = owner});
    m_wizard_by_modal.insert(modal, wizard_id);
    connect(modal, &QDialog::finished, this, &HostImpl::onWizardModalFinished, qontrol::UNIQUE);
    AppController::execModal(modal);
    return wizard_id;
}

void HostImpl::closeWizard(WizardId wizard) {
    assertGuiThread();
    if (!m_wizards.contains(wizard)) {
        return;
    }

    auto entry = m_wizards.take(wizard);
    if (entry.modal != nullptr) {
        m_wizard_by_modal.remove(entry.modal);
        auto *dialog = qobject_cast<QDialog *>(entry.modal.data());
        if (dialog != nullptr && dialog->isVisible()) {
            dialog->reject();
        }
        entry.modal->deleteLater();
    }
    if (entry.content != nullptr) {
        entry.content->deleteLater();
    }
}

HostI18n *HostImpl::i18n() {
    return m_i18n;
}

HostEvents *HostImpl::events() {
    return &m_events;
}

void HostImpl::onInstanceStopped(const QString &id) {
    assertGuiThread();
    auto *instance = m_instances.value(id, nullptr);
    onInstanceStopped(instance);
}

void HostImpl::onInstanceStopped(IInstance *instance) {
    assertGuiThread();
    if (instance == nullptr) {
        return;
    }
    closeOwnedUi(instance);
}

void HostImpl::removeInstance(const QString &id) {
    assertGuiThread();
    if (!m_instances.contains(id)) {
        return;
    }
    auto *instance = m_instances.value(id);
    closeOwnedUi(instance);
    m_instances.remove(id);
    emit m_events.instanceRemoved(id);
}

// NOLINTNEXTLINE(misc-no-recursion)
void HostImpl::runDevTabSmokeIfRequested() {
    static bool smokeAttempted = false;
    if (smokeAttempted) {
        return;
    }
    smokeAttempted = true;

    if (qEnvironmentVariableIntValue("SILENT_HOST_TAB_SMOKE") != 1) {
        return;
    }

    auto *window = mainWindow();
    if (window == nullptr) {
        qWarning() << "Host tab smoke skipped: MainWindow unavailable";
        return;
    }
    if (m_instances.isEmpty()) {
        qWarning() << "Host tab smoke skipped: no registered instance owner available";
        return;
    }

    auto *dummyTab = new QWidget();
    auto *owner = m_instances.constBegin().value();
    const TabId smoke_tab = openTab(dummyTab, owner);
    if (smoke_tab == 0) {
        qWarning() << "Host tab smoke failed: openTab returned 0";
        dummyTab->deleteLater();
        return;
    }
    setTabTitle(smoke_tab, QStringLiteral("host-tab-smoke"));
    closeTab(smoke_tab);
    qInfo() << "Host tab smoke completed";
}

QList<Host::SettingSectionView> HostImpl::settingSections() const {
    assertGuiThread();
    QList<Host::SettingSectionView> sections;
    sections.reserve(m_sections.size());
    for (const auto &entry : m_sections) {
        if (entry.content != nullptr) {
            sections.append(Host::SettingSectionView{
                .id = entry.id,
                .content = entry.content.data(),
                .title = entry.title,
            });
        }
    }
    return sections;
}

void HostImpl::onHostedTabCloseRequested(QWidget *content) {
    assertGuiThread();
    IInstance *ownerToStop = nullptr;
    for (auto it = m_tabs.constBegin(); it != m_tabs.constEnd(); ++it) {
        if (it.value().content == content) {
            ownerToStop = it.value().owner;
            break;
        }
    }

    if (ownerToStop != nullptr) {
        // Closing the tab tears the instance down completely: stop it, then remove
        // it from the host so instanceRemoved fires (clearing open-account state and
        // letting the owning module recycle the id). Without the removal the instance
        // lingers registered and cannot be reopened from the launcher.
        QString id = ownerToStop->id();
        ownerToStop->stop();
        removeInstance(id);
    }
}

void HostImpl::onWizardModalFinished(int result) {
    Q_UNUSED(result);
    assertGuiThread();
    auto *dialog = qobject_cast<QDialog *>(sender());
    if (dialog == nullptr || !m_wizard_by_modal.contains(dialog)) {
        return;
    }
    const WizardId wizard_id = m_wizard_by_modal.value(dialog);
    m_wizard_by_modal.remove(dialog);
    if (!m_wizards.contains(wizard_id)) {
        return;
    }
    auto entry = m_wizards.take(wizard_id);
    if (entry.content != nullptr) {
        entry.content->deleteLater();
    }
    dialog->deleteLater();
}

void HostImpl::closeOwnedUi(IInstance *owner) {
    if (owner == nullptr) {
        return;
    }

    if (m_tab_by_owner.contains(owner)) {
        closeTab(m_tab_by_owner.value(owner));
    }

    QList<SectionId> sectionIds;
    for (auto it = m_sections.constBegin(); it != m_sections.constEnd(); ++it) {
        if (it.value().owner == owner) {
            sectionIds.append(it.key());
        }
    }
    for (SectionId id : sectionIds) {
        delSettingSection(id);
    }

    QList<WizardId> wizardIds;
    for (auto it = m_wizards.constBegin(); it != m_wizards.constEnd(); ++it) {
        if (it.value().owner == owner) {
            wizardIds.append(it.key());
        }
    }
    for (WizardId id : wizardIds) {
        closeWizard(id);
    }
}

Host *Host::get() {
    if (gHostSingleton == nullptr) {
        gHostSingleton = new HostImpl();
    }
    return gHostSingleton;
}

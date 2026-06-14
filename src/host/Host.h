#pragma once

#include "interfaces/host.h"
#include <QObject>
#include <QHash>
#include <QPointer>

class MainWindow;

class HostImpl final : public QObject, public Host {
    Q_OBJECT

public:
    HostImpl();

    void registerInstance(const QString &id, IInstance *instance) override;
    [[nodiscard]] QList<IInstance *> instances() const override;

    TabId openTab(QWidget *content, IInstance *owner) override;
    void setTabTitle(TabId tab, const QString &title) override;
    void closeTab(TabId tab) override;

    SectionId addSettingSection(
        QWidget *content, IInstance *owner, const QString &title = QString()) override;
    void delSettingSection(SectionId section) override;

    WizardId openWizard(QWidget *content, IInstance *owner) override;
    void closeWizard(WizardId wizard) override;

    HostI18n *i18n() override;
    HostEvents *events() override;

    void onInstanceStopped(const QString &id);
    void onInstanceStopped(IInstance *instance);
    void removeInstance(const QString &id);
    [[nodiscard]] QList<SettingSectionView> settingSections() const override;

private slots:
    void onHostedTabCloseRequested(QWidget *content);
    void onWizardModalFinished(int result);

private:
    struct TabEntry {
        TabId id = 0;
        QPointer<QWidget> content;
        IInstance *owner = nullptr;
    };
    struct SectionEntry {
        SectionId id = 0;
        QPointer<QWidget> content;
        IInstance *owner = nullptr;
        QString title;
    };
    struct WizardEntry {
        WizardId id = 0;
        QPointer<QWidget> content;
        QPointer<QWidget> modal;
        IInstance *owner = nullptr;
    };

    [[nodiscard]] MainWindow *mainWindow() const;
    void ensureMainWindowHooked();
    void runDevTabSmokeIfRequested();
    void closeOwnedUi(IInstance *owner);

    QHash<QString, IInstance *> m_instances;
    QHash<TabId, TabEntry> m_tabs;
    QHash<IInstance *, TabId> m_tab_by_owner;
    QHash<SectionId, SectionEntry> m_sections;
    QHash<WizardId, WizardEntry> m_wizards;
    QHash<QWidget *, WizardId> m_wizard_by_modal;
    TabId m_next_tab_id = 0;
    SectionId m_next_section_id = 0;
    WizardId m_next_wizard_id = 0;
    bool m_main_window_hooked = false;
    HostEvents m_events;
    HostI18n *m_i18n = nullptr;
};

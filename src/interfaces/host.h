#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QtGlobal>

class QWidget;
class IInstance;

class HostI18n {
public:
    virtual ~HostI18n() = default;
    virtual bool registerPluginTranslations(const QString &plugin_id, const QString &dir) = 0;
    virtual void unregisterPluginTranslations(const QString &plugin_id) = 0;
};

class HostEvents : public QObject {
    Q_OBJECT

public:
    using QObject::QObject;

signals:
    void instanceRegistered(QString id);
    void instanceRemoved(QString id);
    void settingSectionsChanged();
};

class Host {
public:
    using TabId = quint64;
    using SectionId = quint64;
    using WizardId = quint64;
    struct SettingSectionView {
        SectionId id = 0;
        QWidget *content = nullptr;
        QString title;
    };

    virtual ~Host() = default;

    static Host *get();

    virtual void registerInstance(const QString &id, IInstance *instance) = 0;
    [[nodiscard]] virtual QList<IInstance*> instances() const = 0;

    virtual TabId openTab(QWidget *content, IInstance *owner) = 0;
    virtual void setTabTitle(TabId tab, const QString &title) = 0;
    virtual void closeTab(TabId tab) = 0;

    virtual SectionId addSettingSection(
        QWidget *content, IInstance *owner, const QString &title = QString()) = 0;
    virtual void delSettingSection(SectionId section) = 0;
    [[nodiscard]] virtual QList<SettingSectionView> settingSections() const = 0;

    virtual WizardId openWizard(QWidget *content, IInstance *owner) = 0;
    virtual void closeWizard(WizardId wizard) = 0;

    virtual HostI18n *i18n() = 0;
    virtual HostEvents *events() = 0;
};

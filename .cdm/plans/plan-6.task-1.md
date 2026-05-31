Host singleton interface + HostEvents

## Objective
Define the `Host` singleton interface (the one handle modules use) and `HostEvents`.

## Files to Read
- `docs/PLUGINS.md` - "The Host" section (exact method set + HostEvents signals).
- `src/plugin/sdk/interfaces/instance.h`.

## Implementation Steps
1. **`src/plugin/sdk/host.h`**
   - `class Host` with `static Host* get();`
   - `void registerInstance(const QString &id, IInstance *instance);`
     `QList<IInstance*> instances() const;`
   - id typedefs: `using TabId = quint64; using SectionId = quint64; using WizardId =
     quint64;`
   - push-UI: `TabId openTab(QWidget*, IInstance*); void setTabTitle(TabId, const
     QString&); void closeTab(TabId); SectionId addSettingSection(QWidget*,
     IInstance*); void delSettingSection(SectionId); WizardId openWizard(QWidget*,
     IInstance*); void closeWizard(WizardId);`
   - `HostI18n* i18n(); HostEvents* events();` (forward-declare `HostI18n`; its real
     shape lands with phase 22 / can be a forward decl here).
   - `class HostEvents : public QObject { Q_OBJECT signals: void
     instanceRegistered(QString); void instanceRemoved(QString); };`
   - Keep `Host` as an abstract interface (pure virtuals) OR a declared singleton;
     match docs/PLUGINS.md (interface with `get()`); the concrete impl lands phase 10.

## Files to Create
- `src/plugin/sdk/host.h`.

## Verification
- [ ] SDK smoke target builds including `host.h`.

## Reviewer Criteria
- Must: method set + id typedefs match docs/PLUGINS.md.
- Must: no `IHost*` parameters anywhere (singleton via `Host::get()`).
- Must: `HostEvents` uses named-slot-friendly signals (no lambdas implied).

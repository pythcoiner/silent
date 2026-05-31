IPlugin + IModule interfaces

## Objective
Define the two top-level interfaces: `IPlugin` (library entry, exposes modules) and
`IModule` (owns instances; async `list()`/`autoStart()`/`startInstance()`).

## Files to Read
- `docs/PLUGINS.md` - the `IPlugin` and `IModule` sketches (Capability Interfaces).
- `src/plugin/sdk/types.h` - `ReqId`.

## Implementation Steps
1. **Create `src/plugin/sdk/interfaces/module.h`**
   - `IPlugin`: virtual dtor; `virtual QMap<QString,QString> meta() const = 0;`
     `virtual QList<IModule*> modules() const = 0;` (forward-declare `IModule`).
   - `IModule : public QObject` with `Q_OBJECT`: `meta() const`, `ReqId list()`,
     `ReqId autoStart()`, `void startInstance(const QString &id)`; signals
     `instances(ReqId, QList<QPair<QString,QString>>)`, `autoStart(ReqId, QStringList)`,
     `error(std::optional<ReqId>, QString)`. Include `<optional>`.
2. **Declare interface IIDs**
   - `#define SILENT_IPlugin_IID "dev.silent.IPlugin/1.0"` and `Q_DECLARE_INTERFACE(
     IPlugin, SILENT_IPlugin_IID)` (outside any namespace, after the class).
   - `IModule` is a QObject, no `Q_DECLARE_INTERFACE` needed, but give it an IID
     constant if useful for metadata.
3. **Smoke test**: include the header; ensure it MOC-compiles (the smoke target must
   run AUTOMOC over SDK headers it includes, or add a tiny `.cpp` that includes it).

## Files to Create
- `src/plugin/sdk/interfaces/module.h`.

## Files to Modify
- `src/plugin/sdk/smoke/main.cpp` - include the header so MOC processes it.
- `src/plugin/sdk/CMakeLists.txt` - ensure AUTOMOC on for the smoke target.

## Verification
- [ ] SDK smoke target builds with AUTOMOC clean.

## Reviewer Criteria
- Must: signatures match docs/PLUGINS.md exactly (names, `ReqId`, `std::optional`).
- Must: `Q_DECLARE_INTERFACE` IID present for `IPlugin`.
- Must: no `rust::*` types; Qt/std only.

PluginRegistry compiled-in path (Q_IMPORT_PLUGIN)

## Objective
Add `PluginRegistry` holding `IPlugin*` with per-plugin enabled state, supporting the
compiled-in (built-in module) path only. Runtime `QPluginLoader` is phase 23.

## Files to Read
- `docs/PLUGINS.md` - "Registry" / "Lifecycle" sections.
- `src/plugin/host/Host.h`.
- `silent/src/lib.rs` - `app_enabled_plugins` etc. from phase 9.

## Implementation Steps
1. **`src/plugin/host/PluginRegistry.{h,cpp}`**
   - Hold `QList<IPlugin*>` (registered) + enabled-state map (seed from
     `app_enabled_plugins()`; built-in modules always enabled).
   - `registerBuiltin(IPlugin*)`: add a compiled-in plugin; iterate `modules()`; for
     each module call `requestAutoStart()` and `list()` (results arrive on the
     module's signals; connect named slots that start auto instances and feed the
     launcher).
   - Provide `plugins()` accessor + `enabledModulesForLauncher()` style queries used
     by MenuTab/AppSettings later.
2. **Integration**: the registry is owned by `AppController` (or the Host); built-in
   registration happens at startup. SP registration itself lands phase 14; here just
   provide the registry + a no-op/sample built-in to exercise the path.
3. Use `Q_IMPORT_PLUGIN(...)` plumbing readiness (the macro call lands with SP).

## Files to Create
- `src/plugin/host/PluginRegistry.h`, `src/plugin/host/PluginRegistry.cpp`.

## Verification
- [ ] Build links; registry instantiable; connecting a mock built-in plugin lists its
      modules without crashing.

## Reviewer Criteria
- Must: built-in plugins always enabled; enabled-set seeded from app state.
- Must: module result signals consumed via named slots (async list/autoStart).
- May skip: runtime loading (explicitly phase 23).

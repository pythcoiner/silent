SpPlugin + SpModule (list/startInstance)

## Objective
Add the SP module's top-level classes: `SpPlugin : IPlugin` (one module) and
`SpModule : IModule` exposing the existing accounts as startable instances.

## Files to Read
- `docs/PLUGINS.md` - IPlugin/IModule contract.
- `silent/src/config.rs` / `lib.rs` - `list_configs()` (account enumeration) +
  `get_plugin_id`.
- `src/plugin/host/PluginRegistry.h` - `registerBuiltin`.

## Implementation Steps
1. **`src/module/sp/SpPlugin.{h,cpp}`** - `SpPlugin : public IPlugin` (+
   `Q_PLUGIN_METADATA` is for runtime `.so`; for the built-in module use plain class +
   `Q_IMPORT_PLUGIN`-style registration in the registry). `meta()` returns
   id="sp"/name/version; `modules()` returns the single `SpModule*`.
2. **`src/module/sp/SpModule.{h,cpp}`** - `SpModule : public IModule`:
   - `meta()`; `ReqId list()`: enumerate `list_configs()` filtered to `plugin_id=="sp"`,
     emit `instances(req, [(id,name)])` (name = account display name). Include a
     "create new account" entry per the launcher create flow.
   - `ReqId autoStart()`: emit empty list (SP accounts are user-launched).
   - `void startInstance(const QString &id)`: construct an `SpInstance` (phase 15) and
     `Host::get()->registerInstance(id, instance)`.
3. **Register**: `Q_IMPORT_PLUGIN(SpPlugin)` + `registry.registerBuiltin(new SpPlugin)`
   at startup.

## Files to Create
- `src/module/sp/SpPlugin.{h,cpp}`, `src/module/sp/SpModule.{h,cpp}`.

## Verification
- [ ] Build links; at startup the registry lists SP's accounts (logged) without
      breaking the existing launcher.

## Reviewer Criteria
- Must: `list()`/`autoStart()` are async (emit on signals), not synchronous returns.
- Must: account enumeration filters by stored `plugin_id`.
- Must: SpInstance construction deferred to phase 15 (stub/forward ok).

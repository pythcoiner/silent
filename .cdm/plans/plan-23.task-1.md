PluginRegistry QPluginLoader scan + enable/disable/unload

## Objective
Add runtime plugin discovery: scan `~/.silent/plugins/`, read sidecar metadata
without loading, and on enable `dlopen` via `QPluginLoader`; on disable tear down +
unload. Hot, no restart.

## Files to Read
- `docs/PLUGINS.md` - "Distribution Unit", "Lifecycle: Discover/Enable/Disable",
  "Hot enable/disable".
- `src/plugin/host/PluginRegistry.{h,cpp}` - compiled-in path from phase 12.

## Implementation Steps
1. **Discover**: scan `~/.silent/plugins/*/`, read each `<id>.json` sidecar (id, name,
   version, capabilities) WITHOUT loading the library; list as discovered + disabled
   (unless in the persisted enabled set).
2. **Rescan**: a method the Settings screen can trigger; updates the discovered list
   only (no load/unload).
3. **Enable(id)**: `QPluginLoader::load()` the library; `qobject_cast<IPlugin*>` the
   root; on success register + bring up modules (autoStart + list + i18n register);
   on build-key/cast failure, log + skip (do not crash). Persist enabled.
4. **Disable(id)**: stop all instances of the plugin's modules (close tabs/sections,
   stop threads), unregister i18n, withdraw from discovery, `QPluginLoader::unload()`.
   Persist disabled.
5. Emit registry change signals so launcher + Settings refresh live.

## Files to Modify
- `src/plugin/host/PluginRegistry.{h,cpp}`.

## Verification
- [ ] A built example `.so` + `<id>.json` (phase 26-27) appears disabled; enabling
      loads it live; disabling tears it down; a Qt-mismatched plugin is rejected
      (logged, not crashed).

## Reviewer Criteria
- Must: metadata read strictly before `dlopen` (no code runs while disabled).
- Must: clean teardown before `unload()`; enabled-set persisted.
- Must: cast/build-key failures logged and skipped.

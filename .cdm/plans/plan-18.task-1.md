AppController registry-driven + main.cpp bootstrap

## Objective
Remove SP-specific knowledge from `AppController`; make it own the `PluginRegistry` +
`Host`, and route create/open/delete through modules. Fix bootstrap order in main.

## Files to Read
- `src/AppController.{h,cpp}` - current create/open/delete + `m_accounts` + signals.
- `src/main.cpp` - bootstrap sequence.
- `src/plugin/host/{Host,PluginRegistry}.h`.

## Implementation Steps
1. **AppController**
   - Own/init `PluginRegistry` and the `Host` singleton.
   - Drop `new AccountWidget`/`new_config(...SP...)` direct calls. `openAccount(id)` ->
     resolve the owning module by stored `plugin_id`, call `module->startInstance(id)`.
   - `deleteAccount` stays (confirm modal) but the live teardown goes through the
     instance's `stop()` + registry removal.
   - `listAccounts()` becomes "ask each enabled module for its `list()`" (aggregate;
     async via signals) feeding `accountList`.
2. **main.cpp** - construct Host + registry, `registerBuiltin(new SpPlugin)`, THEN
   `initState()`; keep i18n init before UI.

## Files to Modify
- `src/AppController.{h,cpp}`, `src/main.cpp`.

## Verification
- [ ] `just br`: launcher lists SP accounts; open/create/delete all work via the
      module/registry path.

## Reviewer Criteria
- Must: no `screen::`/SP includes remain in AppController.
- Must: bootstrap constructs registry/host before `initState`.
- Must: account routing uses stored `plugin_id`.

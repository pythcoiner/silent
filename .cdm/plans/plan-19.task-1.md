MenuTab launcher from each module's list()

## Objective
Make the launcher (`MenuTab`) render its buttons from every enabled module's `list()`
results (id, name), and a create entry, instead of SP-specific `list_configs()`.

## Files to Read
- `src/screens/MenuTab.{h,cpp}` - current account-list rendering + signals.
- `src/plugin/host/PluginRegistry.h` - enabled modules + their `list()` results.
- `docs/PLUGINS.md` - launcher behavior (one button per (id,name)).

## Implementation Steps
1. **MenuTab**
   - Subscribe (named slots) to each enabled module's `instances(ReqId, [(id,name)])`
     signal; aggregate into the buttons column. Each button click calls that module's
     `startInstance(id)`.
   - Add a Settings entry/button opening the global Settings screen (phase 20).
   - Keep delete (trash) routing through `AppController::deleteAccount` for SP-style
     entries; generic modules may not offer delete (guard).
2. Re-query on `HostEvents::instanceRegistered/Removed` and on registry
   enable/disable so the launcher stays live.

## Files to Modify
- `src/screens/MenuTab.{h,cpp}`.

## Verification
- [ ] `just br`: launcher shows SP accounts via module `list()`; clicking opens the
      tab; creating still works.

## Reviewer Criteria
- Must: no direct `list_configs()` call in MenuTab (goes through modules).
- Must: dynamic refresh on host events (no startup-cached set).
- Must: named slots; no lambdas; `&QCheckBox::toggled` if any toggle added.

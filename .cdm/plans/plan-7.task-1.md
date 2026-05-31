Plugin-side id-prefixing TR macro

## Objective
Provide an SDK-side `TR(...)` macro that injects the owning plugin's id prefix, so
plugin authors write unprefixed keys exactly like host code while keys never collide.

## Files to Read
- `docs/PLUGINS.md` - "Plugin Translations (i18n)" section.
- `src/i18n/Tr.h` - the host `TR`/`TRN` macros and the `consteval isValidId` (which
  rejects `.`); the host file must remain untouched.

## Implementation Steps
1. **`src/plugin/sdk/tr.h`**
   - Define `SILENT_PLUGIN_ID` expectation: the plugin's TU defines its id (e.g.
     `#define SILENT_PLUGIN_ID "example"` before including, or a constexpr).
   - Provide `TR(id)` that yields `qtTrId("<plugin-id>." id)` at the call site:
     concatenate the prefix with the literal key. Do NOT reuse host `isValidId`
     (it forbids `.`); the plugin-side validator must permit the `<id>.` separator,
     or skip validation and document the constraint.
   - Provide `TRN(id, n)` similarly if plural is needed.
2. Document (header comment) that the host loads the plugin's `.lang` with keys
   prefixed by the same `<plugin-id>.` (wired in phase 22), so call sites and catalog
   entries line up.

## Files to Create
- `src/plugin/sdk/tr.h`.

## Verification
- [ ] SDK smoke target builds including `tr.h`; a test `TR("hello")` expands and
      compiles (define `SILENT_PLUGIN_ID` in the smoke TU).

## Reviewer Criteria
- Must: host `src/i18n/Tr.h` is NOT modified.
- Must: prefix injection is compile-time (string-literal concat), not runtime.
- Must: no em dashes; matches docs/PLUGINS.md namespacing description.

Example plugin capability impl + .lang (end-to-end validation)

## Objective
Flesh out the example plugin so it exercises the whole system end to end: a real
capability, a pushed tab, plugin i18n, and reading a sibling SP account.

## Files to Read
- `docs/PLUGINS.md` - capability + IHost discovery + i18n.
- `contrib/example-plugin/*` - the skeleton from phase 26.
- `src/plugin/sdk/{interfaces/*, host.h, tr.h}`.

## Implementation Steps
1. **Instance + capability**: the example instance implements one capability (e.g.
   `IFeed` emitting a fake `conversionRate`, or `IAccount` returning canned coins) so
   another module can read it. On bring-up it pushes a tab via
   `Host::get()->openTab(widget, this)` showing live data.
2. **Inter-module read**: in its tab, enumerate `Host::get()->instances()`, find the
   SP instance, call `account()->coins()` and display the result (proves async
   ReqId+signal cross-module read).
3. **i18n**: ship `contrib/example-plugin/i18n/silent_en.lang` (+ one other locale);
   use the SDK plugin-side `TR(...)` in the tab; define `SILENT_PLUGIN_ID "example"`.
4. **Hot lifecycle**: confirm enable loads it live (tab + feed appear), disable tears
   it down, enabled state persists, locale switch updates its text live.

## Files to Create / Modify
- `contrib/example-plugin/` capability/instance sources, `i18n/silent_*.lang`, update
  its `CMakeLists.txt` to stage the `.lang` next to the library.

## Verification
- [ ] Enable in Settings -> tab appears, shows SP coins read via `account()->coins()`.
- [ ] Locale switch updates the plugin tab text live; disable removes everything.
- [ ] Disabled-by-default + persisted-enabled across restart all hold.

## Reviewer Criteria
- Must: cross-module read uses the async capability path (not internal hacks).
- Must: i18n keys are unprefixed in source/catalog (SDK TR adds the id prefix).
- Must: clean teardown on disable; no leaked tabs/threads.

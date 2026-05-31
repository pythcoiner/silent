Aggregated setting sections + active-theme picker

## Objective
Extend the global Settings screen to (a) render the host's registered setting sections
(pushed by instances via `Host::addSettingSection`) and (b) offer an active-theme
picker over registered `IThemeProvider`s.

## Files to Read
- `docs/PLUGINS.md` - settings sections + `IThemeProvider`.
- `src/screens/AppSettings.{h,cpp}` - from phase 20.
- `src/theme/Theme.h` - `Theme`/`Palette` apply path.
- `src/plugin/host/Host.h` - section registry + change signal.

## Implementation Steps
1. **Sections**: query the host for registered `(SectionId, widget, title)` entries;
   embed each widget in a collapsible/section in the Settings layout. Refresh on the
   host's section-changed signal (named slot).
2. **Theme picker**: collect themes from enabled `IThemeProvider`s (request
   `themes()`, gather via signal); a combo lists them; selecting one calls the
   provider's `palette(name)`, applies it through `Theme::get()` and repaints; persist
   via `app_set_active_theme`. Apply the persisted theme at startup.

## Files to Modify
- `src/screens/AppSettings.{h,cpp}`, `src/plugin/host/Host.{h,cpp}` (section accessor +
  change signal if not already present).

## Verification
- [ ] `just br`: a pushed setting section appears; selecting a theme repaints live and
      persists across restart.

## Reviewer Criteria
- Must: theme apply is live (repaint) and persisted.
- Must: sections refresh dynamically as instances add/remove them.
- Must: combo/selection uses named slots; no lambdas.

Global Settings screen + enable/disable toggles

## Objective
Create the app-global Settings screen with a plugin manager: list discovered plugins
with enable/disable toggles (built-in modules shown locked-on).

## Files to Read
- `docs/PLUGINS.md` - "Global Settings" + enable/disable semantics.
- `src/screens/MenuTab.cpp` - how a screen/tab is opened from the launcher.
- `src/theme/Toggle.h` - the toggle widget (`&QCheckBox::toggled`).
- `src/plugin/host/PluginRegistry.h`.

## Implementation Steps
1. **`src/screens/AppSettings.{h,cpp}`** - `screen::AppSettings : qontrol::Screen`
   (three-phase `init/doConnect/view`).
   - Plugin manager section: one row per discovered plugin (from registry metadata)
     with a `theme::Toggle`; built-in modules disabled-and-checked (locked on).
   - Toggling calls `PluginRegistry::setEnabled(id, on)` (live enable/disable) and
     persists via `app_set_plugin_enabled` (phase 9 FFI).
2. Open it from the launcher (MenuTab Settings entry) as a tab or modal.

## Files to Create
- `src/screens/AppSettings.{h,cpp}`.

## Files to Modify
- `src/screens/MenuTab.cpp` (open Settings), `CMakeLists.txt` (SOURCES).

## Verification
- [ ] `just br`: Settings lists plugins; toggling persists across restart; modules
      are locked on.

## Reviewer Criteria
- Must: toggles use `&QCheckBox::toggled`; named slots; no lambdas.
- Must: built-in modules cannot be disabled.
- Must: three-phase widget lifecycle; `screen::` namespace; `m_` members.

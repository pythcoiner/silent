Per-plugin CatalogTranslator with id-prefix + reload

## Objective
Let plugins ship `.lang` catalogs that load live with id-prefixed keys, reusing the
existing `CatalogTranslator`. Expose registration via `Host::i18n()`.

## Files to Read
- `docs/PLUGINS.md` - "Plugin Translations (i18n)".
- `src/i18n/I18nManager.{h,cpp}` - `CatalogTranslator`, `applyLocale`,
  `languageChanged`, `installTranslator` flow.
- `src/plugin/sdk/tr.h` - the plugin-side prefixing `TR` (phase 7).

## Implementation Steps
1. **`I18nManager`**
   - Add `registerPluginTranslations(const QString &pluginId, const QString &dir)`:
     create a `CatalogTranslator` that loads `dir/silent_<locale>.lang` and **prefixes
     each key with `<pluginId>.`** at load; `installTranslator`. Track per plugin so it
     can be removed on disable.
   - On `applyLocale`/`languageChanged`, reload each registered plugin translator for
     the new locale (same as host catalog reload).
   - `unregisterPluginTranslations(pluginId)` removes + uninstalls.
2. **Host wiring**: implement `HostI18n` (returned by `Host::i18n()`) delegating to
   `I18nManager`. `PluginRegistry` calls register on enable (using the plugin's
   `i18n/` dir) and unregister on disable.

## Files to Modify
- `src/i18n/I18nManager.{h,cpp}`, `src/plugin/host/{Host,PluginRegistry}.{h,cpp}`.

## Verification
- [ ] A plugin's `silent_en.lang` loads on enable; its `TR("...")` renders; switching
      locale updates plugin text live; no collisions with host keys.

## Reviewer Criteria
- Must: key prefixing matches the plugin-side `TR` prefix exactly.
- Must: host `Tr.h` untouched; reload on locale change covers plugin translators.
- Must: unregister on disable (no stale translators).

# silent-plugin-system

> Runtime-loadable plugin system for the Silent Bitcoin wallet (Qt6 C++ + Rust/CXX).

## Overview

Silent is today a monolithic single-account wallet. This project turns account
types, data feeds, signing devices, themes, and tabs into **runtime-loadable
plugins**. The full architecture is specified in `docs/PLUGINS.md` (branch
`docs/plugin-system`) which is the authoritative source for every interface shape.

The system has three levels: **Plugin** (a loaded library + sidecar metadata, exposing
`IPlugin::modules()`) → **Module** (`IModule`, owns instances) → **Instance**
(`IInstance`, composes capability sub-objects). Capabilities (`IAccount`, `IFeed`,
`ISigner`, `IThemeProvider`) are QObjects exposed to *other* modules; each gates on a
synchronous `implemented() const` and otherwise uses async request methods that return
a `ReqId` with a same-named result signal, plus a universal `raw()` and
`error(std::optional<ReqId>, QString)`. UI is *pushed* to a `Host` singleton. The
static-Qt host re-exports Qt so runtime plugins link no Qt of their own.

## Goals

- Define a stable C++ plugin SDK (header interfaces) matching `docs/PLUGINS.md`.
- Refactor the existing Silent Payments wallet into the first compiled-in module.
- Make the launcher, tabs, and settings generic (registry-driven).
- Load external plugins at runtime from `~/.silent/plugins/` via `QPluginLoader`.
- Ship an example out-of-tree plugin proving the boundary.

## Success Criteria

- [ ] SP wallet works end-to-end as module #1 (`just br`).
- [ ] Example `.so` is discovered disabled, hot-enables live, and is readable by
      siblings via the async capability path.
- [ ] `cdm --sanity-check` passes; `cargo test` and `cargo clippy` clean.

## Phases

The 27 phases (one task each) are listed in `tasks.json` / `ROADMAP.md`, grouped:
SDK (1-7), Rust/persistence (8-9), host impl (10-12), SP as module #1 (13-16),
de-hardcode host UI (17-19), global Settings (20-21), i18n (22), runtime loading +
build (23-25), example plugin (26-27).

## Out of Scope

- Trust pipeline over sidecar metadata (hash/signature verification).
- Cross-account spend-from-sibling multi-account PSBT (needs bwk-sp coordination).
- Deeper theming (QSS/fonts); active-provider selection across competing feeds.

## References

- `docs/PLUGINS.md` (branch `docs/plugin-system`) - authoritative interface spec.
- `CODE_GUIDELINES.md` - mandatory project conventions.

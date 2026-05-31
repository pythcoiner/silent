# Project Structure

## Directory Layout

```
/
├── src/                      - C++ Qt6 GUI (host application)
│   ├── main.cpp              - app bootstrap
│   ├── AppController.*       - singleton app controller (becomes registry/host owner)
│   ├── AccountController.*   - per-account controller (moves to src/module/sp/)
│   ├── AccountWidget.*       - per-account tab UI (moves to src/module/sp/)
│   ├── MainWindow.*          - QTabWidget host window
│   ├── screens/              - Coins/Send/Receive/History/Settings/MenuTab
│   ├── i18n/                 - I18nManager (.lang catalogs), Tr.h
│   ├── theme/                - Theme/Palette/styled widgets
│   ├── plugin/               - NEW: sdk/ (interfaces), host/ (Host, PluginRegistry)
│   └── module/sp/            - NEW: Silent Payments as module #1
├── silent/                   - Rust crate (CXX bridge: lib.rs, config.rs, account.rs)
├── lib/qontrol/              - vendored Qt6 widget framework (STATIC)
├── lib/include/              - generated CXX headers (silent.h, cxx.h)
├── contrib/example-plugin/   - NEW: out-of-tree example plugin
├── docs/PLUGINS.md           - authoritative plugin spec (branch docs/plugin-system)
├── CMakeLists.txt            - host build
└── flake.nix                 - reproducible static-Qt release builds
```

## Key Components

### Plugin SDK (`src/plugin/sdk/`)
- **Purpose**: pure-virtual C++ interfaces defining the host<->module contract.
- **Key files**: `types.h`, `interfaces/*.h`, `host.h`, `tr.h`.
- **Dependencies**: Qt Core/Gui/Widgets, qontrol (for `theme::Palette`).

### Host (`src/plugin/host/`)
- **Purpose**: `Host` singleton (UI push, instance registry) + `PluginRegistry`
  (compiled-in `Q_IMPORT_PLUGIN` + runtime `QPluginLoader`).

### SP module (`src/module/sp/`)
- **Purpose**: the Silent Payments wallet as the first module; `IAccount` adapter
  over the existing `AccountController`/FFI.

## Build Artifacts
- `lib/libsilent.a` + `lib/include/{silent.h,cxx.h}` (from `build.sh`).
- `silent` executable; `contrib/example-plugin` `.so` + `<id>.json`.

## Notes for Agents
- `docs/PLUGINS.md` is the source of truth for every interface; match it exactly.
- Read `CODE_GUIDELINES.md` before writing code. No lambdas in `connect()`; named
  slots + `qontrol::UNIQUE`; three-phase widgets; `m_` members; no em dashes.
- Boundary types are Qt/std only, never `rust::*`.
- Run `./build.sh` after any change to `silent/src/*.rs` to regenerate CXX headers.

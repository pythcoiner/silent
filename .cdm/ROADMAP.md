# silent-plugin-system - Roadmap

This document tracks implementation progress. Check off items as they are completed.

## Phase 1: SDK: neutral types + CMake scaffold

Status: **Not Started** (0/1)

- [ ] SDK neutral types, smoke-test target, root CMake wiring

---

## Phase 2: SDK: IPlugin + IModule

Status: **Not Started** (0/1)

- [ ] IPlugin + IModule interfaces

---

## Phase 3: SDK: IAccount + IFeed

Status: **Not Started** (0/1)

- [ ] IAccount + IFeed capability interfaces

---

## Phase 4: SDK: ISigner + IThemeProvider

Status: **Not Started** (0/1)

- [ ] ISigner (bwk Signer projection) + IThemeProvider

---

## Phase 5: SDK: IInstance + mock smoke test

Status: **Not Started** (0/1)

- [ ] IInstance composition + mock-instance smoke test

---

## Phase 6: SDK: Host + HostEvents interface

Status: **Not Started** (0/1)

- [ ] Host singleton interface + HostEvents

---

## Phase 7: SDK: plugin-side TR macro

Status: **Not Started** (0/1)

- [ ] Plugin-side id-prefixing TR macro

---

## Phase 8: Rust: plugin_id on Config

Status: **Not Started** (0/1)

- [ ] Add plugin_id to Config and thread through FFI

---

## Phase 9: Rust: app-state persistence

Status: **Not Started** (0/1)

- [ ] app.json: enabled-plugin set + active theme

---

## Phase 10: Host: singleton + instance registry

Status: **Not Started** (0/1)

- [ ] Host singleton + registerInstance/instances()

---

## Phase 11: Host: push-UI id maps

Status: **Not Started** (0/1)

- [ ] Push-UI tab/section/wizard id->widget maps

---

## Phase 12: Host: PluginRegistry (compiled-in)

Status: **Not Started** (0/1)

- [ ] PluginRegistry compiled-in path (Q_IMPORT_PLUGIN)

---

## Phase 13: SP: relocate into src/module/sp/

Status: **Not Started** (0/1)

- [ ] Move SP code to src/module/sp/ (behavior unchanged)

---

## Phase 14: SP: SpPlugin + SpModule

Status: **Not Started** (0/1)

- [ ] SpPlugin + SpModule (list/startInstance)

---

## Phase 15: SP: SpInstance + tab push

Status: **Not Started** (0/1)

- [ ] SpInstance composition + tab push

---

## Phase 16: SP: SpAccount IAccount adapter

Status: **Not Started** (0/1)

- [ ] SpAccount IAccount adapter over FFI

---

## Phase 17: Host UI: generic MainWindow tabs

Status: **Not Started** (0/1)

- [ ] MainWindow generic host-driven tabs

---

## Phase 18: Host UI: registry-driven AppController

Status: **Not Started** (0/1)

- [ ] AppController registry-driven + main.cpp bootstrap

---

## Phase 19: Host UI: generic MenuTab launcher

Status: **Not Started** (0/1)

- [ ] MenuTab launcher from each module's list()

---

## Phase 20: Settings: plugin manager

Status: **Not Started** (0/1)

- [ ] Global Settings screen + enable/disable toggles

---

## Phase 21: Settings: sections + theme picker

Status: **Not Started** (0/1)

- [ ] Aggregated setting sections + active-theme picker

---

## Phase 22: i18n: plugin translations

Status: **Not Started** (0/1)

- [ ] Per-plugin CatalogTranslator with id-prefix + reload

---

## Phase 23: Runtime: QPluginLoader discovery

Status: **Not Started** (0/1)

- [ ] PluginRegistry QPluginLoader scan + enable/disable/unload

---

## Phase 24: Build: link config + ABI surface

Status: **Not Started** (0/1)

- [ ] CMake whole-archive Qt + export-dynamic + QtAbiSurface.cpp

---

## Phase 25: Build: Nix link flags

Status: **Not Started** (0/1)

- [ ] flake.nix per-platform link flags

---

## Phase 26: Example plugin: skeleton

Status: **Not Started** (0/1)

- [ ] Example plugin skeleton + metadata + CMake

---

## Phase 27: Example plugin: capability + i18n

Status: **Not Started** (0/1)

- [ ] Example plugin capability impl + .lang (end-to-end validation)

---

## Summary

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: SDK: neutral types + CMake scaffold | Not Started | 0/1 |
| Phase 2: SDK: IPlugin + IModule | Not Started | 0/1 |
| Phase 3: SDK: IAccount + IFeed | Not Started | 0/1 |
| Phase 4: SDK: ISigner + IThemeProvider | Not Started | 0/1 |
| Phase 5: SDK: IInstance + mock smoke test | Not Started | 0/1 |
| Phase 6: SDK: Host + HostEvents interface | Not Started | 0/1 |
| Phase 7: SDK: plugin-side TR macro | Not Started | 0/1 |
| Phase 8: Rust: plugin_id on Config | Not Started | 0/1 |
| Phase 9: Rust: app-state persistence | Not Started | 0/1 |
| Phase 10: Host: singleton + instance registry | Not Started | 0/1 |
| Phase 11: Host: push-UI id maps | Not Started | 0/1 |
| Phase 12: Host: PluginRegistry (compiled-in) | Not Started | 0/1 |
| Phase 13: SP: relocate into src/module/sp/ | Not Started | 0/1 |
| Phase 14: SP: SpPlugin + SpModule | Not Started | 0/1 |
| Phase 15: SP: SpInstance + tab push | Not Started | 0/1 |
| Phase 16: SP: SpAccount IAccount adapter | Not Started | 0/1 |
| Phase 17: Host UI: generic MainWindow tabs | Not Started | 0/1 |
| Phase 18: Host UI: registry-driven AppController | Not Started | 0/1 |
| Phase 19: Host UI: generic MenuTab launcher | Not Started | 0/1 |
| Phase 20: Settings: plugin manager | Not Started | 0/1 |
| Phase 21: Settings: sections + theme picker | Not Started | 0/1 |
| Phase 22: i18n: plugin translations | Not Started | 0/1 |
| Phase 23: Runtime: QPluginLoader discovery | Not Started | 0/1 |
| Phase 24: Build: link config + ABI surface | Not Started | 0/1 |
| Phase 25: Build: Nix link flags | Not Started | 0/1 |
| Phase 26: Example plugin: skeleton | Not Started | 0/1 |
| Phase 27: Example plugin: capability + i18n | Not Started | 0/1 |
| **Total** | | **0/27** |

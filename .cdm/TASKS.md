# silent-plugin-system - Tasks

This document shows phase plans and task status. Generated from tasks.json.

## phase-1: SDK: neutral types + CMake scaffold

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-1.task-1**: SDK neutral types, smoke-test target, root CMake wiring - SDK neutral types, smoke-test target, root CMake wiring

---

## phase-2: SDK: IPlugin + IModule

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-2.task-1**: IPlugin + IModule interfaces - IPlugin + IModule interfaces

---

## phase-3: SDK: IAccount + IFeed

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-3.task-1**: IAccount + IFeed capability interfaces - IAccount + IFeed capability interfaces

---

## phase-4: SDK: ISigner + IThemeProvider

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-4.task-1**: ISigner (bwk Signer projection) + IThemeProvider - ISigner (bwk Signer projection) + IThemeProvider

---

## phase-5: SDK: IInstance + mock smoke test

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-5.task-1**: IInstance composition + mock-instance smoke test - IInstance composition + mock-instance smoke test

---

## phase-6: SDK: Host + HostEvents interface

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-6.task-1**: Host singleton interface + HostEvents - Host singleton interface + HostEvents

---

## phase-7: SDK: plugin-side TR macro

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-7.task-1**: Plugin-side id-prefixing TR macro - Plugin-side id-prefixing TR macro

---

## phase-8: Rust: plugin_id on Config

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-8.task-1**: Add plugin_id to Config and thread through FFI - Add plugin_id to Config and thread through FFI

---

## phase-9: Rust: app-state persistence

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-9.task-1**: app.json: enabled-plugin set + active theme - app.json: enabled-plugin set + active theme

---

## phase-10: Host: singleton + instance registry

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-10.task-1**: Host singleton + registerInstance/instances() - Host singleton + registerInstance/instances()

---

## phase-11: Host: push-UI id maps

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-11.task-1**: Push-UI tab/section/wizard id->widget maps - Push-UI tab/section/wizard id->widget maps

---

## phase-12: Host: PluginRegistry (compiled-in)

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-12.task-1**: PluginRegistry compiled-in path (Q_IMPORT_PLUGIN) - PluginRegistry compiled-in path (Q_IMPORT_PLUGIN)

---

## phase-13: SP: relocate into src/module/sp/

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-13.task-1**: Move SP code (controller, widget, screens) to src/module/sp/ - Move SP code to src/module/sp/ (behavior unchanged)

---

## phase-14: SP: SpPlugin + SpModule

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-14.task-1**: SpPlugin + SpModule (list/startInstance) - SpPlugin + SpModule (list/startInstance)

---

## phase-15: SP: SpInstance + tab push

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-15.task-1**: SpInstance composition + tab push - SpInstance composition + tab push

---

## phase-16: SP: SpAccount IAccount adapter

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-16.task-1**: SpAccount IAccount adapter over FFI - SpAccount IAccount adapter over FFI

---

## phase-17: Host UI: generic MainWindow tabs

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-17.task-1**: MainWindow generic host-driven tabs - MainWindow generic host-driven tabs

---

## phase-18: Host UI: registry-driven AppController

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-18.task-1**: AppController registry-driven + main.cpp bootstrap - AppController registry-driven + main.cpp bootstrap

---

## phase-19: Host UI: generic MenuTab launcher

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-19.task-1**: MenuTab launcher from each module's list() - MenuTab launcher from each module's list()

---

## phase-20: Settings: plugin manager

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-20.task-1**: Global Settings screen + enable/disable toggles - Global Settings screen + enable/disable toggles

---

## phase-21: Settings: sections + theme picker

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-21.task-1**: Aggregated setting sections + active-theme picker - Aggregated setting sections + active-theme picker

---

## phase-22: i18n: plugin translations

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-22.task-1**: Per-plugin CatalogTranslator with id-prefix + reload - Per-plugin CatalogTranslator with id-prefix + reload

---

## phase-23: Runtime: QPluginLoader discovery

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-23.task-1**: PluginRegistry QPluginLoader scan + enable/disable/unload - PluginRegistry QPluginLoader scan + enable/disable/unload

---

## phase-24: Build: link config + ABI surface

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-24.task-1**: CMake whole-archive Qt + export-dynamic + QtAbiSurface.cpp - CMake whole-archive Qt + export-dynamic + QtAbiSurface.cpp

---

## phase-25: Build: Nix link flags

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-25.task-1**: flake.nix per-platform link flags - flake.nix per-platform link flags

---

## phase-26: Example plugin: skeleton

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-26.task-1**: Example plugin skeleton + metadata + CMake - Example plugin skeleton + metadata + CMake

---

## phase-27: Example plugin: capability + i18n

**Status:** Pending (0/1)

### Tasks

- [ ] **phase-27.task-1**: Example plugin capability impl + .lang (end-to-end validation) - Example plugin capability impl + .lang (end-to-end validation)

---

## Summary

| Phase | Status | Tasks | Completed |
|-------|--------|-------|----------:|
| phase-1: SDK: neutral types + CMake ... | Pending | 1 | 0 |
| phase-2: SDK: IPlugin + IModule | Pending | 1 | 0 |
| phase-3: SDK: IAccount + IFeed | Pending | 1 | 0 |
| phase-4: SDK: ISigner + IThemeProvider | Pending | 1 | 0 |
| phase-5: SDK: IInstance + mock smoke... | Pending | 1 | 0 |
| phase-6: SDK: Host + HostEvents inte... | Pending | 1 | 0 |
| phase-7: SDK: plugin-side TR macro | Pending | 1 | 0 |
| phase-8: Rust: plugin_id on Config | Pending | 1 | 0 |
| phase-9: Rust: app-state persistence | Pending | 1 | 0 |
| phase-10: Host: singleton + instance ... | Pending | 1 | 0 |
| phase-11: Host: push-UI id maps | Pending | 1 | 0 |
| phase-12: Host: PluginRegistry (compi... | Pending | 1 | 0 |
| phase-13: SP: relocate into src/modul... | Pending | 1 | 0 |
| phase-14: SP: SpPlugin + SpModule | Pending | 1 | 0 |
| phase-15: SP: SpInstance + tab push | Pending | 1 | 0 |
| phase-16: SP: SpAccount IAccount adapter | Pending | 1 | 0 |
| phase-17: Host UI: generic MainWindow... | Pending | 1 | 0 |
| phase-18: Host UI: registry-driven Ap... | Pending | 1 | 0 |
| phase-19: Host UI: generic MenuTab la... | Pending | 1 | 0 |
| phase-20: Settings: plugin manager | Pending | 1 | 0 |
| phase-21: Settings: sections + theme ... | Pending | 1 | 0 |
| phase-22: i18n: plugin translations | Pending | 1 | 0 |
| phase-23: Runtime: QPluginLoader disc... | Pending | 1 | 0 |
| phase-24: Build: link config + ABI su... | Pending | 1 | 0 |
| phase-25: Build: Nix link flags | Pending | 1 | 0 |
| phase-26: Example plugin: skeleton | Pending | 1 | 0 |
| phase-27: Example plugin: capability ... | Pending | 1 | 0 |
| **Total** | | **27** | **0** |

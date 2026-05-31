Host singleton + registerInstance/instances()

## Objective
Implement the concrete `Host` singleton (the `src/plugin/sdk/host.h` interface):
instance registry + `HostEvents` emission. UI push methods are stubbed here and
filled in phase 11.

## Files to Read
- `docs/PLUGINS.md` - "The Host".
- `src/plugin/sdk/host.h` - the interface to implement.
- `src/AppController.h` - where the host will be owned/initialized.

## Implementation Steps
1. **`src/plugin/host/Host.{h,cpp}`**
   - Concrete `Host` implementing the SDK interface; `static Host* get()` returns a
     process singleton (created at app init).
   - `registerInstance(id, IInstance*)`: store in `QHash<QString, IInstance*>`; emit
     `events()->instanceRegistered(id)`. Provide an internal `removeInstance(id)` that
     emits `instanceRemoved`.
   - `instances()`: return current values.
   - Own a `HostEvents` member; `events()` returns it.
   - UI push methods: declare + stub (return 0 / no-op) with a TODO for phase 11.
2. **Wire init**: construct the singleton in `main.cpp`/`AppController::init` ordering
   (do not call `initState` before the host exists). Minimal change here; full
   AppController rewrite is phase 18.

## Files to Create
- `src/plugin/host/Host.h`, `src/plugin/host/Host.cpp`.

## Files to Modify
- `CMakeLists.txt` - add the host sources; `src/main.cpp` - construct singleton early.

## Verification
- [ ] `just build-local` (or cmake build) links; app still launches unchanged.

## Reviewer Criteria
- Must: single instance; thread-aware (registry accessed on GUI thread).
- Must: `instanceRegistered/Removed` emitted on register/remove.
- Must: named slots only if any connect is added; no lambdas.

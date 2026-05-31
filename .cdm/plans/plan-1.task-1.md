SDK neutral types, smoke-test target, root CMake wiring

## Objective
Create the plugin SDK skeleton: the neutral boundary types and a buildable
smoke-test target so later SDK phases have something to compile against.

## Files to Read
- `docs/PLUGINS.md` - "Capability Interfaces" intro + the `namespace plugin { ... }`
  POD block (authoritative shape for `Coin`/`Balance`).
- `CMakeLists.txt` - how `find_package(Qt6 ...)`, `qontrol`, and SOURCES are wired.

## Implementation Steps
1. **Create `src/plugin/sdk/types.h`**
   - `#pragma once`; include `<QString>`, `<QList>`, `<QtGlobal>`.
   - `using ReqId = quint64;`
   - `namespace plugin { struct Coin { QString outpoint; quint64 value; quint32 height;
     QString label; bool spent; QString accountType; }; struct Balance { quint64
     confirmed; quint64 unconfirmed; }; }` (match docs/PLUGINS.md exactly).
2. **Create the SDK CMake target** (`src/plugin/sdk/CMakeLists.txt`)
   - `add_library(silent_plugin_sdk INTERFACE)`; `target_include_directories(... INTERFACE
     ${CMAKE_CURRENT_SOURCE_DIR}/..)` so includes read `sdk/types.h`.
   - Link `INTERFACE Qt6::Core Qt6::Gui Qt6::Widgets qontrol`.
3. **Create a smoke-test TU** `src/plugin/sdk/smoke/main.cpp`
   - Includes `sdk/types.h`, constructs a `plugin::Coin`/`Balance`, `int main(){return 0;}`.
   - Define the executable `silent_plugin_sdk_smoke` linking `silent_plugin_sdk` + Qt,
     with AUTOMOC ON (later SDK phases add headers it includes).
4. **Wire into root `CMakeLists.txt`**
   - `add_subdirectory(src/plugin/sdk)` (after qontrol is available).
   - The smoke target is **always built** as part of the normal cmake build (NOT
     behind an off-by-default option), so the single `build_commands` set verifies the
     SDK every phase alongside the app. Keep it cheap (one tiny TU).

## Files to Create
- `src/plugin/sdk/types.h`, `src/plugin/sdk/CMakeLists.txt`,
  `src/plugin/sdk/smoke/main.cpp`.

## Files to Modify
- `CMakeLists.txt` - option + `add_subdirectory` + smoke target.

## Verification
- [ ] `bash build.sh` succeeds (Rust crate + CXX headers).
- [ ] `cmake -B build .` configures with the new `add_subdirectory(src/plugin/sdk)`.
- [ ] `cmake --build build -j` builds the app AND the `silent_plugin_sdk_smoke` target
      clean (smoke target is always built, not behind an option).

## Reviewer Criteria
- Must: types match docs/PLUGINS.md byte-for-byte (field names/order/types).
- Must: `#pragma once`, alphabetized includes, no em dashes.
- May skip: installing/exporting the SDK target (later concern).

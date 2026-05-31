Move SP code to src/module/sp/ (behavior unchanged)

## Objective
Relocate the Silent Payments wallet code into `src/module/sp/` so it becomes a
self-contained module. Pure move + include/CMake fixups; no behavior change.

## Files to Read
- `src/AccountController.{h,cpp}`, `src/AccountWidget.{h,cpp}`, `src/StatusBar.{h,cpp}`.
- `src/screens/{Coins,Send,Receive,History,Settings}.{h,cpp}`,
  `src/screens/modals/CreateAccount.{h,cpp}`.
- `CMakeLists.txt` - the SOURCES list referencing these files.

## Implementation Steps
1. **Move files** into `src/module/sp/` (keep `screens/` + `modals/` subfolders),
   e.g. `src/module/sp/AccountController.*`, `src/module/sp/AccountWidget.*`,
   `src/module/sp/StatusBar.*`, `src/module/sp/screens/*`, `src/module/sp/modals/*`.
   Note: `MenuTab` stays in `src/screens/` (it is the launcher, host-level, not SP).
2. **Fix includes**: update `#include` paths in moved files and in any host file that
   referenced them (AppController, MainWindow). Keep the `screen::`/`modal::`
   namespaces unchanged.
3. **CMake**: update SOURCES paths; add `src/module/sp/` to include dirs if needed.
4. Do not change logic. This phase must be a behavior-preserving relocation.

## Files to Modify
- moved files' includes; `CMakeLists.txt`; `src/AppController.*`, `src/MainWindow.*`
  (include paths only).

## Verification
- [ ] `just br` builds and runs; SP account create/scan/send still works exactly as
      before (no behavioral change).

## Reviewer Criteria
- Must: diff is moves + include/path edits only (no logic changes).
- Must: `MenuTab` NOT moved (it is host-level launcher).
- Must: app builds and runs identically.

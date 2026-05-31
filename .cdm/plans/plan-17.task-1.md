MainWindow generic host-driven tabs

## Objective
Generalize `MainWindow` so tabs are driven by the host's push-UI (plain `QWidget*` +
owner), removing the SP-specific `insertAccount(AccountWidget*)` coupling.

## Files to Read
- `src/MainWindow.{h,cpp}` - `insertAccount`/`removeAccount`/`updateTabs`/
  `onTabCloseRequested`.
- `src/plugin/host/Host.{h,cpp}` - tab maps from phase 11.

## Implementation Steps
1. **MainWindow** - replace `insertAccount(AccountWidget*, name)` with a generic
   `addTab(QWidget*, const QString &title) -> int` (or have the Host call MainWindow
   internally). Keep the MenuTab (launcher) pinned as the last, non-closable tab.
2. **Close handling** - `onTabCloseRequested(index)`: map the index back to the owning
   `IInstance` via the Host's tab map and call `instance->stop()` (which closes the
   tab through the host), instead of `AppController::removeAccount`.
3. Ensure `closeEvent` stops all instances via the registry/host.

## Files to Modify
- `src/MainWindow.{h,cpp}`, `src/plugin/host/Host.{h,cpp}` (tab placement hook).

## Verification
- [ ] `just br`: SP tabs open/close through the generic path; launcher tab stays put.

## Reviewer Criteria
- Must: no `AccountWidget` type reference left in MainWindow.
- Must: tab close routes to `IInstance::stop()` via host map.
- Must: named slots; no lambdas.

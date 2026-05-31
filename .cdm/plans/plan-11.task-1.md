Push-UI tab/section/wizard id->widget maps

## Objective
Implement the host's pushed-UI surface: instances supply widgets and get back ids;
the host owns the id->widget maps and the actual `QTabWidget`/settings/wizard
placement.

## Files to Read
- `docs/PLUGINS.md` - "The Host" UI methods + "UI is pushed" model.
- `src/MainWindow.h/.cpp` - current `QTabWidget` (`insertAccount`/`removeAccount`).
- `src/plugin/host/Host.h` - the stubs from phase 10.

## Implementation Steps
1. **`Host` tab API**
   - `openTab(QWidget*, IInstance* owner) -> TabId`: insert into MainWindow's
     `QTabWidget`, store `TabId -> {widget, owner}`; enforce at most one tab per
     instance (return existing id if already open).
   - `setTabTitle(TabId, QString)`, `closeTab(TabId)` (remove + drop from map).
2. **Setting sections + wizard**
   - `addSettingSection(QWidget*, owner) -> SectionId` / `delSettingSection`: hold a
     registry the AppSettings screen (phase 21) reads; emit a change signal so an open
     Settings screen refreshes.
   - `openWizard(QWidget*, owner) -> WizardId` / `closeWizard`: wrap the widget in a
     `qontrol::Modal`-style dialog (reuse qontrol), track by id.
3. **Teardown**: on instance `stop()`/removal, close all its tabs/sections/wizards.

## Files to Modify
- `src/plugin/host/Host.{h,cpp}`, `src/MainWindow.{h,cpp}` (expose a tab-host hook).

## Verification
- [ ] Build links; a temporary smoke call (open+retitle+close a dummy tab) works.

## Reviewer Criteria
- Must: one tab per instance enforced; ids stable; maps cleaned on close/teardown.
- Must: wizard reuses qontrol Modal; no bespoke dialog framework.
- Must: no lambdas in connect; named slots; `qontrol::UNIQUE`.

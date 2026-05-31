SpInstance composition + tab push

## Objective
Implement `SpInstance : IInstance` owning the existing `AccountController` and pushing
its `AccountWidget` tab via the host.

## Files to Read
- `docs/PLUGINS.md` - IInstance + "UI is pushed".
- `src/module/sp/AccountController.h`, `src/module/sp/AccountWidget.h`.
- `src/plugin/host/Host.h` - `openTab`/`closeTab`.

## Implementation Steps
1. **`src/module/sp/SpInstance.{h,cpp}`** - `SpInstance : public IInstance`:
   - ctor takes the account id; constructs `AccountController` + `AccountWidget`
     (as today) and keeps them as members.
   - `id()` returns the account id (immutable).
   - on bring-up: `Host::get()->openTab(accountWidget, this)`, keep the `TabId`.
   - `account()` returns the `SpAccount` adapter (phase 16; return a member here,
     wired in 16). `feed()/signer()/theme()` return small inert objects whose
     `implemented()==false`.
   - `stop()`: stop the controller (existing `AccountController::stop()`), close the
     tab via the stored `TabId`.
2. Hook `SpModule::startInstance` to construct + register an `SpInstance`.

## Files to Create
- `src/module/sp/SpInstance.{h,cpp}` (+ small inert capability stubs, or reuse from a
  shared "InertCapability" helper in the SDK/host).

## Verification
- [ ] `just br`: clicking an SP account in the launcher opens its tab via the host
      push path (not the old direct `insertAccount`).

## Reviewer Criteria
- Must: tab opened through `Host::openTab`, not MainWindow directly.
- Must: `stop()` tears down controller + tab cleanly.
- Must: inert capabilities return `implemented()==false` and never emit.

SpAccount IAccount adapter over FFI

## Objective
Implement `SpAccount : IAccount` as a thin async adapter over the existing
`AccountController`/Rust FFI, so sibling modules can read the SP account. SP's own
screens keep using `AccountController` directly (unchanged).

## Files to Read
- `docs/PLUGINS.md` - IAccount methods/signals.
- `src/module/sp/AccountController.h` - `getCoins()`, `getSpAddress()`, balance, the
  notification signals.
- `silent/src/lib.rs` - `coins()`, `spendable_coins()`, `new_segwit_addr()`, etc.

## Implementation Steps
1. **`src/module/sp/SpAccount.{h,cpp}`** - `SpAccount : public IAccount`:
   - `implemented() const` -> true.
   - request methods allocate a `ReqId` (monotonic counter), perform the work via the
     owning `AccountController`/FFI (cheap reads can be done inline then emit; slow
     ones via the existing notification/`QThread::create` path), and emit the matching
     signal with that `ReqId`:
     - `coins()` -> convert `rust::Vec<RustCoin>` to `QList<plugin::Coin>`, emit
       `coins(req, list)`.
     - `balance()` -> emit `balance(req, {confirmed, unconfirmed})`.
     - `newReceiveAddress()` -> `new_segwit_addr()`, emit `receiveAddress(req, addr)`.
     - `name()/info()/newChangeAddress()` similarly; `raw()` -> `error`/empty for now.
   - Bridge live updates: when `AccountController` emits `updateCoins`/`updateBalance`,
     re-emit `coins`/`balance` with `ReqId 0` (unsolicited push) so subscribers stay
     current.
2. Wire `SpInstance::account()` to return this object.

## Files to Create
- `src/module/sp/SpAccount.{h,cpp}`.

## Verification
- [ ] `just br`: SP works unchanged; a temporary test consumer connecting to
      `instance->account()->coins(...)` receives the converted list.

## Reviewer Criteria
- Must: conversion `rust::Vec<RustCoin>` -> `QList<plugin::Coin>` is correct + total.
- Must: boundary exposes only Qt/std types (no `rust::*` leaks).
- Must: live updates re-emitted with ReqId 0; named slots only.

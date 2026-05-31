IInstance composition + mock-instance smoke test

## Objective
Define `IInstance` (composes the capability sub-objects, not multiple-QObject
inheritance) and prove the whole SDK contract compiles via a mock instance.

## Files to Read
- `docs/PLUGINS.md` - the `IInstance` sketch + the "composition, not inheritance" note.
- `src/plugin/sdk/interfaces/*.h`.

## Implementation Steps
1. **`src/plugin/sdk/interfaces/instance.h`** - `IInstance`:
   - virtual dtor; `virtual QString id() const = 0;` (immutable, set at construction);
     `virtual void stop() = 0;`
   - accessors: `virtual IAccount* account() = 0; virtual IFeed* feed() = 0; virtual
     ISigner* signer() = 0; virtual IThemeProvider* theme() = 0;`
   - `Q_DECLARE_INTERFACE(IInstance, "dev.silent.IInstance/1.0")`.
2. **Mock smoke test** `src/plugin/sdk/smoke/mock.cpp`
   - Define `MockAccount : public IAccount` (and minimal mock feed/signer/theme) with
     `implemented()` returning a constant and request methods returning a counter
     `ReqId` (no real work). Define `MockInstance : public IInstance` returning the
     mocks. Instantiate one in `main()` and call `account()->implemented()`.
   - This proves the pure-virtual surface is implementable and MOC-clean.

## Files to Create
- `src/plugin/sdk/interfaces/instance.h`, `src/plugin/sdk/smoke/mock.cpp`.

## Files to Modify
- `src/plugin/sdk/CMakeLists.txt` - add `mock.cpp` to the smoke target.

## Verification
- [ ] SDK smoke target builds + links with the mock implementation.

## Reviewer Criteria
- Must: `IInstance` composes via accessors, does not inherit the capability QObjects.
- Must: mock subclasses compile (proves no accidentally-unimplementable signatures).
- May skip: realistic mock behavior (constants are fine).

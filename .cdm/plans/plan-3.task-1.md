IAccount + IFeed capability interfaces

## Objective
Define the `IAccount` and `IFeed` capability QObjects: `implemented() const` gate,
async request methods returning `ReqId`, same-named result signals, universal
`raw()` + `error()`.

## Files to Read
- `docs/PLUGINS.md` - the `IAccount` and `IFeed` sketches (exact methods + signals).
- `src/plugin/sdk/types.h`.

## Implementation Steps
1. **`src/plugin/sdk/interfaces/account.h`** - `IAccount : public QObject`/`Q_OBJECT`:
   - methods: `bool implemented() const`; `ReqId name()/info()/coins()/balance()/
     newReceiveAddress()/newChangeAddress()`; `ReqId raw(const QByteArray&)`.
   - signals: `name(ReqId,QString)`, `info(ReqId,QMap<QString,QString>)`,
     `coins(ReqId,QList<plugin::Coin>)`, `balance(ReqId,plugin::Balance)`,
     `receiveAddress(ReqId,QString)`, `changeAddress(ReqId,QString)`,
     `raw(ReqId,QByteArray)`, `error(std::optional<ReqId>,QString)`.
   - `Q_DECLARE_INTERFACE(IAccount, "dev.silent.IAccount/1.0")`.
2. **`src/plugin/sdk/interfaces/feed.h`** - `IFeed : public QObject`/`Q_OBJECT`:
   - methods: `bool implemented() const`; `ReqId raw(const QByteArray&)`.
   - signals: `feeRate(ReqId,double)`, `conversionRate(ReqId,QString,double)`,
     `raw(ReqId,QByteArray)`, `error(std::optional<ReqId>,QString)`.
   - `Q_DECLARE_INTERFACE(IFeed, "dev.silent.IFeed/1.0")`.
3. Register `QList<plugin::Coin>` / `plugin::Balance` as metatypes if needed for
   queued signal delivery (`Q_DECLARE_METATYPE` in types.h or here).

## Files to Create
- `src/plugin/sdk/interfaces/account.h`, `src/plugin/sdk/interfaces/feed.h`.

## Verification
- [ ] SDK smoke target builds (include both headers) with AUTOMOC clean.

## Reviewer Criteria
- Must: method/signal name+signature parity with docs/PLUGINS.md.
- Must: `implemented()` is `const`; every other method returns `ReqId`.
- Must: `Q_DECLARE_METATYPE` for any non-builtin signal arg used cross-thread.

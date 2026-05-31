ISigner (bwk Signer projection) + IThemeProvider

## Objective
Define `ISigner` (descriptor-based, the Qt/std projection of bwk's `Signer`/`HwSigner`)
and `IThemeProvider`.

## Files to Read
- `docs/PLUGINS.md` - the `ISigner` and `IThemeProvider` sketches.
- `/home/pyth/bwk/hwi/src/lib.rs` - the `HWI` trait (background; the SDK mirrors the
  higher-level `Signer`/`HwSigner` surface described in the doc, not raw HWI).

## Implementation Steps
1. **`src/plugin/sdk/interfaces/signer.h`** - `ISigner : public QObject`/`Q_OBJECT`:
   - sync: `bool implemented() const`; immutable accessors `QString id() const`,
     `QString fingerprint() const`, `QString walletName() const`.
   - requests: `ReqId init()`, `ReqId info()`, `ReqId getXpub(const QString&)`,
     `ReqId isDescriptorRegistered(const QString&)`, `ReqId registerDescriptor(const
     QString&)`, `ReqId signWithDescriptor(const QString &psbtBase64, const QString
     &descriptor)`, `ReqId raw(const QByteArray&)`.
   - signals: `initialized(ReqId)`, `info(ReqId,QMap<QString,QString>)`,
     `xpub(ReqId,QString)`, `descriptorRegistered(ReqId,bool)`,
     `descriptorIsRegistered(ReqId,bool)`, `signed_(ReqId,QString)`,
     `raw(ReqId,QByteArray)`, `error(std::optional<ReqId>,QString)`.
   - `Q_DECLARE_INTERFACE(ISigner, "dev.silent.ISigner/1.0")`.
2. **`src/plugin/sdk/interfaces/theme.h`** - `IThemeProvider : public QObject`/`Q_OBJECT`:
   - `bool implemented() const`; `ReqId themes()`, `ReqId palette(const QString&)`,
     `ReqId raw(const QByteArray&)`.
   - signals: `themes(ReqId,QStringList)`, `palette(ReqId,theme::Palette)`,
     `raw(ReqId,QByteArray)`, `error(std::optional<ReqId>,QString)`.
   - include qontrol's `theme::Palette` header; `Q_DECLARE_INTERFACE(IThemeProvider,
     "dev.silent.IThemeProvider/1.0")`.

## Files to Create
- `src/plugin/sdk/interfaces/signer.h`, `src/plugin/sdk/interfaces/theme.h`.

## Verification
- [ ] SDK smoke target builds (include both) with AUTOMOC clean.

## Reviewer Criteria
- Must: descriptor-based signer surface (no policy `registerWallet`); `signed_` signal.
- Must: immutable accessors are `const`; requests return `ReqId`.
- Must: `theme::Palette` resolves (qontrol include/link present).

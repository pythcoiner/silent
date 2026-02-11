# CODE_GUIDELINES.md

Project-specific coding conventions and patterns for the Silent wallet.

## Rust: CXX Bridge Conventions (`silent/src/lib.rs`)

All FFI surface area lives in `lib.rs` inside the `#[cxx::bridge]` module.
Implementation goes in `account.rs` or `config.rs`.

**Shared types** use `#[derive(Debug, Clone)]` (structs) or `#[derive(Debug, Clone,
Copy)]` (enums). Prefix wrapper types with `Rust` when they mirror an internal type
(`RustCoin`, `RustTx`). Intermediate serde types use the `Internal` suffix
(`NetworkInternal`).

**Opaque types** (`Config`, `Account`, `Poll`, `PsbtResult`) are declared without
fields in the `extern "Rust"` block. Methods use explicit receiver syntax:
```rust
fn method(self: &Account) -> RetType;
fn method_mut(self: &mut Account) -> RetType;
```

**Error handling across the bridge:** never expose internal error types to C++.
Convert all errors to `String` at the FFI boundary:
```rust
pub fn method(&self) -> Result<String, String> {
    operation.map_err(|e| format!("context: {e}"))
}
```

**Option types** cannot cross the bridge. Use sentinel values (0 for `Option<u64>`)
and convert on the Rust side:
```rust
pub fn get_dust_limit(&self) -> u64 {
    self.dust_limit.unwrap_or(0)
}
pub fn set_dust_limit(&mut self, limit: u64) {
    self.dust_limit = if limit > 0 { Some(limit) } else { None };
}
```

**Sections** in `lib.rs` are delimited by comment headers for organization:
```rust
// ===== Shared Enums =====
// ===== Shared Structs =====
// ===== Opaque Rust Types =====
// ===== Utility Functions =====
// ===== Config Methods =====
// ===== Account Methods =====
```

## Rust: Error Handling

- FFI boundary methods return `Result<T, String>` — never panic across FFI.
- File I/O never panics; log errors with `log::error!()` and return gracefully.
- `unreachable!()` branches on CXX enum conversions include a safety comment:
  ```rust
  _ => unreachable!("CXX enum type safety guarantees no other variants exist")
  ```

## Rust: Config File I/O (`silent/src/config.rs`)

Configs are stored as pretty-printed JSON at `{data_dir}/{account_name}/config.json`.

- Linux: `~/.silent/<account>/`
- Other OS: `{dirs::config_dir()}/Silent/<account>/`
- Unix directories use `0o700` permissions (user-only access).
- `#[serde(skip)]` on runtime-only fields (`data_dir`).
- Custom serde module (`network_serde`) for CXX enums that don't derive Serde.
- Parent directories are created before any write.

## Rust: Notification Polling

The scanner runs in a background thread and sends `SpNotification` values through an
`mpsc::channel`. The C++ side polls via `try_recv()` which returns a `Box<Poll>`
wrapper — never blocks. The `Poll` type encapsulates Ok/None/Err states with
getter methods callable from C++.

## C++: Naming Conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes | PascalCase | `AccountController`, `MainWindow` |
| Methods | camelCase, trailing return type | `auto get() -> AppController *` |
| Member variables | `m_` prefix | `m_account`, `m_notif_timer` |
| Signals | camelCase, descriptive noun/verb | `accountCreated`, `scanProgress` |
| Slot handlers | `on` prefix for signal handlers | `onBackendInfoReady`,
`onLabelEdited` |
| Action slots | imperative verb | `poll()`, `loadPanel()`, `sendTransaction()` |
| Constants | UPPER_CASE (in `common.h`) | `MARGIN`, `SATS`, `V_SPACER` |
| Screen classes | `screen::` namespace | `screen::Send`, `screen::Coins` |
| Modal classes | `modal::` namespace | `modal::SelectCoins`, `modal::CoinWidget` |

**Widget member suffixes:**
`m_*_btn` (QPushButton), `m_*_input` (QLineEdit/QTextEdit), `m_*_label` (QLabel),
`m_*_column` (qontrol::Column), `m_*_row` (qontrol::Row), `m_*_table`
(QTableWidget), `m_*_timer` (QTimer), `m_*_frame` (framed widgets).

## C++: Qontrol Framework Usage

**Screen subclasses** (`qontrol::Screen`) implement a three-phase lifecycle:
1. `init()` — create all widgets
2. `doConnect()` — connect signals/slots
3. `view()` — build layout (called again to rebuild on data changes)

Constructor calls all three phases in order, plus any initial state setup.

**Layout building** uses the fluent API:
```cpp
auto *col = (new qontrol::Column)
    ->push(titleRow)
    ->pushSpacer(20)
    ->push(m_outputs_column)
    ->pushSpacer();
```

**Layout rebuilding:** delete the old layout and main widget, build new ones, then
call `setLayout()`:
```cpp
auto *oldWidget = m_main_widget;
m_main_widget = margin(newRow, 10);
delete oldWidget;
delete this->layout();
this->setLayout(m_main_widget->layout());
```

**Modals:** always use `qontrol::Modal`, never `QDialog`. Modals call
`setMainWidget(widget)` to set their content and are displayed via
`AppController::execModal(modal)`.

**Signal connections** use `qontrol::UNIQUE` flag to avoid duplicate connections:
```cpp
connect(source, &Source::signal, dest, &Dest::slot, qontrol::UNIQUE);
```

## C++: Singleton Pattern

`AppController` inherits from `qontrol::Controller`:
```cpp
AppController::init();           // create singleton (once, fatal on duplicate)
AppController::get();            // retrieve via dynamic_cast
```

## C++: Error Handling

Never throw exceptions on the C++ side. Rust FFI calls that return `Result` will throw
a CXX exception on error — always catch those with `try`/`catch` and handle them
gracefully (e.g. display a `qontrol::Modal` with the error message, or log with
`qWarning()`). The C++ layer should never propagate or originate exceptions itself.

## C++: Rust FFI Integration

**Type conversions:**
```cpp
// rust::String -> QString
QString::fromStdString(std::string(rust_str.c_str()))

// QString -> rust::String
rust::String(qstring.toStdString())
```

**Owned Rust objects** are held as `std::optional<rust::Box<Account>>`. Always check
`has_value()` before access.

**Never block the GUI thread.** Any Rust FFI call that performs I/O or network
requests (e.g. `get_backend_info()`) must run on a background `QThread`. Return the
result to the main thread via `QMetaObject::invokeMethod`. Example from
`Settings`/`CreateAccount`:
```cpp
auto *thread = QThread::create([this, url = url.toStdString()]() {
    auto info = ::get_backend_info(rust::String(url));
    QMetaObject::invokeMethod(this, [this, info]() {
        onBackendInfoReady(info);
    });
});
connect(thread, &QThread::finished, thread, &QThread::deleteLater);
thread->start();
```
This pattern applies to any new blocking FFI call — always `QThread::create` +
lambda + `QMetaObject::invokeMethod` back to the GUI thread.

## C++: Timer-Based Polling

`AccountController` runs two polling timers:
- **100ms** (`m_notif_timer`) — polls `try_recv()` for scanner notifications
- **1000ms** (`m_coins_timer`) — polls coin state and balance

Both timers are stopped in `stop()` before the scanner is shut down. The
`pollNotifications()` loop drains all pending notifications in a single tick.

## C++: Composite Widget Pattern

Reusable widget groups (e.g. `OutputW`, `InputW`, `CoinWidget`, `RadioElement`) are
plain classes (not QWidget subclasses) that own a widget tree and expose it via
`widget()`. Stored in `QHash<int, WidgetType*>` keyed by ID. Manually deleted on
removal:
```cpp
auto *output = m_outputs.take(id);
delete output->widget();
delete output;
```

## C++: Utility Functions (`src/screens/common.h`)

```cpp
auto margin(QWidget *widget) -> QWidget *;             // default margin wrapper
auto margin(QWidget *widget, int margin) -> QWidget *;  // custom margin
auto frame(QWidget *widget) -> QWidget *;               // rounded-rect border
auto toBitcoin(uint64_t sats, bool with_unit = true) -> QString;  // sat -> BTC
string
auto coinsCount(uint64_t count) -> QString;             // pluralized coin count
```

`Frame` is a custom `QFrame` subclass with a `paintEvent` that draws a rounded
rectangle border.

## C++: Pointer Conventions

- **Raw pointers** (`QWidget *m_btn = nullptr`): parent-owned Qt objects (most
widgets).
- **QPointer** (`QPointer<qontrol::Panel>`): observing pointers that auto-null on
deletion.
- **std::optional<rust::Box<T>>**: owned Rust objects that may not be initialized
yet.
- All member pointers initialized to `nullptr` in the header.

## C++: ComboBox Enum Binding

Network enums are stored as `QVariant` item data:
```cpp
m_network_combo->addItem("Regtest", static_cast<int>(Network::Regtest));
// Retrieve:
auto network = static_cast<Network>(m_network_combo->currentData().toInt());
```

## Testing Conventions

**Account naming:** `format!("test_account_{}", std::process::id())` for unique
per-run names.

**Standard test mnemonic:** `"abandon abandon abandon abandon abandon abandon
abandon abandon abandon abandon abandon about"` (BIP39 test vector, Regtest network).

**Lifecycle pattern:**
```rust
let account_name = test_account_name();
let config = create_test_config(account_name.clone());
let account = Account::new(config).expect("...");
// ... assertions ...
cleanup_test_account(&account_name);
```

**Cleanup:** `cleanup_test_account()` loads the config, calls `remove_dir_all()` on
the account directory. Uses `.ok()` to tolerate missing state.

**`#[ignore]` tests** require external infrastructure (BlindBit server, Bitcoin
node). Run with `cargo test -- --ignored`.

**Assertions always include descriptive messages** with context values:
```rust
assert!(sp_addr.len() >= 100, "SP address should be at least 100 characters, got
{}", sp_addr.len());
```

**Test sections** are grouped with comment headers:
```rust
// ===== Wallet Creation and Config Tests =====
// ===== Error Handling Tests =====
```

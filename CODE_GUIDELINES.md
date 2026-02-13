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

**Never return `Result` from Rust to C++ across the CXX FFI boundary.** CXX converts
`Result::Err` into a C++ exception, which we do not want. Instead:

- **Void operations:** return `bool` (`true` = success, `false` = failure). Log the
  error on the Rust side with `log::error!()`.
  ```rust
  // FFI function
  pub fn delete_config(account_name: String) -> bool {
      match SpConfig::delete_account_dir(&data_dir, &account_name) {
          Ok(()) => true,
          Err(e) => { log::error!("failed to delete: {e}"); false }
      }
  }
  ```

- **Operations with response data:** return a specific shared struct with `is_ok: bool`,
  `error: String`, and response fields. Do **not** create a generic `FfiResult` — each
  return type should be purpose-built.
  ```rust
  // In #[cxx::bridge]
  pub struct TxResult {
      pub is_ok: bool,
      pub error: String,
      pub value: String,
  }
  ```

- **Opaque types that can fail construction:** wrap an `Option<Inner>` and expose
  `is_ok()` / `get_error()` methods. Always return the type (never `Result`).
  ```rust
  pub struct Account {
      inner: Option<AccountInner>,
      error: String,
  }
  // FFI: fn new_account(name: String) -> Box<Account>;
  // C++: check account->is_ok() before use
  ```

- `Result` is fine for **internal Rust code** and tests — the restriction only applies
  to functions declared in the `extern "Rust"` block of the CXX bridge.
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
| Local variables | camelBack | `coinState`, `urlRow`, `btnFont` |
| Parameters | lower_case | `tip_height`, `blind_bit_url` |
| Member variables | `m_` prefix + lower_case | `m_account`, `m_notif_timer` |
| Local constants | `c_` prefix + lower_case | `c_threshold` |
| Constants | UPPER_CASE (in `utils.h`) | `MARGIN`, `SATS`, `V_SPACER` |
| Signals | camelCase, descriptive noun/verb | `accountCreated`, `scanProgress` |
| Slot handlers | `on` prefix for signal handlers | `onBackendInfoReady`,
`onLabelEdited` |
| Action slots | imperative verb | `poll()`, `loadPanel()`, `sendTransaction()` |
| Screen classes | `screen::` namespace | `screen::Send`, `screen::Coins` |
| Modal classes | `modal::` namespace | `modal::SelectCoins`, `modal::CoinWidget` |

**Widget member suffixes:**
`m_*_btn` (QPushButton), `m_*_input` (QLineEdit/QTextEdit), `m_*_label` (QLabel),
`m_*_column` (qontrol::Column), `m_*_row` (qontrol::Row), `m_*_table`
(QTableWidget), `m_*_timer` (QTimer), `m_*_frame` (framed widgets).

## C++: Clangd Lint Rules

The `.clangd` config enforces clang-tidy checks and `UnusedIncludes: Strict`. All
new code must satisfy these:

**Include hygiene:**
- Headers use `#pragma once` (not traditional include guards).
- Sort `#include` directives alphabetically within each block (enforced by
  `llvm-include-order`). Case-sensitive sort (uppercase before lowercase).
- Only include headers whose symbols are directly used in the file. clangd's
  `UnusedIncludes: Strict` flags unused includes.
- When a forward declaration (`class Foo;`) is sufficient (pointer/reference members
  or parameters), prefer it over `#include "Foo.h"` in headers. Include the full
  header in the `.cpp` file instead.

**`[[nodiscard]]`:** Mark `const` methods that return a value (especially `bool`) with
`[[nodiscard]]`:
```cpp
[[nodiscard]] auto isAccountOpen(const QString &name) const -> bool;
```

**Trailing return types:** Use trailing return type syntax for all functions with
non-void return types (enforced by `modernize-use-trailing-return-type`):
```cpp
auto main(int argc, char *argv[]) -> int;
auto getCoins() -> rust::Vec<RustCoin>;
```

**Explicit nullptr comparisons:** Never rely on implicit pointer-to-bool conversion.
Use explicit comparisons (enforced by `readability-implicit-bool-conversion`):
```cpp
if (m_controller != nullptr) { ... }  // good
if (m_controller) { ... }             // bad
```

**Container emptiness:** Use `empty()` instead of `size() == 0` (enforced by
`readability-container-size-empty`):
```cpp
if (m_outputs.empty()) { ... }  // good
if (m_outputs.size() == 0) { ... }  // bad
```

**Prefer algorithms over manual loops:** Use `std::ranges::any_of`,
`std::ranges::sort`, etc. instead of hand-written loops when possible (enforced by
`readability-use-anyofallof`):
```cpp
return std::ranges::any_of(m_tabs, [&name](const auto &tab) { return tab.first == name; });
```

**Switch over enum if-else chains:** When handling `NotificationFlag` or other CXX
enums, prefer `switch` statements over long if-else chains. This reduces cognitive
complexity (enforced by `readability-function-cognitive-complexity`, threshold 25):
```cpp
switch (flag) {
case NotificationFlag::ScanProgress: { ... break; }
case NotificationFlag::NewOutput:
case NotificationFlag::OutputSpent:
    pollCoins();
    break;
default: break;
}
```

**One declaration per statement:** Don't combine multiple variable declarations
(enforced by `readability-isolate-declaration`):
```cpp
bool ok1 = false;  // good
bool ok2 = false;
bool ok1 = false, ok2 = false;  // bad
```

**No redundant base class initializers:** Don't explicitly call the default
constructor of a base class in the initializer list (enforced by
`readability-redundant-member-init`):
```cpp
ConfirmDelete(...) : m_name(name) { ... }           // good
ConfirmDelete(...) : qontrol::Modal(), m_name(name) { ... }  // bad
```

**Static member access:** Access static members through the class name, not through an
instance (enforced by `readability-static-accessed-through-instance`):
```cpp
QApplication::exec();  // good
app.exec();            // bad
```

**Unused parameters:** For parameters required by a signal-slot signature but unused
in the implementation, annotate with `[[maybe_unused]]`:
```cpp
void onScanProgress(uint32_t height, [[maybe_unused]] uint32_t tip) { ... }
```

**Qt slots and `readability-convert-member-functions-to-static`:** Qt slots must
remain non-static member functions. Suppress with `// NOLINTNEXTLINE`:
```cpp
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void sendTransaction() { ... }
```

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

**Modal subclasses** (`qontrol::Modal`) follow the same three-phase lifecycle as
screens:
1. `init()` — create all widgets
2. `doConnect()` — connect signals/slots
3. `view()` — build layout with `setMainWidget(widget)`

Always use `qontrol::Modal`, never `QDialog`. Modals are displayed via
`AppController::execModal(modal)`.

**Signal connections** use `qontrol::UNIQUE` flag to avoid duplicate connections:
```cpp
connect(source, &Source::signal, dest, &Dest::slot, qontrol::UNIQUE);
```

**Prefer named slots over lambdas in `connect()`.** Lambdas make signal-slot
connections harder to read, test, and debug. Extract the callback logic into a named
slot method instead:
```cpp
// good
connect(modal, &ConfirmDelete::confirmed, this, &AppController::onDeleteConfirmed);

// bad
connect(modal, &ConfirmDelete::confirmed, this, [this](const QString &account) { ... });
```

**Chain signals to slots for state updates.** When an action (e.g. account deletion)
needs to trigger a UI refresh, call the refresh from the slot that performs the work —
never from the caller that showed the modal. This keeps the update in the signal-slot
chain where Qt manages lifetime and ordering correctly:
```cpp
// good — slot does work then refreshes
auto AppController::onDeleteConfirmed(const QString &account) -> void {
    delete_config(rust::String(account.toStdString()));
    listAccounts(); // refresh here
}

// bad — caller refreshes after modal returns
auto AppController::deleteAccount(const QString &name) -> void {
    auto *modal = new ConfirmDelete(name);
    connect(modal, &ConfirmDelete::confirmed, this, &AppController::onDeleteConfirmed);
    AppController::execModal(modal);
    listAccounts(); // wrong: bypasses signal-slot chain
}
```
Never use `QTimer::singleShot(0, ...)` to work around signal-slot ordering issues —
restructure the chain instead.

**Clang-tidy NOLINT:** slots that clang-tidy suggests could be made static must be
suppressed with `// NOLINT` — Qt slots must remain non-static member functions.

## C++: Singleton Pattern

`AppController` inherits from `qontrol::Controller`:
```cpp
AppController::init();           // create singleton (once, fatal on duplicate)
AppController::get();            // retrieve via dynamic_cast
```

## C++: Error Handling

Never throw or catch exceptions. Rust FFI functions never return `Result` (see
"Rust: Error Handling" above), so no CXX exceptions will be thrown. Check return
values instead:
- `bool` returns: check directly (`if (!delete_config(...)) { ... }`)
- Struct returns: check `is_ok` field (`if (!result.is_ok) { ... }`)
- Opaque types: check `is_ok()` method (`if (!account->is_ok()) { ... }`)

Display errors via `qontrol::Modal` or log with `qWarning()` / `qCritical()`.

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

## C++: Utility Functions (`src/screens/utils.h`)

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

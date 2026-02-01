# Templar Project Structure

## Directory Layout

```
/
├── src/                  - C++ Qt6 GUI application
│   ├── main.cpp          - Application entry point
│   ├── AppController.h/cpp    - Global app controller (qontrol::Controller)
│   ├── MainWindow.h/cpp       - Main window with tab widget (qontrol::Window)
│   ├── AccountController.h/cpp - Per-account controller with polling timers
│   ├── AccountWidget.h/cpp     - Account UI container with side menu
│   ├── screens/               - Screen implementations (qontrol::Screen)
│   │   ├── Coins.h/cpp        - UTXO display and labeling
│   │   ├── Send.h/cpp         - SP transaction creation
│   │   ├── Receive.h/cpp      - SP address display
│   │   ├── Settings.h/cpp     - BlindBit/network configuration
│   │   ├── MenuTab.h/cpp      - Account creation menu
│   │   ├── common.h/cpp       - UI utility functions
│   │   └── modals/            - Modal dialogs
│   │       ├── CreateAccount.h/cpp
│   │       └── SelectCoins.h/cpp
│   └── widgets/               - Custom widgets (if needed)
├── templar/              - Rust backend crate (CXX FFI bridge to bwk-sp)
│   ├── Cargo.toml        - Rust dependencies (bwk-sp, cxx, serde)
│   ├── build.rs          - CXX build script
│   └── src/
│       ├── lib.rs         - CXX bridge definitions (all exported types)
│       ├── account.rs     - Account state machine wrapping bwk-sp::Account
│       └── config.rs      - Config persistence (~/.templar/<account>/)
├── lib/                  - Built Rust library outputs
│   ├── libtemplar.a      - Static library
│   └── include/
│       ├── templar.h      - Generated CXX C++ bindings
│       └── cxx.h          - CXX runtime support header
├── CMakeLists.txt        - Build configuration (Qt6, qontrol, Rust linking)
├── build.sh              - Build pipeline (cargo build → copy artifacts)
└── .cm/                  - CM project management files
```

## Module Organization

- **templar (Rust)**: CXX FFI wrapper around bwk-sp, exposed to C++
- **src (C++)**: Qt6 GUI using qontrol framework, polls Rust backend via timers

## Key Components

### Rust Backend (templar/)

- **Purpose**: Wallet logic — wraps bwk-sp::Account for config, scanning, SP addresses, transactions
- **Location**: `templar/src/`
- **Dependencies**: bwk-sp (Silent Payments wallet), cxx, serde
- **Key files**: `lib.rs` (CXX bridge), `account.rs` (bwk-sp wrapper), `config.rs` (persistence)

### C++ GUI (src/)

- **Purpose**: Desktop user interface with screens for wallet operations
- **Location**: `src/`
- **Dependencies**: Qt6 Widgets, qontrol framework
- **Key files**: `AccountController.cpp` (polling), screens/*.cpp (UI)

### Build Pipeline

- **Purpose**: Two-stage build — Rust first, then C++ linking against Rust artifacts
- **Location**: `build.sh`, `CMakeLists.txt`
- **Key files**: `build.sh` (Rust compilation), `CMakeLists.txt` (C++ compilation + linking)

## Data Flow

1. User interacts with qontrol Screen (e.g., clicks Send)
2. Screen emits Qt signal to AccountController
3. AccountController calls Rust Account method via CXX FFI
4. Account delegates to bwk-sp (transaction building, signing, broadcast via HTTP)
5. Scanner thread detects new outputs/spends via BlindBit, sends Notification via mpsc
6. AccountController polls try_recv() on QTimer, updates UI screens

## Configuration

- **~/.templar/<account>/config.json**: Per-account wallet config (mnemonic, network, BlindBit URL)
- **CMakeLists.txt**: Build configuration
- **templar/Cargo.toml**: Rust dependencies

## External Dependencies

- **bwk-sp**: Silent Payments wallet crate (account, config, coin store, tx store, scanner, signing)
- **spdk-core**: Silent Payments SDK (key management, address derivation, scanning)
- **backend-blindbit-native-non-async**: BlindBit blockchain backend (HTTP via ureq)
- **qontrol**: Qt6 C++ GUI framework (Screen/Panel/Controller/Window hierarchy)
- **cxx**: Rust ↔ C++ FFI bridge (type-safe, zero-cost)
- **Qt6**: Widget toolkit for desktop GUI

## Build Artifacts

- **lib/libtemplar.a**: Compiled Rust static library
- **lib/include/templar.h**: Generated C++ header from CXX bridge
- **build/templar**: Final linked executable

## Important Patterns

### CXX FFI Bridge

- **When to use**: Any Rust ↔ C++ communication
- **How to use**: Define types in `#[cxx::bridge]` module in lib.rs, implement in Rust, call from C++
- **Example**: Account methods exposed via `extern "Rust"` block

### Notification Polling

- **When to use**: Async events from bwk-sp scanner thread to C++ GUI
- **How to use**: bwk-sp sends Notification via mpsc channel, CXX wrapper exposes try_recv(), C++ polls on QTimer
- **Notifications**: ScanStarted, ScanProgress, ScanCompleted, NewOutput, OutputSpent, Stopped, ScanError

### qontrol Screen Lifecycle

- **When to use**: Every GUI screen
- **How to use**: Inherit qontrol::Screen, implement init() → view() → doConnect()
- **Example**: Coins screen updates in view() when NewOutput notification received

## Notes for Agents

- This is an SP-only wallet — no legacy Bitcoin addresses anywhere
- All 4 networks supported: Regtest, Signet, Testnet, Mainnet
- No CoinJoin, no Nostr, no Electrum — those are from qoinstr and not used here
- Backend is bwk-sp with BlindBit, NOT bwk with Electrum
- Reference qoinstr code for GUI/CXX patterns but replace all backend logic with bwk-sp
- The Rust crate is named "templar", not "cpp_joinstr"

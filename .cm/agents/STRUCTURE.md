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
│   │   ├── Settings.h/cpp     - Electrum/network configuration
│   │   ├── MenuTab.h/cpp      - Account creation menu
│   │   ├── common.h/cpp       - UI utility functions
│   │   └── modals/            - Modal dialogs
│   │       ├── CreateAccount.h/cpp
│   │       └── SelectCoins.h/cpp
│   └── widgets/               - Custom widgets (if needed)
├── templar/              - Rust backend crate (CXX FFI bridge to bwk)
│   ├── Cargo.toml        - Rust dependencies (bwk, cxx, serde)
│   ├── build.rs          - CXX build script
│   └── src/
│       ├── lib.rs         - CXX bridge definitions (all exported types)
│       ├── account.rs     - Account state machine wrapping bwk::Account
│       ├── config.rs      - Config persistence (~/.templar/<account>/)
│       └── address.rs     - SP address management
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

- **templar (Rust)**: Core wallet logic via bwk, exposed to C++ through CXX FFI
- **src (C++)**: Qt6 GUI using qontrol framework, polls Rust backend via timers

## Key Components

### Rust Backend (templar/)

- **Purpose**: Wallet logic — config, accounts, SP addresses, transactions, Electrum sync
- **Location**: `templar/src/`
- **Dependencies**: bwk (Bitcoin Wallet Kit), cxx, serde
- **Key files**: `lib.rs` (CXX bridge), `account.rs` (state machine), `config.rs` (persistence)

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
4. Account delegates to bwk (transaction building, signing)
5. Electrum listener thread sends notifications via mpsc channel
6. AccountController polls try_recv() every 100ms, updates UI screens

## Configuration

- **~/.templar/<account>/config.json**: Per-account wallet config (mnemonic, network, electrum URL)
- **CMakeLists.txt**: Build configuration
- **templar/Cargo.toml**: Rust dependencies

## External Dependencies

- **bwk**: Bitcoin Wallet Kit — modular Rust wallet library (keys, electrum, tx, sign, sp)
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

### Signal/Poll Notification

- **When to use**: Async events from Rust threads (Electrum sync) to C++ GUI
- **How to use**: Rust sends Signal via mpsc channel, C++ polls try_recv() on QTimer
- **Example**: CoinUpdate signal triggers Coins screen refresh

### qontrol Screen Lifecycle

- **When to use**: Every GUI screen
- **How to use**: Inherit qontrol::Screen, implement init() → view() → doConnect()
- **Example**: Coins screen updates in view() when coinUpdate signal received

## Notes for Agents

- This is an SP-only wallet — no legacy Bitcoin addresses anywhere
- All 4 networks supported: Regtest, Signet, Testnet, Mainnet
- No CoinJoin or Nostr — those are from qoinstr and not used here
- Reference qoinstr code for patterns but replace joinstr/pool logic with bwk/SP
- The Rust crate is named "templar", not "cpp_joinstr"

# Templar

> Privacy-focused desktop Bitcoin wallet using Silent Payments protocol

## Overview

Templar is a desktop Bitcoin wallet that provides financial privacy through the Silent Payments protocol. It is modeled on the qoinstr architecture — a C++ Qt6 GUI frontend communicating with a Rust backend via CXX FFI — but replaces qoinstr's CoinJoin/Nostr privacy model with Silent Payments via the bwk (Bitcoin Wallet Kit) crate.

The wallet supports all Bitcoin networks (Regtest, Signet, Testnet, Mainnet) and operates exclusively with Silent Payment addresses for both sending and receiving. The GUI is built with the qontrol framework (a Qt6 widget abstraction layer), and the backend leverages the bwk-sp crate — a self-contained Silent Payments wallet implementation using BlindBit as its blockchain backend.

## Goals

- Generate and manage Silent Payment addresses for receiving
- Send BTC to Silent Payment addresses with coin selection
- Sync wallet state via BlindBit backend
- Support all Bitcoin networks (Regtest, Signet, Testnet, Mainnet)
- Multi-account wallet management
- UTXO labeling and coin management

## Success Criteria

- [ ] Can create and restore an SP wallet from a BIP39 mnemonic
- [ ] Can receive Bitcoin to a silent payment address
- [ ] Can send BTC with manual coin selection
- [ ] BlindBit sync works on all supported networks

## Architecture

```
Qt Main Thread (C++ GUI)
    |  (CXX FFI bridge)
    v
Account (Rust, wrapping bwk-sp::Account)
    ^  (mpsc::channel)
    |
Scanner Thread (Rust, background)
    ^  (HTTP via ureq)
    |
BlindBit Server
```

The bwk-sp Account uses a background scanner thread that polls the BlindBit server every 5 seconds for new blocks. It sends `Notification` events (ScanStarted, ScanProgress, ScanCompleted, NewOutput, OutputSpent) via mpsc channel. The C++ GUI polls the Rust wrapper via a CXX-exposed `try_recv()` method on a QTimer.

### Components

1. **`templar/` Rust crate** - CXX FFI bridge wrapping bwk-sp. Exposes Account, Config, CoinState, transaction types, and Notification polling system to C++.
2. **`src/` C++ GUI** - Qt6 application using qontrol framework. Contains AppController, MainWindow, AccountController, and screen implementations (Coins, Send, Receive, Settings).
3. **CMake build system** - Fetches qontrol via FetchContent, links against prebuilt Rust static library, manages the two-language build pipeline.

### Data Flow

1. User interacts with qontrol Screen (e.g., Send screen)
2. Screen emits Qt signal to AccountController
3. AccountController calls Rust Account method via CXX FFI
4. Account delegates to bwk-sp (tx building, signing, broadcast via ureq HTTP)
5. Scanner thread detects new outputs/spends via BlindBit, sends Notification via mpsc
6. AccountController polls `try_recv()` on timer, updates UI screens

## Modules

### Config

**Purpose:** Wallet configuration persistence (mnemonic, network, BlindBit URL)

**Key files:**
- `templar/src/config.rs` - Config struct, file I/O, validation

**Dependencies:** bwk-sp::Config, serde_json, dirs

### Account

**Purpose:** Main wallet state machine wrapping bwk-sp::Account with CXX-exposed interface

**Key files:**
- `templar/src/account.rs` - Account struct, try_recv polling, CXX methods
- `templar/src/lib.rs` - CXX bridge definitions

**Dependencies:** bwk-sp::Account, SpCoinStore, SpTxStore, SpLabelStore

### CoinStore

**Purpose:** UTXO tracking, balance calculation, coin status management

**Key files:**
- Delegated to bwk-sp's SpCoinStore (BTreeMap<OutPoint, SpCoinEntry>)

### SP Address Management

**Purpose:** Silent Payment address generation via SpClient

**Key files:**
- Managed by bwk-sp::Account via spdk-core's SpClient

**Dependencies:** bwk-sp, spdk-core, silentpayments

### Transaction Builder

**Purpose:** SP transaction creation, coin selection, fee estimation, broadcast via ureq HTTP

**Key files:**
- Managed by bwk-sp::Account transaction methods

**Dependencies:** bwk-sp (uses spdk-core SilentPaymentUnsignedTransaction, ureq for broadcast)

### Scanner

**Purpose:** Background thread for blockchain sync via BlindBit backend

**Key files:**
- Managed by bwk-sp::Account start_scanner/stop_scanner methods
- Notifications via mpsc channel: ScanStarted, ScanProgress, ScanCompleted, NewOutput, OutputSpent

**Dependencies:** bwk-sp (uses backend-blindbit-native-non-async, UreqClient)

## Phases

### Phase 1: Scaffolding

**Goal:** Set up the project skeleton — CMake, qontrol, Rust crate with CXX bridge

**Tasks:**
- Set up CMake project with Qt6 and qontrol FetchContent
- Create `templar/` Rust crate with CXX bridge skeleton and bwk dependency
- Create build pipeline script (build.sh)

**Deliverables:**
- CMakeLists.txt that builds an empty Qt6 window
- templar/ Rust crate that compiles with CXX bridge
- build.sh that compiles Rust and copies artifacts

### Phase 2: Config & Account

**Goal:** Implement the foundational Rust backend — configuration persistence and Account state machine with CXX FFI bridge

**Tasks:**
- Config module wrapping bwk-sp::Config with mnemonic, network, BlindBit URL, file persistence (~/.templar/<account>/config.json)
- Account struct wrapping bwk-sp::Account with CXX-exposed lifecycle methods (new, start_scanner, stop_scanner)
- Notification polling: wrap bwk-sp's mpsc Receiver into CXX-compatible try_recv returning Poll<Notification>
- CXX bridge expansion: Config, Account, Network enum, LogLevel, Notification types

**Deliverables:**
- Config can be created, saved to disk, and loaded from disk
- Account can be instantiated from config with CXX-exposed methods
- Notification channel ready for GUI polling (ScanStarted, ScanProgress, ScanCompleted, NewOutput, OutputSpent, Stopped, ScanError)

### Phase 3: SP Addresses & BlindBit Sync

**Goal:** Add Silent Payment address management and BlindBit blockchain scanning to the Account

**Tasks:**
- SP address generation via bwk-sp's SpClient (receive address from mnemonic)
- CXX-exposed SP address display (the wallet's static SP address)
- BlindBit scanner integration: expose start_scanner/stop_scanner via CXX
- Coin tracking: wrap bwk-sp's SpCoinStore, expose coin list and balances via CXX
- CXX-exposed types: RustCoin (outpoint, value, height, label), CoinState (balances)
- Payment history: wrap bwk-sp's SpTxStore, expose transaction list via CXX

**Deliverables:**
- SP receive address can be displayed
- BlindBit scanner syncs UTXOs and sends Notifications through mpsc channel
- Coin state and payment history available via CXX

### Phase 4: Transaction Types & Simulation

**Goal:** Define CXX-exposed transaction data types and implement transaction simulation

**Tasks:**
- TransactionTemplate struct (inputs, outputs, fee_rate) exposed via CXX
- Output struct (SP address, amount, label, max flag)
- TransactionSimulation struct (is_valid, fee, weight, error)
- CoinState struct (confirmed/unconfirmed counts and balances)
- simulate_transaction() method on Account
- PsbtResult wrapper type for CXX error handling

**Deliverables:**
- All transaction-related types available to C++ via CXX bridge
- Transaction simulation returns fee estimates and validation results

### Phase 5: Coin Selection, Signing & Broadcast

**Goal:** Complete the send flow — coin selection, PSBT creation with SP outputs, signing, and broadcast

**Tasks:**
- Coin selection from spendable SP UTXOs (manual and automatic)
- Transaction creation via bwk-sp's SilentPaymentUnsignedTransaction
- Change output handling
- Transaction signing via bwk-sp (uses spdk-core signing)
- Transaction broadcast via ureq HTTP POST
- Notification on success

**Deliverables:**
- Complete send flow: select coins → build SP transaction → sign → broadcast
- Error handling for insufficient funds, invalid addresses, broadcast failures

### Phase 6: GUI Controllers

**Goal:** Build the Qt6 application infrastructure — controllers, main window, account widget with polling

**Tasks:**
- AppController (qontrol::Controller) managing account lifecycle
- MainWindow (qontrol::Window) with QTabWidget for account tabs
- AccountController with QTimer polling (100ms signals, 1000ms coins)
- AccountWidget with side menu (qontrol::Column) and screen container
- MenuTab screen for the "+" tab (account creation entry point)
- CreateAccount modal (mnemonic generation/restore, network selector)
- Update src/main.cpp with full application startup
- Update CMakeLists.txt with all new source files

**Deliverables:**
- Application launches with tabbed account management
- AccountController polls Rust backend and emits Qt signals
- New accounts can be created via modal dialog

### Phase 7: Wallet Screens

**Goal:** Implement all wallet screens — Coins, Receive, Send, Settings

**Tasks:**
- Coins screen: UTXO table (outpoint, value, confirmations, status, label), balance display, coin labeling
- Receive screen: SP address display (large, copyable), "Copy" button, "New Address" button, address list
- Send screen: recipient SP address input, amount, fee rate, "Max" checkbox, coin selection via SelectCoins modal, simulate button, send button
- SelectCoins modal: available UTXOs with checkboxes, selected total display
- Settings screen: BlindBit URL input, network selector (all 4 networks), save/connect/disconnect buttons
- Common UI utilities (BTC formatting, standard layouts)

**Deliverables:**
- All 4 screens fully functional and connected to AccountController signals
- Send screen can simulate and execute SP transactions
- Settings screen persists config changes

### Phase 8: Integration & Testing

**Goal:** End-to-end testing and hardening across all networks

**Tasks:**
- End-to-end integration testing on regtest (send/receive SP flow)
- Testing on signet (SP address generation, sync, transactions)
- Wallet restore from mnemonic (verify funds recovery)
- Multi-account testing (independent state, different networks)
- Error handling: invalid BlindBit URL, invalid mnemonic, insufficient funds, invalid SP address, connection loss
- Edge cases: zero balance send, dust outputs, address reuse detection

**Deliverables:**
- Tested wallet on regtest and signet
- Robust error handling (no panics on any error path)
- Multi-account isolation verified

## Technical Decisions

### Silent Payments Only

**Context:** qoinstr used CoinJoin via Nostr for privacy. Templar needs a different privacy model.
**Decision:** Use Silent Payments protocol exclusively — no legacy address support.
**Rationale:** SP provides receiver privacy without coordination. Simpler architecture (no Nostr dependency). bwk already implements SP via the sp crate.

### bwk-sp as Backend

**Context:** qoinstr used cpp_joinstr (custom Rust crate wrapping joinstr) with Electrum for sync.
**Decision:** Use bwk-sp, a self-contained Silent Payments wallet crate that uses BlindBit as its blockchain backend.
**Rationale:** bwk-sp provides a complete SP wallet implementation (account, config, coin store, tx store, scanner, signing, broadcast) with BlindBit backend. No Electrum dependency needed.

### CXX FFI Bridge

**Context:** Need to connect Rust backend to C++ GUI.
**Decision:** Use CXX crate (same as qoinstr).
**Rationale:** Proven approach from qoinstr. Type-safe, zero-cost FFI. Generates C++ headers automatically.

## Out of Scope

- CoinJoin / Nostr integration
- Legacy Bitcoin addresses (P2PKH, P2SH, P2WPKH, P2TR without SP)
- Hardware wallet / cold signing support
- Mobile platforms
- Lightning Network

## References

- qoinstr: `../qoinstr/` — Architecture reference
- bwk: `github.com/pythcoiner/bwk` — Rust wallet backend
- qontrol: `github.com/pythcoiner/qontrol` — Qt6 GUI framework
- Silent Payments: BIP-352

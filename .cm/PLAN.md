# Templar

> Privacy-focused desktop Bitcoin wallet using Silent Payments protocol

## Overview

Templar is a desktop Bitcoin wallet that provides financial privacy through the Silent Payments protocol. It is modeled on the qoinstr architecture — a C++ Qt6 GUI frontend communicating with a Rust backend via CXX FFI — but replaces qoinstr's CoinJoin/Nostr privacy model with Silent Payments via the bwk (Bitcoin Wallet Kit) crate.

The wallet supports all Bitcoin networks (Regtest, Signet, Testnet, Mainnet) and operates exclusively with Silent Payment addresses for both sending and receiving. The GUI is built with the qontrol framework (a Qt6 widget abstraction layer), and the backend leverages bwk's modular crates for key management, Electrum sync, transaction building, and signing.

## Goals

- Generate and manage Silent Payment addresses for receiving
- Send BTC to Silent Payment addresses with coin selection
- Sync wallet state via Electrum protocol
- Support all Bitcoin networks (Regtest, Signet, Testnet, Mainnet)
- Multi-account wallet management
- UTXO labeling and coin management

## Success Criteria

- [ ] Can create and restore an SP wallet from a BIP39 mnemonic
- [ ] Can receive Bitcoin to a silent payment address
- [ ] Can send BTC with manual coin selection
- [ ] Electrum sync works on all supported networks

## Architecture

```
Qt Main Thread (C++ GUI)
    |  (CXX FFI bridge)
    v
Account (Rust, main thread)
    ^  (mpsc::channel)
    |
Electrum Listener Thread (Rust)
    ^  (TCP/WebSocket)
    |
Electrum Server
```

The C++ GUI polls the Rust backend every 100ms via `Account::try_recv()` which returns `Poll<Signal>` notifications. This event-driven polling model is inherited from qoinstr's architecture.

### Components

1. **`templar/` Rust crate** - CXX FFI bridge wrapping bwk. Exposes Account, Config, CoinState, transaction types, and Signal/Poll notification system to C++.
2. **`src/` C++ GUI** - Qt6 application using qontrol framework. Contains AppController, MainWindow, AccountController, and screen implementations (Coins, Send, Receive, Settings).
3. **CMake build system** - Fetches qontrol via FetchContent, links against prebuilt Rust static library, manages the two-language build pipeline.

### Data Flow

1. User interacts with qontrol Screen (e.g., Send screen)
2. Screen emits Qt signal to AccountController
3. AccountController calls Rust Account method via CXX FFI
4. Account delegates to bwk (tx building, signing, broadcast)
5. Electrum listener thread receives confirmations via mpsc channel
6. AccountController polls `try_recv()` on timer, updates UI

## Modules

### Config

**Purpose:** Wallet configuration persistence (mnemonic, network, Electrum URL, SP descriptor)

**Key files:**
- `templar/src/config.rs` - Config struct, file I/O, validation

**Dependencies:** bwk::Config, serde_json, dirs

### Account

**Purpose:** Main wallet state machine wrapping bwk::Account with CXX-exposed interface

**Key files:**
- `templar/src/account.rs` - Account struct, try_recv polling, CXX methods
- `templar/src/lib.rs` - CXX bridge definitions

**Dependencies:** bwk::Account, CoinStore, AddressStore, SigningManager

### CoinStore

**Purpose:** UTXO tracking, balance calculation, coin status management

**Key files:**
- Delegated to bwk's internal coin management

### SP Address Management

**Purpose:** Silent Payment address generation and tracking for receive/change

**Key files:**
- `templar/src/address.rs` - SP address derivation, address store wrapper

**Dependencies:** bwk-sp, silentpayments

### Transaction Builder

**Purpose:** PSBT creation with SP outputs, coin selection, fee estimation

**Key files:**
- `templar/src/tx.rs` - Transaction template types, simulation, PSBT creation

**Dependencies:** bwk_tx, bwk_sign

### Electrum Listener

**Purpose:** Background thread for blockchain sync via Electrum protocol

**Key files:**
- Managed by bwk_electrum, exposed through Account start/stop methods

**Dependencies:** bwk_electrum

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

### Phase 2: Core Backend

**Goal:** Implement the Rust backend — config, account, SP addresses, Electrum sync

**Tasks:**
- Config module with mnemonic, network, Electrum URL, SP descriptor, file persistence
- Account struct wrapping bwk::Account with CXX-exposed lifecycle methods
- SP address generation via bwk-sp with address store
- Electrum sync integration with Signal/Poll notification channel

**Deliverables:**
- Working Rust backend that can create accounts, generate SP addresses, sync via Electrum
- Full CXX bridge with all types exposed to C++

### Phase 3: Transaction Flow

**Goal:** Enable sending and receiving — transaction types, coin selection, signing, broadcast

**Tasks:**
- CXX-exposed transaction types (TransactionTemplate, TransactionSimulation, CoinState)
- Coin selection and PSBT creation with SP outputs
- Transaction signing and Electrum broadcast

**Deliverables:**
- Complete send flow: select coins -> build PSBT -> sign -> broadcast
- Fee estimation and transaction simulation

### Phase 4: GUI Screens

**Goal:** Build the Qt6 GUI with all wallet screens

**Tasks:**
- AppController + MainWindow + AccountController with timer-based polling
- Coins screen (UTXO display, labeling) + Receive screen (SP address display)
- Send screen (SP send, fee estimation, coin selection) + Settings screen (config editor)

**Deliverables:**
- Fully functional desktop wallet GUI
- All screens connected to Rust backend via AccountController

### Phase 5: Integration & Testing

**Goal:** End-to-end testing and hardening

**Tasks:**
- End-to-end integration testing on regtest/signet
- Multi-account testing, error handling, edge cases

**Deliverables:**
- Tested wallet on regtest and signet
- Robust error handling

## Technical Decisions

### Silent Payments Only

**Context:** qoinstr used CoinJoin via Nostr for privacy. Templar needs a different privacy model.
**Decision:** Use Silent Payments protocol exclusively — no legacy address support.
**Rationale:** SP provides receiver privacy without coordination. Simpler architecture (no Nostr dependency). bwk already implements SP via the sp crate.

### bwk as Backend

**Context:** qoinstr used cpp_joinstr (custom Rust crate wrapping joinstr).
**Decision:** Use bwk (Bitcoin Wallet Kit) which is the structural successor to cpp_joinstr.
**Rationale:** bwk is modular (keys, electrum, tx, sign, descriptor as separate crates), includes SP support, and is actively maintained.

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

# Templar - Roadmap

This document tracks implementation progress. Check off items as they are completed.

## Phase 1: Scaffolding

Status: **Complete** (3/3)

- [x] Set up CMake project with Qt6 and qontrol
  - [x] CMakeLists.txt with Qt6 Widgets dependency
  - [x] qontrol fetched via FetchContent
  - [x] Empty Qt window compiles and runs
- [x] Create templar Rust crate with CXX bridge skeleton
  - [x] Cargo.toml with bwk-sp and cxx dependencies
  - [x] lib.rs with cxx::bridge module skeleton
  - [x] build.rs with cxx_build
- [x] Create build pipeline script
  - [x] build.sh runs cargo build --release
  - [x] Copies .a library and generated headers to lib/
  - [x] CMake links against built library

---

## Phase 2: Config & Account

Status: **Complete** (4/4)

- [x] Config module wrapping bwk-sp::Config
  - [x] Config struct with mnemonic, network, BlindBit URL
  - [x] JSON file persistence (~/.templar/<account>/)
  - [x] CXX-exposed config creation and loading
- [x] Account struct wrapping bwk-sp::Account
  - [x] Account struct with bwk-sp::Account inner
  - [x] CXX-exposed lifecycle methods (new, start_scanner, stop_scanner)
- [x] Notification polling via CXX try_recv
  - [x] Wrap bwk-sp mpsc Receiver
  - [x] CXX-compatible Poll<Notification> return type
- [x] CXX bridge expansion with all types
  - [x] Config, Account opaque types
  - [x] Network enum, LogLevel, Notification types

---

## Phase 3: SP Addresses & BlindBit Sync

Status: **Complete** (3/3)

- [x] SP address generation and display
  - [x] SP receive address from mnemonic via SpClient
  - [x] CXX-exposed address display
- [x] BlindBit scanner and coin tracking
  - [x] start_scanner/stop_scanner exposed via CXX
  - [x] SpCoinStore wrapped with coin list and balances
  - [x] RustCoin and CoinState CXX types
- [x] Payment history via CXX
  - [x] SpTxStore wrapped for transaction list
  - [x] Transaction history exposed via CXX

---

## Phase 4: Transaction Types & Simulation

Status: **Complete** (2/2)

- [x] CXX transaction type definitions
  - [x] TransactionTemplate struct (inputs, outputs, fee_rate)
  - [x] Output struct (SP address, amount, label, max flag)
  - [x] TransactionSimulation struct (is_valid, fee, weight, error)
- [x] Transaction simulation and result wrappers
  - [x] simulate_transaction() method on Account
  - [x] PsbtResult wrapper type for CXX error handling

---

## Phase 5: Coin Selection, Signing & Broadcast

Status: **Complete** (2/2)

- [x] Coin selection and SP transaction creation
  - [x] Manual and automatic coin selection
  - [x] SilentPaymentUnsignedTransaction via bwk-sp
  - [x] Change output handling
- [x] Transaction signing and broadcast
  - [x] Signing via bwk-sp (spdk-core)
  - [x] Broadcast via ureq HTTP POST
  - [x] Success notification

---

## Phase 6: GUI Controllers

Status: **Complete** (3/3)

- [x] AppController, MainWindow, AccountController
  - [x] AppController (qontrol::Controller) with account management
  - [x] MainWindow (qontrol::Window) with tab widget
  - [x] AccountController with QTimer polling
- [x] AccountWidget, MenuTab, CreateAccount modal
  - [x] AccountWidget with side menu and screen container
  - [x] MenuTab for account creation entry
  - [x] CreateAccount modal (mnemonic gen/restore, network)
- [x] main.cpp and CMakeLists.txt updates
  - [x] Full application startup in main.cpp
  - [x] All source files added to CMakeLists.txt

---

## Phase 7: Wallet Screens

Status: **Not Started** (0/3)

- [ ] Coins screen and Receive screen
  - [ ] Coins: UTXO table, balance display, labels
  - [ ] Receive: SP address display, copy, address list
- [ ] Send screen with SelectCoins modal
  - [ ] Send: SP address input, amount, fee rate, simulate, send
  - [ ] SelectCoins: UTXO checkboxes, selected total
- [ ] Settings screen and common utilities
  - [ ] Settings: BlindBit URL, network selector, save/connect
  - [ ] Common: BTC formatting, standard layouts

---

## Phase 8: Integration & Testing

Status: **Not Started** (0/2)

- [ ] End-to-end integration testing
  - [ ] Send/receive SP flow on regtest
  - [ ] SP address generation and sync on signet
  - [ ] Wallet restore from mnemonic
- [ ] Multi-account and error handling
  - [ ] Multiple accounts with independent state
  - [ ] Invalid BlindBit URL, mnemonic, SP address handling
  - [ ] Edge cases: zero balance, dust outputs

---

## Summary

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: Scaffolding | Complete | 12/12 |
| Phase 2: Config & Account | Complete | 13/13 |
| Phase 3: SP Addresses & BlindBit Sync | Complete | 10/10 |
| Phase 4: Transaction Types & Simulation | Complete | 7/7 |
| Phase 5: Coin Selection, Signing & Broadcast | Complete | 8/8 |
| Phase 6: GUI Controllers | Complete | 11/11 |
| Phase 7: Wallet Screens | Not Started | 0/9 |
| Phase 8: Integration & Testing | Not Started | 0/8 |
| **Total** | | **61/78** |

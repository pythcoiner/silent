# Templar - Roadmap

This document tracks implementation progress. Check off items as they are completed.

## Phase 1: Scaffolding

Status: **Not Started** (0/3)

- [ ] Set up CMake project with Qt6 and qontrol
  - [ ] CMakeLists.txt with Qt6 Widgets dependency
  - [ ] qontrol fetched via FetchContent
  - [ ] Empty Qt window compiles and runs
- [ ] Create templar Rust crate with CXX bridge skeleton
  - [ ] Cargo.toml with bwk-sp and cxx dependencies
  - [ ] lib.rs with cxx::bridge module skeleton
  - [ ] build.rs with cxx_build
- [ ] Create build pipeline script
  - [ ] build.sh runs cargo build --release
  - [ ] Copies .a library and generated headers to lib/
  - [ ] CMake links against built library

---

## Phase 2: Config & Account

Status: **Not Started** (0/4)

- [ ] Config module wrapping bwk-sp::Config
  - [ ] Config struct with mnemonic, network, BlindBit URL
  - [ ] JSON file persistence (~/.templar/<account>/)
  - [ ] CXX-exposed config creation and loading
- [ ] Account struct wrapping bwk-sp::Account
  - [ ] Account struct with bwk-sp::Account inner
  - [ ] CXX-exposed lifecycle methods (new, start_scanner, stop_scanner)
- [ ] Notification polling via CXX try_recv
  - [ ] Wrap bwk-sp mpsc Receiver
  - [ ] CXX-compatible Poll<Notification> return type
- [ ] CXX bridge expansion with all types
  - [ ] Config, Account opaque types
  - [ ] Network enum, LogLevel, Notification types

---

## Phase 3: SP Addresses & BlindBit Sync

Status: **Not Started** (0/3)

- [ ] SP address generation and display
  - [ ] SP receive address from mnemonic via SpClient
  - [ ] CXX-exposed address display
- [ ] BlindBit scanner and coin tracking
  - [ ] start_scanner/stop_scanner exposed via CXX
  - [ ] SpCoinStore wrapped with coin list and balances
  - [ ] RustCoin and CoinState CXX types
- [ ] Payment history via CXX
  - [ ] SpTxStore wrapped for transaction list
  - [ ] Transaction history exposed via CXX

---

## Phase 4: Transaction Types & Simulation

Status: **Not Started** (0/2)

- [ ] CXX transaction type definitions
  - [ ] TransactionTemplate struct (inputs, outputs, fee_rate)
  - [ ] Output struct (SP address, amount, label, max flag)
  - [ ] TransactionSimulation struct (is_valid, fee, weight, error)
- [ ] Transaction simulation and result wrappers
  - [ ] simulate_transaction() method on Account
  - [ ] PsbtResult wrapper type for CXX error handling

---

## Phase 5: Coin Selection, Signing & Broadcast

Status: **Not Started** (0/2)

- [ ] Coin selection and SP transaction creation
  - [ ] Manual and automatic coin selection
  - [ ] SilentPaymentUnsignedTransaction via bwk-sp
  - [ ] Change output handling
- [ ] Transaction signing and broadcast
  - [ ] Signing via bwk-sp (spdk-core)
  - [ ] Broadcast via ureq HTTP POST
  - [ ] Success notification

---

## Phase 6: GUI Controllers

Status: **Not Started** (0/3)

- [ ] AppController, MainWindow, AccountController
  - [ ] AppController (qontrol::Controller) with account management
  - [ ] MainWindow (qontrol::Window) with tab widget
  - [ ] AccountController with QTimer polling
- [ ] AccountWidget, MenuTab, CreateAccount modal
  - [ ] AccountWidget with side menu and screen container
  - [ ] MenuTab for account creation entry
  - [ ] CreateAccount modal (mnemonic gen/restore, network)
- [ ] main.cpp and CMakeLists.txt updates
  - [ ] Full application startup in main.cpp
  - [ ] All source files added to CMakeLists.txt

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
| Phase 1: Scaffolding | Not Started | 0/12 |
| Phase 2: Config & Account | Not Started | 0/13 |
| Phase 3: SP Addresses & BlindBit Sync | Not Started | 0/10 |
| Phase 4: Transaction Types & Simulation | Not Started | 0/7 |
| Phase 5: Coin Selection, Signing & Broadcast | Not Started | 0/8 |
| Phase 6: GUI Controllers | Not Started | 0/11 |
| Phase 7: Wallet Screens | Not Started | 0/9 |
| Phase 8: Integration & Testing | Not Started | 0/8 |
| **Total** | | **0/78** |

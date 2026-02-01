# Templar - Roadmap

This document tracks implementation progress. Check off items as they are completed.

## Phase 1: Scaffolding

Status: **Not Started** (0/3)

- [ ] Set up CMake project with Qt6 and qontrol FetchContent
  - [ ] CMakeLists.txt with Qt6 Widgets dependency
  - [ ] qontrol fetched via FetchContent
  - [ ] Empty Qt window compiles and runs
- [ ] Create templar Rust crate with CXX bridge skeleton
  - [ ] Cargo.toml with bwk and cxx dependencies
  - [ ] lib.rs with cxx::bridge module skeleton
  - [ ] build.rs with cxx_build
- [ ] Create build pipeline script
  - [ ] build.sh runs cargo build --release
  - [ ] Copies .a library and generated headers to lib/
  - [ ] CMake links against built library

---

## Phase 2: Core Backend

Status: **Not Started** (0/4)

- [ ] Config module with persistence
  - [ ] Config struct with mnemonic, network, electrum URL, SP descriptor
  - [ ] JSON file persistence (~/.templar/<account>/config.json)
  - [ ] CXX-exposed config creation and loading
- [ ] Account struct wrapping bwk::Account
  - [ ] Account struct with bwk::Account inner
  - [ ] CXX-exposed lifecycle methods (new, start, stop)
  - [ ] Signal/Poll notification types exposed to C++
- [ ] SP address generation
  - [ ] SP receive address derivation via bwk-sp
  - [ ] Address store with status tracking
  - [ ] CXX-exposed address list and generation
- [ ] Electrum sync integration
  - [ ] Electrum listener thread via bwk_electrum
  - [ ] mpsc channel for notifications
  - [ ] Signal types (TxListenerStarted, CoinUpdate, etc.)

---

## Phase 3: Transaction Flow

Status: **Not Started** (0/3)

- [ ] CXX-exposed transaction types
  - [ ] TransactionTemplate (inputs, outputs, fee rate)
  - [ ] TransactionSimulation (fee estimation, validation)
  - [ ] CoinState (confirmed/unconfirmed balances)
- [ ] Coin selection and PSBT creation
  - [ ] Coin selection from SP UTXOs
  - [ ] PSBT creation with SP outputs
  - [ ] Fee estimation modes
- [ ] Transaction signing and broadcast
  - [ ] PSBT signing via bwk_sign (hot signer from mnemonic)
  - [ ] Broadcast via Electrum
  - [ ] SignedTx signal notification

---

## Phase 4: GUI Screens

Status: **Not Started** (0/3)

- [ ] AppController, MainWindow, and AccountController
  - [ ] AppController (qontrol::Controller) with account management
  - [ ] MainWindow (qontrol::Window) with tab widget
  - [ ] AccountController with 100ms polling timer
  - [ ] AccountWidget with side menu and screen container
- [ ] Coins screen and Receive screen
  - [ ] Coins screen: UTXO table, balance display, labels
  - [ ] Receive screen: SP address display and copy
- [ ] Send screen and Settings screen
  - [ ] Send screen: SP address input, amount, fee rate, coin selection
  - [ ] Settings screen: Electrum URL/port, network selector

---

## Phase 5: Integration and Testing

Status: **Not Started** (0/2)

- [ ] End-to-end integration testing
  - [ ] Send/receive flow on regtest
  - [ ] Send/receive flow on signet
  - [ ] Wallet restore from mnemonic
- [ ] Multi-account and error handling
  - [ ] Multiple accounts with independent state
  - [ ] Electrum connection error handling
  - [ ] Edge cases (zero balance send, dust outputs)

---

## Summary

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: Scaffolding | Not Started | 0/12 |
| Phase 2: Core Backend | Not Started | 0/16 |
| Phase 3: Transaction Flow | Not Started | 0/12 |
| Phase 4: GUI Screens | Not Started | 0/11 |
| Phase 5: Integration and Testing | Not Started | 0/8 |
| **Total** | | **0/59** |

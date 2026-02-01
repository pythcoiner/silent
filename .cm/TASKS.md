# templar - Tasks

This document shows phase plans and task status. Generated from tasks.json.

## phase-1: Scaffolding

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-1.task-1**: Set up CMake project with Qt6 and qontrol FetchContent - # Phase 1: Scaffolding
- [ ] **phase-1.task-2**: Create templar Rust crate with CXX bridge skeleton and bwk-sp dependency - # Phase 1: Scaffolding
- [ ] **phase-1.task-3**: Create build pipeline script (build.sh: cargo build, copy libs and headers) - # Phase 1: Scaffolding

---

## phase-2: Config & Account

**Status:** Pending (0/4)

### Tasks

- [ ] **phase-2.task-1**: Implement Config module wrapping bwk-sp::Config with mnemonic, network, BlindBit URL, file persistence - # Phase 2: Config & Account
- [ ] **phase-2.task-2**: Implement Account struct wrapping bwk-sp::Account with CXX-exposed lifecycle methods - # Phase 2: Config & Account
- [ ] **phase-2.task-3**: Implement Notification polling: wrap bwk-sp mpsc Receiver into CXX-compatible try_recv - # Phase 2: Config & Account
- [ ] **phase-2.task-4**: Expand CXX bridge with Config, Account, Network enum, LogLevel, Notification types - # Phase 2: Config & Account

---

## phase-3: SP Addresses & BlindBit Sync

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-3.task-1**: Expose SP address generation and display via CXX - # Phase 3: SP Addresses & BlindBit Sync
- [ ] **phase-3.task-2**: Expose BlindBit scanner start/stop and coin tracking via CXX - # Phase 3: SP Addresses & BlindBit Sync
- [ ] **phase-3.task-3**: Expose payment history and CXX types (RustCoin, CoinState) - # Phase 3: SP Addresses & BlindBit Sync

---

## phase-4: Transaction Types & Simulation

**Status:** Pending (0/2)

### Tasks

- [ ] **phase-4.task-1**: Define CXX-exposed TransactionTemplate, Output, TransactionSimulation structs - # Phase 4: Transaction Types & Simulation
- [ ] **phase-4.task-2**: Implement simulate_transaction method and PsbtResult wrapper - # Phase 4: Transaction Types & Simulation

---

## phase-5: Coin Selection, Signing & Broadcast

**Status:** Pending (0/2)

### Tasks

- [ ] **phase-5.task-1**: Implement coin selection and SP transaction creation via bwk-sp - # Phase 5: Coin Selection, Signing & Broadcast
- [ ] **phase-5.task-2**: Implement transaction signing and broadcast via ureq HTTP - # Phase 5: Coin Selection, Signing & Broadcast

---

## phase-6: GUI Controllers

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-6.task-1**: Implement AppController, MainWindow, and AccountController with timer-based polling - # Phase 6: GUI Controllers
- [ ] **phase-6.task-2**: Implement AccountWidget, MenuTab, and CreateAccount modal - # Phase 6: GUI Controllers
- [ ] **phase-6.task-3**: Update main.cpp and CMakeLists.txt for full application startup - # Phase 6: GUI Controllers

---

## phase-7: Wallet Screens

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-7.task-1**: Implement Coins screen and Receive screen - # Phase 7: Wallet Screens
- [ ] **phase-7.task-2**: Implement Send screen with SelectCoins modal - # Phase 7: Wallet Screens
- [ ] **phase-7.task-3**: Implement Settings screen and common UI utilities - # Phase 7: Wallet Screens

---

## phase-8: Integration & Testing

**Status:** Pending (0/2)

### Tasks

- [ ] **phase-8.task-1**: End-to-end integration testing on regtest and signet - # Phase 8: Integration & Testing
- [ ] **phase-8.task-2**: Multi-account testing, error handling, and edge cases - # Phase 8: Integration & Testing

---

## Summary

| Phase | Status | Tasks | Completed |
|-------|--------|-------|----------:|
| phase-1: Scaffolding | Pending | 3 | 0 |
| phase-2: Config & Account | Pending | 4 | 0 |
| phase-3: SP Addresses & BlindBit Sync | Pending | 3 | 0 |
| phase-4: Transaction Types & Simulation | Pending | 2 | 0 |
| phase-5: Coin Selection, Signing & B... | Pending | 2 | 0 |
| phase-6: GUI Controllers | Pending | 3 | 0 |
| phase-7: Wallet Screens | Pending | 3 | 0 |
| phase-8: Integration & Testing | Pending | 2 | 0 |
| **Total** | | **22** | **0** |

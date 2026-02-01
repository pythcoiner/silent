# templar - Tasks

This document shows phase plans and task status. Generated from tasks.json.

## phase-1: Scaffolding

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-1.task-1**: Set up CMake project with Qt6 and qontrol FetchContent - # Phase 1: Scaffolding
- [ ] **phase-1.task-2**: Create templar Rust crate with CXX bridge skeleton and bwk dependency - # Phase 1: Scaffolding
- [ ] **phase-1.task-3**: Create build pipeline script (build.sh: cargo build, copy libs and headers) - # Phase 1: Scaffolding

---

## phase-2: Core Backend

**Status:** Pending (0/4)

### Tasks

- [ ] **phase-2.task-1**: Implement Config module with mnemonic, network, Electrum URL, SP descriptor, file persistence - # Phase 2: Core Backend
- [ ] **phase-2.task-2**: Implement Account struct wrapping bwk::Account with CXX-exposed lifecycle methods - # Phase 2: Core Backend
- [ ] **phase-2.task-3**: Implement SP address generation via bwk-sp with address store - # Phase 2: Core Backend
- [ ] **phase-2.task-4**: Integrate Electrum sync with Signal/Poll notification channel - # Phase 2: Core Backend

---

## phase-3: Transaction Flow

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-3.task-1**: Define CXX-exposed transaction types (TransactionTemplate, TransactionSimulation, CoinState) - # Phase 3: Transaction Flow
- [ ] **phase-3.task-2**: Implement coin selection and PSBT creation with SP outputs - # Phase 3: Transaction Flow
- [ ] **phase-3.task-3**: Implement transaction signing and Electrum broadcast - # Phase 3: Transaction Flow

---

## phase-4: GUI Screens

**Status:** Pending (0/3)

### Tasks

- [ ] **phase-4.task-1**: Implement AppController, MainWindow, and AccountController with timer-based polling - # Phase 4: GUI Screens
- [ ] **phase-4.task-2**: Implement Coins screen and Receive screen - # Phase 4: GUI Screens
- [ ] **phase-4.task-3**: Implement Send screen and Settings screen - # Phase 4: GUI Screens

---

## phase-5: Integration and Testing

**Status:** Pending (0/2)

### Tasks

- [ ] **phase-5.task-1**: End-to-end integration testing on regtest and signet - # Phase 5: Integration and Testing
- [ ] **phase-5.task-2**: Multi-account testing, error handling, and edge cases - # Phase 5: Integration and Testing

---

## Summary

| Phase | Status | Tasks | Completed |
|-------|--------|-------|----------:|
| phase-1: Scaffolding | Pending | 3 | 0 |
| phase-2: Core Backend | Pending | 4 | 0 |
| phase-3: Transaction Flow | Pending | 3 | 0 |
| phase-4: GUI Screens | Pending | 3 | 0 |
| phase-5: Integration and Testing | Pending | 2 | 0 |
| **Total** | | **15** | **0** |

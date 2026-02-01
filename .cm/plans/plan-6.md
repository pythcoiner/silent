# Phase 6: GUI Controllers

## Objective

Build the Qt6 application infrastructure: AppController, MainWindow with tabs, AccountController with polling, AccountWidget, MenuTab, and CreateAccount modal.

## Files to Read

- `../qoinstr/src/AppController.h` and `.cpp` - Reference app controller
- `../qoinstr/src/MainWindow.h` and `.cpp` - Reference main window
- `../qoinstr/src/AccountController.h` and `.cpp` - Reference account controller (polling pattern)
- `../qoinstr/src/AccountWidget.h` and `.cpp` - Reference account widget
- `../qoinstr/src/screens/MenuTab.h` and `.cpp` - Reference menu tab
- `../qoinstr/src/screens/modals/CreateAccount.h` and `.cpp` - Reference account creation

## Implementation Steps

### Task 1: AppController, MainWindow, AccountController

1. **AppController** (src/AppController.h/.cpp)
   - Inherit qontrol::Controller
   - Manage HashMap of AccountController instances
   - createAccount(name), removeAccount(name), listAccounts()

2. **MainWindow** (src/MainWindow.h/.cpp)
   - Inherit qontrol::Window
   - QTabWidget for account tabs + "+" tab

3. **AccountController** (src/AccountController.h/.cpp)
   - Manages one Account (Rust) via CXX
   - QTimer at 100ms polling try_recv() for Notifications
   - Emits Qt signals: scanProgress(), newOutput(), outputSpent(), scanError()
   - Manages panel HashMap for screen navigation

### Task 2: AccountWidget, MenuTab, CreateAccount modal

1. **AccountWidget** (src/AccountWidget.h/.cpp)
   - Side menu (qontrol::Column) with buttons: Coins, Send, Receive, Settings
   - Screen container for current screen

2. **MenuTab** (src/screens/MenuTab.h/.cpp)
   - "Create Account" button → opens CreateAccount modal

3. **CreateAccount modal** (src/screens/modals/CreateAccount.h/.cpp)
   - Account name input
   - Generate new mnemonic or restore
   - Network selector (Regtest/Signet/Testnet/Bitcoin)
   - BlindBit URL input

### Task 3: main.cpp and CMakeLists.txt updates

1. **src/main.cpp** — Full startup: create AppController, MainWindow, load accounts, show
2. **CMakeLists.txt** — Add all new .h/.cpp source files

## Files to Create

- `src/AppController.h`, `src/AppController.cpp`
- `src/MainWindow.h`, `src/MainWindow.cpp`
- `src/AccountController.h`, `src/AccountController.cpp`
- `src/AccountWidget.h`, `src/AccountWidget.cpp`
- `src/screens/MenuTab.h`, `src/screens/MenuTab.cpp`
- `src/screens/modals/CreateAccount.h`, `src/screens/modals/CreateAccount.cpp`

## Files to Modify

- `src/main.cpp` - Full application startup
- `CMakeLists.txt` - Add new source files

## Verification

- [ ] Application compiles and launches
- [ ] Account can be created via modal
- [ ] AccountController polls Rust backend
- [ ] Tab navigation works

## Reviewer Criteria

**Must check:**
- [ ] No Electrum/Nostr/Pool references in any GUI code
- [ ] AccountController polls Notifications (not Electrum signals)
- [ ] CreateAccount modal has BlindBit URL field (not Electrum)
- [ ] Network selector includes all 4 networks

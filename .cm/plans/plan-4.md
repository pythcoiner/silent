# Phase 4: GUI Screens

## Objective

Build the complete Qt6 GUI using qontrol framework: application controllers, main window with tab-based account management, and all wallet screens (Coins, Receive, Send, Settings).

## Files to Read

- `../qoinstr/src/AppController.h` and `.cpp` - Reference app controller
- `../qoinstr/src/MainWindow.h` and `.cpp` - Reference main window
- `../qoinstr/src/AccountController.h` and `.cpp` - Reference account controller
- `../qoinstr/src/AccountWidget.h` and `.cpp` - Reference account widget
- `../qoinstr/src/screens/Coins.h` and `.cpp` - Reference coins screen
- `../qoinstr/src/screens/Receive.h` and `.cpp` - Reference receive screen
- `../qoinstr/src/screens/Send.h` and `.cpp` - Reference send screen
- `../qoinstr/src/screens/Settings.h` and `.cpp` - Reference settings screen
- `../qoinstr/src/screens/MenuTab.h` and `.cpp` - Reference menu tab
- `../qoinstr/src/screens/common.h` and `.cpp` - Reference UI utilities

## Implementation Steps

### Task 1: AppController + MainWindow + AccountController

1. **Create AppController** (`src/AppController.h`, `src/AppController.cpp`)
   - Inherit from `qontrol::Controller`
   - Manage HashMap of AccountController instances
   - Methods: createAccount(name), removeAccount(name), listAccounts()
   - Handle application-level signals

2. **Create MainWindow** (`src/MainWindow.h`, `src/MainWindow.cpp`)
   - Inherit from `qontrol::Window`
   - Contains QTabWidget for account tabs
   - Plus "+" tab (MenuTab) for creating new accounts
   - Methods: addAccountTab(name), removeAccountTab(name)

3. **Create AccountController** (`src/AccountController.h`, `src/AccountController.cpp`)
   - Manages one Account (Rust backend via CXX)
   - QTimer polling at 100ms for try_recv() notifications
   - QTimer polling at 1000ms for coin state updates
   - Signal/slot connections to update screens
   - Manages panel HashMap for screen navigation
   - Signals: coinUpdate(), addressUpdate(), txSigned(), error(QString)

4. **Create AccountWidget** (`src/AccountWidget.h`, `src/AccountWidget.cpp`)
   - QWidget container for one account
   - Side menu (qontrol::Column) with navigation buttons: Coins, Send, Receive, Settings
   - Screen container (QWidget) holding current screen
   - Connect button clicks to screen switching

5. **Update src/main.cpp**
   - Create AppController
   - Create MainWindow
   - Load existing accounts from config
   - Show window

6. **Update CMakeLists.txt**
   - Add all new .h/.cpp files to sources
   - Include templar headers from lib/include/

### Task 2: Coins screen + Receive screen

1. **Create Coins screen** (`src/screens/Coins.h`, `src/screens/Coins.cpp`)
   - Inherit from `qontrol::Screen`
   - Override init(), view(), doConnect()
   - Display UTXO table: outpoint, value (BTC), confirmations, status, label
   - Show total balance (confirmed/unconfirmed)
   - Coin labeling (click to edit)
   - Connect to AccountController::coinUpdate()

2. **Create Receive screen** (`src/screens/Receive.h`, `src/screens/Receive.cpp`)
   - Inherit from `qontrol::Screen`
   - Override init(), view(), doConnect()
   - Display current SP receive address (large, copyable)
   - "Copy" button to clipboard
   - "New Address" button to generate next
   - Address list with usage status
   - Connect to AccountController::addressUpdate()

3. **Create common UI utilities** (`src/screens/common.h`, `src/screens/common.cpp`)
   - Helper functions for formatting BTC amounts
   - Standard row/column layouts
   - Common stylesheet definitions

### Task 3: Send screen + Settings screen

1. **Create Send screen** (`src/screens/Send.h`, `src/screens/Send.cpp`)
   - Inherit from `qontrol::Screen`
   - Override init(), view(), doConnect()
   - Input fields:
     - Recipient SP address (InputLine)
     - Amount in BTC (InputLine)
     - Fee rate in sats/vB (InputLine)
     - "Max" checkbox to send entire balance
   - Coin selection button (opens SelectCoins modal)
   - "Simulate" button → shows fee estimate
   - "Send" button → prepare, sign, broadcast
   - Status display for transaction result
   - Connect to AccountController signals

2. **Create SelectCoins modal** (`src/screens/modals/SelectCoins.h`, `src/screens/modals/SelectCoins.cpp`)
   - Modal dialog showing available UTXOs
   - Checkboxes for selecting specific coins
   - Show selected total vs required amount
   - OK/Cancel buttons

3. **Create Settings screen** (`src/screens/Settings.h`, `src/screens/Settings.cpp`)
   - Inherit from `qontrol::Screen`
   - Override init(), view(), doConnect()
   - Editable fields:
     - Electrum URL (InputLine)
     - Electrum port (InputLine)
     - Network selector (ComboBox: Regtest/Signet/Testnet/Bitcoin)
   - "Save" button to persist config
   - "Connect"/"Disconnect" buttons for Electrum
   - Connection status indicator

4. **Create MenuTab** (`src/screens/MenuTab.h`, `src/screens/MenuTab.cpp`)
   - Screen for the "+" tab in MainWindow
   - "Create Account" button → opens CreateAccount modal
   - List of existing accounts

5. **Create CreateAccount modal** (`src/screens/modals/CreateAccount.h`, `src/screens/modals/CreateAccount.cpp`)
   - Modal for creating new wallet account
   - Input: account name
   - Option: generate new mnemonic or restore from existing
   - Mnemonic input field (for restore)
   - Network selector
   - OK/Cancel buttons

## Files to Create

- `src/AppController.h`, `src/AppController.cpp`
- `src/MainWindow.h`, `src/MainWindow.cpp`
- `src/AccountController.h`, `src/AccountController.cpp`
- `src/AccountWidget.h`, `src/AccountWidget.cpp`
- `src/screens/Coins.h`, `src/screens/Coins.cpp`
- `src/screens/Receive.h`, `src/screens/Receive.cpp`
- `src/screens/Send.h`, `src/screens/Send.cpp`
- `src/screens/Settings.h`, `src/screens/Settings.cpp`
- `src/screens/MenuTab.h`, `src/screens/MenuTab.cpp`
- `src/screens/common.h`, `src/screens/common.cpp`
- `src/screens/modals/SelectCoins.h`, `src/screens/modals/SelectCoins.cpp`
- `src/screens/modals/CreateAccount.h`, `src/screens/modals/CreateAccount.cpp`

## Files to Modify

- `src/main.cpp` - Full application startup
- `CMakeLists.txt` - Add all new source files

## Verification

- [ ] Application compiles and launches
- [ ] Account can be created via CreateAccount modal
- [ ] All 4 screens (Coins, Send, Receive, Settings) display and navigate correctly
- [ ] AccountController polls backend and updates screens
- [ ] Send screen can simulate a transaction
- [ ] Settings can be saved and loaded

## Reviewer Criteria

**Must check:**
- [ ] All screens inherit from qontrol::Screen and implement init/view/doConnect
- [ ] AccountController polling timer is set up correctly (100ms for signals, 1000ms for coins)
- [ ] No pools/CoinJoin UI elements (SP-only wallet)
- [ ] Network selector includes all 4 networks
- [ ] Memory management follows Qt parent-child ownership

**May skip:**
- [ ] Pixel-perfect UI layout
- [ ] QSS styling/theming details

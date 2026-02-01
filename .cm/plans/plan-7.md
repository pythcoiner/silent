# Phase 7: Wallet Screens

## Objective

Implement all wallet screens: Coins, Receive, Send (with SelectCoins modal), Settings, and common UI utilities.

## Files to Read

- `../qoinstr/src/screens/Coins.h` and `.cpp` - Reference coins screen
- `../qoinstr/src/screens/Receive.h` and `.cpp` - Reference receive screen
- `../qoinstr/src/screens/Send.h` and `.cpp` - Reference send screen
- `../qoinstr/src/screens/Settings.h` and `.cpp` - Reference settings screen
- `../qoinstr/src/screens/modals/SelectCoins.h` and `.cpp` - Reference coin selection
- `../qoinstr/src/screens/common.h` and `.cpp` - Reference UI utilities

## Implementation Steps

### Task 1: Coins screen and Receive screen

1. **Coins screen** (src/screens/Coins.h/.cpp)
   - Inherit qontrol::Screen, implement init/view/doConnect
   - UTXO table: outpoint, value (BTC), confirmations, status, label
   - Total balance (confirmed/unconfirmed)
   - Coin labeling (click to edit)
   - Connect to AccountController::newOutput()

2. **Receive screen** (src/screens/Receive.h/.cpp)
   - Inherit qontrol::Screen
   - Display static SP receive address (large, copyable)
   - "Copy" button to clipboard

### Task 2: Send screen with SelectCoins modal

1. **Send screen** (src/screens/Send.h/.cpp)
   - Recipient SP address input, amount, fee rate, "Max" checkbox
   - Coin selection button → opens SelectCoins modal
   - "Simulate" button → fee estimate
   - "Send" button → sign and broadcast

2. **SelectCoins modal** (src/screens/modals/SelectCoins.h/.cpp)
   - UTXO list with checkboxes
   - Selected total display

### Task 3: Settings screen and common utilities

1. **Settings screen** (src/screens/Settings.h/.cpp)
   - BlindBit URL input
   - Network selector (all 4 networks)
   - Save/connect/disconnect buttons

2. **Common utilities** (src/screens/common.h/.cpp)
   - BTC amount formatting
   - Standard row/column layouts

## Files to Create

- `src/screens/Coins.h`, `src/screens/Coins.cpp`
- `src/screens/Receive.h`, `src/screens/Receive.cpp`
- `src/screens/Send.h`, `src/screens/Send.cpp`
- `src/screens/Settings.h`, `src/screens/Settings.cpp`
- `src/screens/modals/SelectCoins.h`, `src/screens/modals/SelectCoins.cpp`
- `src/screens/common.h`, `src/screens/common.cpp`

## Files to Modify

- `CMakeLists.txt` - Add new source files

## Verification

- [ ] All screens display and navigate correctly
- [ ] Send screen can simulate a transaction
- [ ] Settings saves BlindBit URL
- [ ] Coins screen updates on NewOutput notification

## Reviewer Criteria

**Must check:**
- [ ] All screens inherit qontrol::Screen with init/view/doConnect
- [ ] Settings uses BlindBit URL (not Electrum)
- [ ] Receive shows static SP address
- [ ] No Pools/CoinJoin screen

# Phase 3: SP Addresses & BlindBit Sync

## Objective

Expose Silent Payment address generation, BlindBit scanner control, coin tracking, and payment history via CXX.

## Files to Read

- `../bwk/sp/src/account.rs` - SP address via SpClient, scan_blocks, start_scanner, coins, payment_history
- `../bwk/sp/src/coin_store.rs` - SpCoinStore, SpCoinEntry (outpoint, value, height, label)
- `../bwk/sp/src/tx_store.rs` - SpTxStore, SpTxEntry (txid, direction, amount, fees, confirmation)
- `../bwk/sp/src/scan_state.rs` - ScanState (last_scanned_height, birthday_height)
- `../bwk/sp/src/lib.rs` - Public API exports

## Implementation Steps

### Task 1: SP address generation and display

1. **Add to silent/src/account.rs**
   - sp_address(&self) -> String — returns the wallet's static SP receive address
   - Expose via CXX bridge in lib.rs

### Task 2: BlindBit scanner and coin tracking

1. **Add scanner control to account.rs**
   - start_scanner(&mut self) — calls bwk-sp Account::start_scanner()
   - stop_scanner(&mut self) — calls bwk-sp Account::stop_scanner()
   - Already exposed in Phase 2, verify CXX bridge works

2. **Add coin tracking**
   - Define RustCoin CXX struct: outpoint (String), value (u64), height (u32), label (String), spent (bool)
   - coins(&self) -> Vec<RustCoin> — maps bwk-sp SpCoinEntry to RustCoin
   - Define CoinState CXX struct: confirmed_count, confirmed_balance, unconfirmed_count, unconfirmed_balance
   - spendable_coins(&self) -> CoinState — aggregates from SpCoinStore
   - update_coin_label(&mut self, outpoint: String, label: String)

### Task 3: Payment history

1. **Add transaction history**
   - Define RustTx CXX struct: txid (String), direction (String: "incoming"/"outgoing"), amount (u64), fee (u64), height (u32)
   - payment_history(&self) -> Vec<RustTx> — maps bwk-sp SpTxEntry

2. **Update CXX bridge** with all new types and methods

## Files to Modify

- `silent/src/account.rs` - Add address, coin, tx methods
- `silent/src/lib.rs` - Add RustCoin, CoinState, RustTx types, expose methods

## Verification

- [ ] `cargo build --release` succeeds
- [ ] `cargo clippy` passes
- [ ] SP address can be retrieved
- [ ] Scanner can start/stop
- [ ] Coins list populated after scan

## Reviewer Criteria

**Must check:**
- [ ] SP address is the static silent payment address (not a derived per-tx address)
- [ ] No Electrum references anywhere
- [ ] RustCoin maps correctly from SpCoinEntry

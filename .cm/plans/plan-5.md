# Phase 5: Coin Selection, Signing & Broadcast

## Objective

Complete the send flow: coin selection, SP transaction creation via bwk-sp, signing, and broadcast via ureq HTTP.

## Files to Read

- `../bwk/sp/src/account.rs` - bwk-sp transaction building, signing, broadcast methods
- `../bwk/sp/src/coin_store.rs` - Coin selection from SpCoinStore

## Implementation Steps

### Task 1: Coin selection and SP transaction creation

1. **Add to silent/src/account.rs**
   - prepare_transaction(&mut self, template: TransactionTemplate) -> Box<TxResult>
   - Map template inputs to OutPoints for coin selection
   - Use bwk-sp's SilentPaymentUnsignedTransaction for SP output construction
   - Handle change output
   - Support manual (specific outpoints) and automatic coin selection

### Task 2: Signing and broadcast

1. **Add to silent/src/account.rs**
   - sign_and_broadcast(&mut self, template: TransactionTemplate) -> Box<TxResult>
   - Or: prepare → sign → broadcast as separate steps
   - Signing via bwk-sp (uses spdk-core internally)
   - Broadcast via ureq HTTP POST (bwk-sp handles this)
   - Send success Notification on broadcast

2. **Update CXX bridge** with send methods

## Files to Modify

- `silent/src/account.rs` - Add transaction methods
- `silent/src/lib.rs` - Expose send methods via CXX

## Verification

- [ ] `cargo build --release` succeeds
- [ ] `cargo clippy` passes
- [ ] Transaction can be created with SP outputs
- [ ] Signing works with mnemonic-derived keys
- [ ] Broadcast sends via HTTP (testable with mock)

## Reviewer Criteria

**Must check:**
- [ ] SP outputs correctly constructed (not legacy)
- [ ] Change output handled
- [ ] Error cases: insufficient funds, invalid SP address
- [ ] No Electrum broadcast — uses ureq HTTP POST

# Phase 4: Transaction Types & Simulation

## Objective

Define CXX-exposed transaction data types and implement transaction simulation for fee estimation.

## Files to Read

- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX transaction type patterns
- `../bwk/sp/src/account.rs` - bwk-sp transaction building methods

## Implementation Steps

### Task 1: CXX transaction type definitions

1. **Add to templar/src/lib.rs** (CXX bridge shared types)
   - TransactionTemplate: inputs (Vec<String> outpoints), outputs (Vec<Output>), fee_rate (f64 sats/vB)
   - Output: address (String, SP address), amount (u64), label (String), max (bool)
   - TransactionSimulation: is_valid (bool), fee (u64), weight (usize), error (String)
   - CoinState: confirmed_count (u32), confirmed_balance (u64), unconfirmed_count (u32), unconfirmed_balance (u64)

### Task 2: Simulation method and result wrappers

1. **Add to templar/src/account.rs**
   - simulate_transaction(&self, template: TransactionTemplate) -> TransactionSimulation
   - Validate inputs exist and are spendable
   - Calculate fee based on weight estimation
   - Return simulation result with is_valid, fee, weight

2. **Define result wrapper** for CXX error handling
   - TxResult opaque type: is_ok(), is_err(), error() -> String

## Files to Modify

- `templar/src/lib.rs` - Add transaction types to CXX bridge
- `templar/src/account.rs` - Add simulate_transaction method

## Verification

- [ ] `cargo build --release` succeeds
- [ ] `cargo clippy` passes
- [ ] Transaction simulation returns valid fee estimates

## Reviewer Criteria

**Must check:**
- [ ] All types are CXX-compatible (shared structs, not opaque)
- [ ] Simulation handles edge cases (no inputs, invalid outpoints)

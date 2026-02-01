# Phase 3: Transaction Flow

## Objective

Enable the complete send flow: define CXX-exposed transaction types, implement coin selection with SP output construction, and add PSBT signing and Electrum broadcast.

## Files to Read

- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX transaction types
- `../qoinstr/cpp_joinstr/src/coin_store.rs` - Reference coin selection
- `../qoinstr/cpp_joinstr/src/signing_manager.rs` - Reference PSBT signing
- `../qoinstr/cpp_joinstr/src/signer/mod.rs` - Reference hot signer
- `../bwk/bwk-tx/src/lib.rs` - bwk transaction builder
- `../bwk/bwk-sign/src/lib.rs` - bwk signing module
- `templar/src/lib.rs` - Current CXX bridge
- `templar/src/account.rs` - Current Account implementation

## Implementation Steps

### Task 1: CXX-exposed transaction types

1. **Define TransactionTemplate** (`templar/src/lib.rs`)
   - Shared struct with:
     - inputs: Vec<String> (outpoint strings)
     - outputs: Vec<Output>
     - fee_rate: f64 (sats/vB)
   - Output struct: address (SP address), amount (u64), label (String), max (bool)

2. **Define TransactionSimulation** (`templar/src/lib.rs`)
   - Shared struct with:
     - is_valid: bool
     - fee: u64
     - weight: usize
     - error: String (empty if valid)

3. **Define CoinState** (`templar/src/lib.rs`)
   - Shared struct with:
     - confirmed_count: u32, confirmed_balance: u64
     - unconfirmed_count: u32, unconfirmed_balance: u64
     - total_count: u32, total_balance: u64

4. **Export simulation method** (`templar/src/account.rs`)
   - `simulate_transaction(&self, template: TransactionTemplate) -> TransactionSimulation`
   - Validate inputs exist and are spendable
   - Calculate fee based on weight estimation
   - Return simulation result

### Task 2: Coin selection and PSBT creation

1. **Implement coin selection** (`templar/src/account.rs`)
   - Use bwk's tx_builder for coin selection
   - Support manual coin selection (specific outpoints from template)
   - Support automatic selection (if no inputs specified)
   - Filter only confirmed, spendable SP coins

2. **Implement PSBT creation** (`templar/src/account.rs`)
   - `prepare_transaction(&mut self, template: TransactionTemplate) -> Box<PsbtResult>`
   - Build SP outputs using bwk-sp
   - Create PSBT with selected inputs and SP outputs
   - Handle change output (SP change address)
   - Return PsbtResult (wraps Result with is_ok/is_err/value/error)

3. **Define PsbtResult type** (`templar/src/lib.rs`)
   - Opaque Rust type in CXX bridge
   - Methods: is_ok(), is_err(), error() -> String

### Task 3: Transaction signing and broadcast

1. **Implement signing** (`templar/src/account.rs`)
   - Use bwk_sign with hot signer (mnemonic-derived xprv)
   - Sign all inputs in the PSBT
   - Finalize PSBT into raw transaction

2. **Implement broadcast** (`templar/src/account.rs`)
   - Send raw transaction via Electrum
   - On success, send SignedTx signal through notification channel
   - On failure, send Error signal with details

3. **Export to CXX** (`templar/src/lib.rs`)
   - `sign_and_broadcast(&mut self, template: TransactionTemplate) -> Box<TxResult>`
   - Or split: `prepare_transaction` + `sign_transaction` + `broadcast_transaction`

## Files to Modify

- `templar/src/lib.rs` - Add transaction types and methods to CXX bridge
- `templar/src/account.rs` - Add transaction methods to Account

## Files to Create

None (all additions to existing files)

## Verification

- [ ] `cargo build --release` succeeds
- [ ] `cargo clippy` passes
- [ ] TransactionSimulation returns valid fee estimates
- [ ] PSBT can be created with SP outputs
- [ ] PSBT can be signed with hot signer
- [ ] `cargo test` passes (unit tests for simulation/selection)

## Reviewer Criteria

**Must check:**
- [ ] SP outputs are correctly constructed (not legacy addresses)
- [ ] Change output uses SP change address
- [ ] Fee calculation accounts for witness data correctly
- [ ] Coin selection only picks confirmed, spendable coins
- [ ] Error cases are handled (insufficient funds, invalid address)

**May skip:**
- [ ] Advanced fee estimation modes (target blocks)
- [ ] RBF (Replace-By-Fee) support

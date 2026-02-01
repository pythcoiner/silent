# Phase 8: Integration & Testing

## Objective

End-to-end testing on regtest/signet, multi-account testing, error handling, and edge cases.

## Files to Read

- `templar/src/account.rs` - Account implementation
- `templar/src/config.rs` - Config implementation
- `templar/src/lib.rs` - CXX bridge types

## Implementation Steps

### Task 1: End-to-end integration testing

1. **Create templar/tests/integration.rs**
   - Test wallet creation from mnemonic
   - Test config save/load round-trip
   - Test SP address generation (verify format)
   - Test BlindBit connection on regtest (requires local BlindBit server)
   - Test send/receive SP flow on regtest
   - Test wallet restore from mnemonic (verify funds recovery)

2. **Test on signet** — manual testing with signet BlindBit server

### Task 2: Multi-account and error handling

1. **Create templar/tests/multi_account.rs**
   - Multiple accounts with different mnemonics
   - Independent state, different networks
   - Config files in separate directories

2. **Error handling tests**
   - Invalid BlindBit URL → ScanError notification
   - Invalid mnemonic → config creation fails gracefully
   - Insufficient funds → transaction error
   - Invalid SP address → prepare error
   - Connection loss → ScanError with retries

3. **Edge cases**
   - Zero balance send attempt
   - Dust output detection
   - Max send (entire balance)

## Files to Create

- `templar/tests/integration.rs`
- `templar/tests/multi_account.rs`

## Verification

- [ ] `cargo test` passes all tests
- [ ] No panics on any error path
- [ ] Multi-account isolation verified
- [ ] Wallet restore works correctly

## Reviewer Criteria

**Must check:**
- [ ] All error paths return Result/Notification, never panic
- [ ] Integration tests cover send and receive SP flows
- [ ] Tests use BlindBit (not Electrum)

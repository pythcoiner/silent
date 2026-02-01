# Phase 5: Integration and Testing

## Objective

Validate the complete wallet with end-to-end testing on regtest and signet. Test multi-account support, error handling, and edge cases.

## Files to Read

- `templar/src/account.rs` - Account implementation
- `templar/src/config.rs` - Config implementation
- `templar/src/lib.rs` - CXX bridge types
- `src/AccountController.cpp` - GUI polling logic

## Implementation Steps

### Task 1: End-to-end integration testing

1. **Create Rust integration tests** (`templar/tests/integration.rs`)
   - Test wallet creation from mnemonic
   - Test config save/load round-trip
   - Test SP address generation (verify format)
   - Test Electrum connection on regtest (requires local Electrum server)
   - Test send/receive flow:
     - Generate SP receive address
     - Fund wallet (via regtest RPC)
     - Verify coin appears after sync
     - Send to another SP address
     - Verify transaction broadcast

2. **Test on signet**
   - Manual testing with signet Electrum server
   - Verify SP address generation works on signet network
   - Verify sync picks up signet transactions

3. **Test wallet restore**
   - Create wallet, generate addresses, receive funds
   - Delete wallet state files
   - Restore from same mnemonic
   - Verify funds are recovered after sync

### Task 2: Multi-account and error handling

1. **Multi-account tests** (`templar/tests/multi_account.rs`)
   - Create multiple accounts with different mnemonics
   - Verify accounts have independent state
   - Verify accounts can use different networks
   - Verify config files are stored in separate directories

2. **Error handling tests**
   - Invalid Electrum URL → error signal, not panic
   - Invalid mnemonic → config creation fails gracefully
   - Send with insufficient funds → simulation returns error
   - Send to invalid SP address → prepare_transaction returns error
   - Connection lost during sync → TxListenerError signal

3. **Edge case tests**
   - Zero balance send attempt
   - Dust output detection
   - Maximum number of inputs in transaction
   - Address reuse detection

## Files to Create

- `templar/tests/integration.rs` - Integration test suite
- `templar/tests/multi_account.rs` - Multi-account test suite

## Files to Modify

- `templar/src/account.rs` - Fix any issues found during testing
- `templar/src/config.rs` - Fix any issues found during testing

## Verification

- [ ] `cargo test` passes all unit tests
- [ ] `cargo test --test integration` passes (with local regtest)
- [ ] `cargo test --test multi_account` passes
- [ ] No panics on any error path
- [ ] Application handles Electrum disconnection gracefully

## Reviewer Criteria

**Must check:**
- [ ] All error paths return Result/Signal, never panic
- [ ] Integration tests cover send and receive flows
- [ ] Multi-account isolation is verified
- [ ] Wallet restore from mnemonic works correctly

**May skip:**
- [ ] Performance benchmarks
- [ ] Stress testing with many concurrent accounts

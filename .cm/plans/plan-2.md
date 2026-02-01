# Phase 2: Config & Account

## Objective

Implement the foundational Rust backend: Config wrapping bwk-sp::Config, Account wrapping bwk-sp::Account, and Notification polling via CXX FFI.

## Files to Read

- `../bwk/sp/src/config.rs` - bwk-sp Config implementation (mnemonic, blindbit_url, network)
- `../bwk/sp/src/account.rs` - bwk-sp Account (constructor, receiver(), start_scanner, stop_scanner, Notification enum)
- `../qoinstr/cpp_joinstr/src/config.rs` - Reference CXX config wrapper pattern
- `../qoinstr/cpp_joinstr/src/account.rs` - Reference CXX account wrapper pattern
- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX bridge type definitions

## Implementation Steps

### Task 1: Config module

1. **Create templar/src/config.rs**
   - Wrap bwk-sp::Config with CXX-compatible interface
   - Fields: mnemonic (String), network (Network), blindbit_url (String), dust_limit (Option<u64>)
   - File persistence: save/load JSON at ~/.templar/<account>/config.json
   - CXX functions: new_config(), config_from_file(), getters/setters
   - list_configs() -> Vec<String> to enumerate existing accounts

### Task 2: Account struct

1. **Create templar/src/account.rs**
   - Wrap bwk-sp::Account
   - Constructor: new_account(name) -> Box<Account> (loads config, creates bwk-sp::Account)
   - Lifecycle: start_scanner(), stop_scanner()
   - Store the mpsc::Receiver<bwk_sp::Notification> from account.receiver()

### Task 3: Notification polling

1. **Implement try_recv** in account.rs
   - Wrap bwk-sp Notification enum into CXX-compatible NotificationFlag enum
   - NotificationFlag variants: ScanStarted, ScanProgress, ScanCompleted, ScanError, NewOutput, OutputSpent, Stopped
   - Poll struct: is_some + Notification value
   - try_recv(&mut self) -> Box<Poll> — non-blocking check on mpsc Receiver

### Task 4: CXX bridge expansion

1. **Update templar/src/lib.rs**
   - Add Config as opaque Rust type with extern methods
   - Add Account as opaque Rust type with extern methods
   - Add shared enums: Network (Regtest, Signet, Testnet, Bitcoin), LogLevel, NotificationFlag
   - Add shared structs: Poll, Notification (flag + optional payload)
   - Export: new_config, config_from_file, new_account, list_configs, try_recv

## Files to Create

- `templar/src/config.rs`
- `templar/src/account.rs`

## Files to Modify

- `templar/src/lib.rs` - Expand CXX bridge

## Verification

- [ ] `cargo build --release` succeeds
- [ ] `cargo clippy` passes
- [ ] Config can be created, saved, and loaded from file
- [ ] Account instantiates from config
- [ ] Notification channel is functional

## Reviewer Criteria

**Must check:**
- [ ] Config uses blindbit_url, NOT electrum_url
- [ ] Account wraps bwk-sp::Account, NOT bwk::Account
- [ ] NotificationFlag maps correctly to bwk-sp::Notification variants
- [ ] Network enum covers all 4 networks

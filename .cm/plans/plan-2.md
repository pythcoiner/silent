# Phase 2: Core Backend

## Objective

Implement the Rust backend: configuration persistence, Account struct wrapping bwk::Account with CXX FFI, Silent Payment address generation, and Electrum sync with a notification channel for the GUI to poll.

## Files to Read

- `../qoinstr/cpp_joinstr/src/config.rs` - Reference config implementation
- `../qoinstr/cpp_joinstr/src/account.rs` - Reference account state machine
- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX bridge types
- `../qoinstr/cpp_joinstr/src/address_store.rs` - Reference address management
- `../qoinstr/cpp_joinstr/src/derivator.rs` - Reference address derivation
- `../bwk/bwk/src/lib.rs` - bwk public API
- `../bwk/bwk/src/account.rs` - bwk Account implementation
- `../bwk/sp/src/lib.rs` - bwk Silent Payments module

## Implementation Steps

### Task 1: Config module

1. **Create templar/src/config.rs** (`templar/src/config.rs`)
   - Define Config struct with fields:
     - electrum_url: String
     - electrum_port: u16
     - network: Network (Regtest/Signet/Testnet/Bitcoin)
     - look_ahead: usize (default 20)
     - mnemonic: String (BIP39)
     - sp_descriptor: String (Silent Payment descriptor)
   - Implement file persistence:
     - Save to `~/.templar/<account>/config.json`
     - Load from file with serde_json
   - Implement conversion to/from bwk::Config
   - Add CXX-compatible wrapper functions:
     - `new_config(mnemonic, account, network) -> Box<Config>`
     - `config_from_file(account) -> Box<Config>`
     - Getter/setter methods for each field

2. **Update CXX bridge** (`templar/src/lib.rs`)
   - Add Config to shared types
   - Add Network enum (Regtest, Signet, Testnet, Bitcoin)
   - Export config creation and loading functions
   - Export `list_configs() -> Vec<String>`

### Task 2: Account struct

1. **Create templar/src/account.rs** (`templar/src/account.rs`)
   - Define Account struct wrapping bwk::Account
   - Add mpsc::Sender/Receiver for notification channel
   - Implement lifecycle methods:
     - `new_account(account_name: String) -> Box<Account>` - creates from config file
     - `start(&mut self)` - starts Electrum listener
     - `stop(&mut self)` - stops listeners gracefully
   - Implement `try_recv(&mut self) -> Box<Poll>` - polls notification channel
   - Expose basic query methods:
     - `spendable_coins(&self) -> CoinState`
     - `balance(&self) -> u64`

2. **Define Signal/Poll types** (`templar/src/lib.rs`)
   - SignalFlag enum: TxListenerStarted, TxListenerStopped, TxListenerError, AddressTipChanged, CoinUpdate, AccountError, SignedTx, Stopped, Error
   - Signal struct: flag + optional payload string
   - Poll struct: is_some flag + Signal value
   - Result wrapper macros (follow qoinstr pattern)

3. **Update CXX bridge** (`templar/src/lib.rs`)
   - Add Account as opaque Rust type
   - Export all Account methods
   - Add Signal, Poll, SignalFlag, CoinState shared types

### Task 3: SP address generation

1. **Create templar/src/address.rs** (`templar/src/address.rs`)
   - Wrap bwk-sp address derivation
   - Define RustAddress struct for CXX:
     - address: String (SP address)
     - index: u32
     - status: AddressStatus (NotUsed/Used/Reused)
     - account: AddrAccount (Receive/Change)
   - Implement address generation:
     - `receive_addresses(&self) -> Vec<RustAddress>`
     - `next_receive_address(&mut self) -> String`
   - Implement address status tracking

2. **Update CXX bridge** (`templar/src/lib.rs`)
   - Add RustAddress, AddressStatus, AddrAccount shared types
   - Export address query methods on Account

### Task 4: Electrum sync integration

1. **Implement Electrum listener** (`templar/src/account.rs`)
   - Add `start_electrum(&mut self)` method
   - Spawn listener thread using bwk_electrum
   - Route notifications through mpsc channel as Signal values
   - Map bwk::TxListenerNotif to SignalFlag variants
   - Handle connection errors gracefully (send Error signal)

2. **Add coin tracking** (`templar/src/account.rs`)
   - Expose `coins(&self) -> Vec<RustCoin>` via CXX
   - Define RustCoin struct: outpoint, value, height, status, address, label
   - Define CoinStatus enum: Unconfirmed, Confirmed, BeingSpend, Spent
   - Map from bwk coin entries to RustCoin

## Files to Create

- `templar/src/config.rs` - Configuration module
- `templar/src/account.rs` - Account state machine
- `templar/src/address.rs` - SP address management

## Files to Modify

- `templar/src/lib.rs` - Expand CXX bridge with all types and methods

## Verification

- [ ] `cargo build --release` succeeds with all new modules
- [ ] `cargo clippy` passes
- [ ] Config can be created, saved, and loaded from file
- [ ] Account can be instantiated from config
- [ ] SP addresses can be generated
- [ ] Electrum listener starts and sends notifications through channel

## Reviewer Criteria

**Must check:**
- [ ] All CXX types are correctly defined (shared enums, opaque types)
- [ ] Config file persistence uses correct paths (~/.templar/<account>/)
- [ ] Signal/Poll pattern matches qoinstr's design for GUI polling
- [ ] Network enum covers all 4 networks (Regtest, Signet, Testnet, Bitcoin)
- [ ] SP address generation uses bwk-sp correctly

**May skip:**
- [ ] Exhaustive error message formatting
- [ ] Label store implementation (can be minimal initially)

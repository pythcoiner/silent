# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Templar is a privacy-focused desktop Bitcoin wallet using the Silent Payments protocol. It has a two-language architecture: C++ (Qt6 GUI) and Rust (backend via CXX FFI).

## Build Commands

**Rust backend only:**
```bash
cargo build --release --manifest-path templar/Cargo.toml
```

**Full build pipeline** (builds Rust, copies libs/headers to `lib/`):
```bash
./build.sh
```

**C++ GUI build** (after `./build.sh`):
```bash
cmake -B build . && cmake --build build
```

**Lint:**
```bash
cargo clippy --manifest-path templar/Cargo.toml
```

**Tests:**
```bash
cargo test --manifest-path templar/Cargo.toml
```

Run a single test:
```bash
cargo test --manifest-path templar/Cargo.toml <test_name>
```

## Architecture

```
Qt6 GUI (C++)  ──CXX FFI──>  Account (Rust, wraps bwk-sp)
                                   │
                              mpsc::channel
                                   │
                              Scanner Thread (Rust)
                                   │
                              BlindBit Server (HTTP/ureq)
```

**Rust crate (`templar/src/`):** CXX bridge wrapping `bwk-sp` (Bitcoin Wallet Kit - Silent Payments).
- `lib.rs` — CXX bridge definition, all FFI types and exported methods
- `config.rs` — Wallet config persistence (`~/.templar/<account>/`), network settings, mnemonic storage
- `account.rs` — Account wrapper around `bwk-sp::Account`, notification polling via mpsc, transaction lifecycle (simulate → prepare → sign → broadcast)

**C++ GUI (`src/`):** Qt6 app using the `qontrol` framework (fetched via CMake FetchContent).
- `AppController` — Singleton managing multi-account wallet state
- `AccountController` — Per-account controller, wraps `rust::Box<Account>`, QTimer-based polling for notifications and coin state updates
- `MainWindow` — Tab-based account management
- `src/screens/` — Coins, Send, Receive, Settings screens
- `src/screens/modals/` — CreateAccount, SelectCoins dialogs

**Data flow:** User action → Qt signal → AccountController → CXX FFI → Rust Account → bwk-sp. Background scanner sends notifications via mpsc channel, polled by QTimer in AccountController.

## Key Dependencies

- **bwk-sp** / **spdk-core** — Local path dependencies (`../../bwk/sp`) for Silent Payments wallet logic
- **cxx** — Rust-C++ FFI bridge
- **Qt6** (Widgets, Gui, Core) + **qontrol** framework
- **bitcoin** 0.32 — Bitcoin primitives

## Build Pipeline

`build.sh` compiles the Rust crate, then copies generated CXX bridge headers (`templar.h`, `cxx.h`) and static library (`libtemplar.a`) into `lib/`. CMake then links against these when building the C++ GUI. The Rust `build.rs` uses `cxx_build` to generate the C++ bridge code.

## Testing

Tests are in `templar/tests/` using standard Rust `#[test]`:
- `integration.rs` — End-to-end wallet operations (creation, config roundtrip, SP addresses)
- `multi_account.rs` — Multi-account isolation and edge cases

Tests use Regtest network with standard BIP39 test mnemonic and clean up test accounts after each run.

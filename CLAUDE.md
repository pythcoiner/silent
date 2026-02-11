# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Silent is a privacy-focused desktop Bitcoin wallet using the Silent Payments protocol. It has a two-language architecture: C++ (Qt6 GUI) and Rust (backend via CXX FFI).

## Build Commands

**Development (build + run):**
```bash
just br           # build-local + run
just build-local  # Rust binding + CMake build (no Nix)
just run          # run the built binary (./build/silent)
```

**First-time setup** (fetch qontrol framework into `lib/qontrol`):
```bash
just qontrol
```

**Individual steps:**
```bash
cargo build --release --manifest-path silent/Cargo.toml  # Rust only
./build.sh                                                # Rust + copy libs/headers to lib/
cmake -B build . && cmake --build build                   # C++ GUI (after build.sh)
```

**Lint:**
```bash
cargo clippy --manifest-path silent/Cargo.toml
```

**Tests:**
```bash
cargo test --manifest-path silent/Cargo.toml
cargo test --manifest-path silent/Cargo.toml <test_name>  # single test
```

**Clean:**
```bash
just clean  # removes Rust target, build/, and lib/ artifacts
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

**Rust crate (`silent/src/`):** CXX bridge wrapping `bwk-sp` (Bitcoin Wallet Kit - Silent Payments).
- `lib.rs` — CXX bridge definition, all FFI types and exported methods
- `config.rs` — Wallet config persistence (`~/.silent/<account>/`), network settings, mnemonic storage
- `account.rs` — Account wrapper around `bwk-sp::Account`, notification polling via mpsc, transaction lifecycle (simulate → prepare → sign → broadcast)

**C++ GUI (`src/`):** Qt6 app using the `qontrol` framework (fetched via CMake FetchContent).
- `AppController` — Singleton managing multi-account wallet state
- `AccountController` — Per-account controller, wraps `rust::Box<Account>`, QTimer-based polling for notifications and coin state updates
- `MainWindow` — Tab-based account management
- `src/screens/` — Coins, Send, Receive, Settings screens
- `src/screens/modals/` — CreateAccount, SelectCoins dialogs

**Data flow:** User action → Qt signal → AccountController → CXX FFI → Rust Account → bwk-sp. Background scanner sends notifications via mpsc channel, polled by QTimer in AccountController (100ms for notifications, 1000ms for coin state).

**Transaction lifecycle:** `simulate_transaction()` (validate & estimate fee) → `prepare_transaction()` (create unsigned PSBT) → `sign_transaction()` (sign to hex) → `broadcast_transaction()` (submit to network). There is also a combined `sign_and_broadcast()`.

**CXX bridge conventions:** All FFI types and exported methods are defined in `lib.rs` within the `#[cxx::bridge]` module. To expose new Rust functionality to C++, add the type/function declaration in `lib.rs` and implement it in the corresponding module (`account.rs` or `config.rs`). After changes, run `./build.sh` to regenerate the C++ headers (`lib/include/silent.h`, `lib/include/cxx.h`).

## Code Guidelines

See [CODE_GUIDELINES.md](CODE_GUIDELINES.md) for project-specific coding conventions: CXX bridge patterns, naming conventions, qontrol framework usage, Rust/C++ FFI integration, composite widget patterns, and testing conventions.

## Key Dependencies

- **bwk-sp** / **spdk-core** — Local path dependencies (`../../bwk/sp`) for Silent Payments wallet logic
- **cxx** — Rust-C++ FFI bridge
- **Qt6** (Widgets, Gui, Core) + **qontrol** framework
- **bitcoin** 0.32 — Bitcoin primitives

## Build Pipeline

`build.sh` compiles the Rust crate, then copies generated CXX bridge headers (`silent.h`, `cxx.h`) and static library (`libsilent.a`) into `lib/`. CMake then links against these when building the C++ GUI. The Rust `build.rs` uses `cxx_build` to generate the C++ bridge code.

## Nix Build System

Reproducible release builds via Nix flakes with statically linked Qt6. Output binaries go to `./result/bin/`.

```bash
just build           # Linux
just build-win       # Windows (MinGW cross-compilation)
just build-apple     # macOS ARM + x86_64
just build-all       # all of the above
nix develop          # development shell (Rust toolchain, CMake, Qt6)
```

The flake resolves Cargo path dependencies (`../../bwk/sp`, `../../spdk/spdk-core`) by laying out `bwk`, `spdk`, and `silent` as sibling directories in the Nix sandbox. Git dependencies are vendored via `importCargoLock` with pinned hashes.

## Testing

Tests are in `silent/tests/` using standard Rust `#[test]`:
- `integration.rs` — End-to-end wallet operations (creation, config roundtrip, SP addresses)
- `multi_account.rs` — Multi-account isolation and edge cases

Tests use Regtest network with standard BIP39 test mnemonic and clean up test accounts after each run.

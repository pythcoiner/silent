# Build and Test Actions

## Build Commands

The runner verifies every phase with the SAME command set: the full local build.
It compiles the Rust crate (regenerating CXX headers), then configures and builds
the full C++ app, which includes the plugin SDK and its always-built smoke-test
target. This holds from phase 1 onward.

```bash
bash build.sh                    # Rust crate -> lib/libsilent.a + CXX headers
cmake -B build .                 # configure
cmake --build build -j           # build app + SDK + smoke target
# convenience: just build-local
```

First run fetches git crates from GitHub (needs network).

## Lint Commands
```bash
cargo clippy --manifest-path silent/Cargo.toml
cargo fmt --manifest-path silent/Cargo.toml -- --check
```

## Test Commands
```bash
cargo test --manifest-path silent/Cargo.toml
```

## Verification Sequence
1. Build (SDK smoke target early; full local build once host lands)
2. Lint (`cargo clippy`)
3. Test (`cargo test`) for phases touching Rust (8, 9)

## Notes
- `config.toml` `build_commands` is currently set to the SDK smoke target. Switch it
  to the full local build once phase 10 introduces host C++ code.
- Qt6 + a C++20 compiler are required; release builds use Nix (`flake.nix`).

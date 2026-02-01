# Build and Test Actions

## Build Commands

### Primary Build (Rust backend)

```bash
cargo build --release --manifest-path templar/Cargo.toml
```

### Full Build (Rust + C++)

```bash
./build.sh && cmake -B build && cmake --build build
```

### Development Build

```bash
cargo build --manifest-path templar/Cargo.toml
```

## Lint Commands

### Primary Linter

```bash
cargo clippy --manifest-path templar/Cargo.toml
```

## Test Commands

### Unit Tests

```bash
cargo test --manifest-path templar/Cargo.toml
```

### Integration Tests

```bash
cargo test --manifest-path templar/Cargo.toml --test integration
```

### All Tests

```bash
cargo test --manifest-path templar/Cargo.toml --all
```

## Verification Sequence

1. Build Rust crate: `cargo build --release --manifest-path templar/Cargo.toml`
2. Lint: `cargo clippy --manifest-path templar/Cargo.toml`
3. Test: `cargo test --manifest-path templar/Cargo.toml`
4. Full build: `./build.sh && cmake -B build && cmake --build build`

## Environment Setup

```bash
export RUST_BACKTRACE=1
```

## Clean Commands

```bash
cargo clean --manifest-path templar/Cargo.toml && rm -rf build/ lib/
```

## Notes

- Rust must be built before C++ (build.sh handles this)
- Integration tests require a local BlindBit server on regtest
- The build.sh script copies libtemplar.a and headers to lib/ for CMake linking

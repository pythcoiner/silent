# Templar

Privacy-focused desktop Bitcoin wallet using the Silent Payments protocol.

## Architecture

```
Qt6 GUI (C++)  --CXX FFI-->  Rust Backend (bwk-sp)
                                   |
                              Scanner Thread
                                   |
                              BlindBit Server
```

- **C++ / Qt6** — Desktop GUI using the [qontrol](https://github.com/pythcoiner/qontrol) framework
- **Rust** — Wallet logic via [bwk-sp](https://github.com/pythcoiner/bwk) (Bitcoin Wallet Kit - Silent Payments)
- **CXX** — FFI bridge between the two languages

## Prerequisites

- [Nix](https://nixos.org/) with flakes enabled (for release builds)
- [just](https://github.com/casey/just) command runner

## Build

### Development (dynamic Qt)

For local development with system Qt6:

```sh
just br          # build + run
just build-local # build only
```

This uses `build.sh` to compile the Rust backend, then CMake to build the GUI
linked against your system's dynamic Qt6.

### Release (static Qt, Nix)

Release builds use Nix flakes to produce fully reproducible binaries with
statically linked Qt6. All cross-compilation happens from a Linux x86_64 host.

```sh
just build       # Linux
just build-win   # Windows (MinGW cross-compilation)
just build-apple # macOS ARM + x86_64
just build-all   # all of the above
```

Individual macOS targets:

```sh
just build-apple-arm  # aarch64-apple-darwin
just build-apple-x86  # x86_64-apple-darwin
```

Output binaries are in `./result/bin/`.

### Tests

```sh
cargo test --manifest-path templar/Cargo.toml
```

### Lint

```sh
cargo clippy --manifest-path templar/Cargo.toml
```

## License

See [LICENSE](LICENSE).

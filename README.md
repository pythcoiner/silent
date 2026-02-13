# Silent

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
cargo test --manifest-path silent/Cargo.toml
```

### Lint

```sh
cargo clippy --manifest-path silent/Cargo.toml
```

## Vendored Dependencies

Crates.io dependencies are vendored in `silent/vendor/` for offline/reproducible
builds. Git dependencies are **not** vendored and are still fetched from GitHub:

| Git dependency | Repository | Crates pulled |
|---|---|---|
| bwk | `pythcoiner/bwk.git` | `bwk-sp` |
| spdk | `pythcoiner/spdk.git` | `spdk-core`, `backend-blindbit-native-non-async`, `bitcoin` (spdk's internal wrapper) |
| rust-silentpayments | `pythcoiner/rust-silentpayments.git` | `silentpayments` |
| ureq fork | `pythcoiner/ureq.git` | `ureq` (3.x with gzip branch) |

The configuration lives in `silent/.cargo/config.toml`.

### Re-vendor after changing dependencies

After adding, removing, or updating a crates.io dependency in `silent/Cargo.toml`:

```sh
cd silent
cargo vendor
```

This regenerates `vendor/`. `cargo vendor` also vendors git dependencies — those
must be removed since they are not meant to be vendored:

```sh
cd silent

# list what cargo vendor put from git sources:
ls -d vendor/bwk-sp vendor/spdk-core vendor/backend-blindbit-native-non-async \
      vendor/bitcoin-0.1.0 vendor/silentpayments vendor/ureq 2>/dev/null

# remove them:
rm -rf vendor/bwk-sp vendor/spdk-core vendor/backend-blindbit-native-non-async \
       vendor/bitcoin-0.1.0 vendor/silentpayments vendor/ureq
```

`cargo vendor` also prints a suggested `.cargo/config.toml` — **ignore it**,
the existing config is already correct (only crates-io is replaced, git sources
are left alone).

> **ureq note:** `vendor/ureq` (no version suffix) is the git fork — remove it.
> `vendor/ureq-2.12.1` is from crates.io — keep it.

> **Identifying git crates:** if the list above is outdated, check
> `silent/Cargo.lock` for entries with `source = "git+https://..."` — those are
> the git-sourced crates. The vendored directory name matches the crate name
> (with a version suffix when there are duplicates, e.g. `bitcoin-0.1.0` vs
> `bitcoin`).

### Moving a dependency between vendored and non-vendored

**Vendor a git dependency** (stop fetching from GitHub):

1. Add a `[source."git+<url>"]` section to `silent/.cargo/config.toml` with
   `replace-with = "vendored-sources"` (see the output of `cargo vendor` for
   the exact syntax).
2. Re-run `cargo vendor` and keep that crate in `vendor/`.

**Stop vendoring entirely** (fetch everything from the network):

Delete `silent/vendor/` and `silent/.cargo/config.toml`. Cargo will fetch from
crates.io and git as usual.


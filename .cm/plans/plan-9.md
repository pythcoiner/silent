# Phase 9: Nix Build System

## Objective

Add a Nix flake-based build system that reproducibly builds Silent from Linux, targeting Linux, Windows, and macOS. Use `pythcoiner/qt_static` (branch `6.6.3`) for statically linked Qt6 libraries to produce self-contained binaries.

## Background

Silent currently builds via `build.sh` (Rust) + CMake (C++), requiring manual setup of Qt6, Rust toolchain, and path dependencies (`../../bwk/sp`, `../../spdk/spdk-core`). A Nix flake makes this reproducible and enables cross-compilation to Windows and macOS from a Linux host.

The `pythcoiner/qt_static` repo provides a Nix flake that builds Qt 6.6.3 as static libraries for Linux, Windows (MinGW), macOS ARM, and macOS x86_64. Its outputs are consumed via `CMAKE_PREFIX_PATH`.

## Files to Read

- `build.sh` — Current build pipeline to replicate in Nix
- `CMakeLists.txt` — C++ build config, Qt6 dependencies, linked libraries
- `silent/Cargo.toml` — Rust dependencies, path deps to bwk and spdk
- `silent/build.rs` — CXX bridge build script (if exists)
- `lib/qontrol/CMakeLists.txt` — qontrol build config

## Implementation Steps

### Task 1: Create flake.nix with Linux build derivation

1. **Create `flake.nix`** (project root)
   - Define flake inputs:
     - `nixpkgs` (use a recent stable nixos release)
     - `qt_static` → `github:pythcoiner/qt_static/6.6.3`
     - `bwk` → `github:pythcoiner/bwk/master`
     - `spdk` → `github:pythcoiner/spdk/blindbit_backend_non_async`
     - `qontrol` → `github:pythcoiner/qontrol`
   - Set `system = "x86_64-linux"`

2. **Resolve Cargo path dependencies in Nix sandbox**
   - The Cargo.toml references `../../bwk/sp` and `../../spdk/spdk-core`
   - In the derivation, create a source layout that places:
     - `bwk` input at `../../bwk/` relative to the silent crate
     - `spdk` input at `../../spdk/` relative to the silent crate
   - Use `symlinkJoin` or copy sources into a build directory with the correct relative layout
   - Alternative: patch Cargo.toml in the derivation to use absolute paths

3. **Build Rust crate**
   - Use `rustPlatform.buildRustPackage` or manual `cargo build --release`
   - Ensure CXX bridge headers are generated (`target/cxxbridge/`)
   - Copy `libsilent.a`, `silent.h`, `cxx.h` to `lib/` (replicating build.sh)

4. **Build C++ GUI with static Qt6**
   - Use `qt_static` Linux output as `CMAKE_PREFIX_PATH`
   - Place `qontrol` source in `lib/qontrol/`
   - Run CMake build: `cmake -B build -DCMAKE_PREFIX_PATH=${qt6Static}` then `cmake --build build`
   - The CMakeLists.txt `RunBeforeBuild` target runs build.sh — either skip it (Rust already built) or set a flag to bypass

5. **Package the output**
   - Install the `silent` binary to `$out/bin/`
   - Add a `devShells.default` with Rust toolchain, Qt6, cmake, ninja for development

6. **Add `nix develop` shell**
   - Include: rustc, cargo, clippy, cmake, ninja, pkg-config, qt6Static
   - Export `CMAKE_PREFIX_PATH` pointing to qt_static Linux output

### Task 2: Add Windows and macOS cross-compilation targets

1. **Windows target (MinGW cross-compilation)**
   - Use `qt_static`'s Windows output (`packages.x86_64-linux.windows`)
   - Set up MinGW cross-compilation for C++ (`pkgs.pkgsCross.mingwW64`)
   - Configure Rust cross-compilation target `x86_64-pc-windows-gnu`
   - Build Rust with `--target x86_64-pc-windows-gnu`
   - Build C++ with MinGW CMake toolchain + qt_static Windows CMAKE_PREFIX_PATH
   - Output: `silent.exe`

2. **macOS ARM target (aarch64-apple-darwin)**
   - Use `qt_static`'s macOS ARM output (`packages.x86_64-linux.aarch64-apple-darwin`)
   - Set up macOS cross-compilation toolchain (clang + lld, Xcode SDK)
   - Configure Rust target `aarch64-apple-darwin`
   - Build C++ with macOS cross toolchain + qt_static macOS ARM CMAKE_PREFIX_PATH
   - Output: `silent` (macOS ARM binary)

3. **macOS x86_64 target**
   - Use `qt_static`'s macOS x86 output (`packages.x86_64-linux.x86_64-apple-darwin`)
   - Configure Rust target `x86_64-apple-darwin`
   - Build C++ with macOS cross toolchain + qt_static macOS x86 CMAKE_PREFIX_PATH
   - Output: `silent` (macOS x86 binary)

4. **Expose all targets as flake packages**
   - `packages.x86_64-linux.linux` — Linux build
   - `packages.x86_64-linux.windows` — Windows cross-build
   - `packages.x86_64-linux.aarch64-apple-darwin` — macOS ARM cross-build
   - `packages.x86_64-linux.x86_64-apple-darwin` — macOS x86 cross-build
   - `packages.x86_64-linux.default` — Linux build

### Task 3: Review and test Nix build pipeline

1. **Verify `nix flake check` passes**
2. **Verify `nix build .#linux` produces a working binary**
3. **Verify cross-compilation targets build without errors**
4. **Review flake structure and derivation quality**
5. **Update CLAUDE.md with Nix build commands**
   - Add section documenting `nix build .#linux`, `nix build .#windows`, etc.
   - Document `nix develop` for development shell
6. **Check that no unnecessary dependencies are included**

## Files to Create

- `flake.nix` — Main Nix flake definition

## Files to Modify

- `CLAUDE.md` — Add Nix build documentation section

## Verification

- [ ] `nix flake check` passes
- [ ] `nix build .#linux` produces a binary at `result/bin/silent`
- [ ] `nix build .#windows` produces `result/bin/silent.exe`
- [ ] `nix build .#aarch64-apple-darwin` produces a macOS ARM binary
- [ ] `nix build .#x86_64-apple-darwin` produces a macOS x86 binary
- [ ] `nix develop` provides a working development shell
- [ ] CLAUDE.md documents Nix build commands

## Reviewer Criteria

**Must check:**
- [ ] All flake inputs are correct (repos, branches)
- [ ] Cargo path dependencies resolve correctly in the Nix sandbox
- [ ] Static Qt6 linking works (no runtime Qt dependencies)
- [ ] Cross-compilation toolchains are properly configured
- [ ] Binary runs on target platform (at minimum, Linux verified)

**May skip:**
- [ ] Testing cross-compiled binaries on actual target platforms (build success is sufficient)
- [ ] Nix cache optimization

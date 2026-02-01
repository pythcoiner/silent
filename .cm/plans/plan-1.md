# Phase 1: Scaffolding

## Objective

Set up the project skeleton: CMake build system with Qt6 and qontrol, a Rust crate with CXX bridge to bwk-sp, and a build pipeline script.

## Files to Read

- `../qoinstr/CMakeLists.txt` - Reference CMake configuration with qontrol FetchContent
- `../qoinstr/cpp_joinstr/Cargo.toml` - Reference Rust crate dependencies
- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX bridge definitions
- `../qoinstr/cpp_joinstr/build.rs` - Reference CXX build script
- `../qoinstr/joinstr.sh` - Reference build pipeline script
- `../bwk/sp/Cargo.toml` - bwk-sp dependencies (the actual backend we use)

## Implementation Steps

### Task 1: Set up CMake project with Qt6 and qontrol FetchContent

1. **Create CMakeLists.txt** (project root)
   - cmake_minimum_required 3.22, project templar, CMAKE_CXX_STANDARD 20
   - Find Qt6 (Widgets, Gui, Core)
   - FetchContent_Declare qontrol from `https://github.com/pythcoiner/qontrol.git`
   - Custom target "RunBeforeBuild" that runs `./build.sh`
   - Add executable with `src/main.cpp`
   - Include `lib/include/` for generated CXX headers
   - Link against Qt6, qontrol, and `lib/libtemplar.a`
   - Link system libs: ssl, crypto, pthread, dl

2. **Create src/main.cpp** — Minimal Qt6 app entry point with placeholder QMainWindow

### Task 2: Create templar Rust crate with CXX bridge skeleton

1. **Create templar/Cargo.toml**
   - Package: templar, edition 2021
   - crate-type: ["rlib", "cdylib", "staticlib"]
   - Dependencies: bwk-sp (git = "https://github.com/pythcoiner/bwk.git"), cxx = "1.0", serde, serde_json, log, env_logger, dirs

2. **Create templar/build.rs** — cxx_build::bridge("src/lib.rs"), C++20 standard

3. **Create templar/src/lib.rs** — Minimal `#[cxx::bridge]` module with placeholder Account opaque type

### Task 3: Create build pipeline script (build.sh)

1. **Create build.sh** — cargo build --release, copy libtemplar.a and headers to lib/
2. **Update .gitignore** — Add build/, lib/, target/

## Files to Create

- `CMakeLists.txt`, `src/main.cpp`
- `templar/Cargo.toml`, `templar/build.rs`, `templar/src/lib.rs`
- `build.sh`

## Verification

- [ ] `cd templar && cargo build --release` succeeds
- [ ] `./build.sh` completes without errors
- [ ] `cmake -B build && cmake --build build` succeeds
- [ ] Application launches and shows empty window

## Reviewer Criteria

**Must check:**
- [ ] CMakeLists.txt correctly fetches qontrol from GitHub
- [ ] Cargo.toml depends on bwk-sp (NOT bwk or bwk_electrum)
- [ ] CXX bridge compiles with no warnings
- [ ] Build pipeline copies all required artifacts

# Phase 1: Scaffolding

## Objective

Set up the project skeleton: CMake build system with Qt6 and qontrol, a Rust crate with CXX bridge to bwk, and a build pipeline script that compiles Rust and links it into the C++ application.

## Files to Read

- `../qoinstr/CMakeLists.txt` - Reference CMake configuration with qontrol FetchContent
- `../qoinstr/cpp_joinstr/Cargo.toml` - Reference Rust crate dependencies
- `../qoinstr/cpp_joinstr/src/lib.rs` - Reference CXX bridge definitions
- `../qoinstr/cpp_joinstr/build.rs` - Reference CXX build script
- `../qoinstr/joinstr.sh` - Reference build pipeline script

## Implementation Steps

### Task 1: Set up CMake project with Qt6 and qontrol FetchContent

1. **Create CMakeLists.txt** (project root)
   - Set cmake_minimum_required to 3.22
   - Project name: templar
   - Set CMAKE_CXX_STANDARD to 20
   - Find Qt6 (Widgets, Gui, Core)
   - FetchContent_Declare qontrol from `https://github.com/pythcoiner/qontrol.git`
   - Add custom target "RunBeforeBuild" that runs `./build.sh`
   - Add executable with `src/main.cpp`
   - Include `lib/include/` for generated CXX headers
   - Link against Qt6, qontrol, and `lib/libtemplar.a`
   - Link system libs: ssl, crypto, pthread, dl

2. **Create src/main.cpp** (`src/main.cpp`)
   - Minimal Qt6 application entry point
   - Create QApplication
   - Show a placeholder QMainWindow
   - Return app.exec()

3. **Create .clang-format** (project root)
   - Copy formatting config from qoinstr or use a standard style

### Task 2: Create templar Rust crate with CXX bridge skeleton

1. **Create templar/Cargo.toml** (`templar/Cargo.toml`)
   - Package name: templar
   - Edition: 2021
   - crate-type: ["rlib", "cdylib", "staticlib"]
   - Dependencies:
     - bwk (git = "https://github.com/pythcoiner/bwk.git")
     - cxx = "1.0"
     - serde = { version = "1", features = ["derive"] }
     - serde_json = "1"
     - log = "0.4"
     - env_logger = "0.10"
     - dirs = "5"

2. **Create templar/build.rs** (`templar/build.rs`)
   - Use cxx_build::bridge("src/lib.rs")
   - Set C++ standard to "20"
   - Rerun on changes to src/*.rs

3. **Create templar/src/lib.rs** (`templar/src/lib.rs`)
   - Define `#[cxx::bridge]` module with placeholder types
   - Export a minimal `Account` opaque type
   - Export a `new_account` function that returns Box<Account>
   - Include basic enums: Network, LogLevel
   - This is the skeleton — full types added in Phase 2

### Task 3: Create build pipeline script (build.sh)

1. **Create build.sh** (project root)
   - Check for OFFLINE env variable
   - Run `cargo build --release --manifest-path templar/Cargo.toml`
   - Create `lib/` and `lib/include/` directories
   - Copy `templar/target/release/libtemplar.a` to `lib/`
   - Copy generated CXX headers from cargo build output to `lib/include/`
   - Make script executable (chmod +x)

2. **Create .gitignore updates**
   - Add build/, lib/, target/ to .gitignore

## Files to Create

- `CMakeLists.txt` - Build configuration
- `src/main.cpp` - Qt application entry point
- `templar/Cargo.toml` - Rust crate manifest
- `templar/build.rs` - CXX build script
- `templar/src/lib.rs` - CXX bridge skeleton
- `build.sh` - Build pipeline script

## Verification

- [ ] `cd templar && cargo build --release` succeeds
- [ ] `./build.sh` completes without errors
- [ ] `lib/libtemplar.a` exists after build
- [ ] `lib/include/templar.h` exists after build
- [ ] `cmake -B build && cmake --build build` succeeds (after build.sh)
- [ ] Application launches and shows empty window

## Reviewer Criteria

**Must check:**
- [ ] CMakeLists.txt correctly fetches qontrol from GitHub
- [ ] CXX bridge compiles with no warnings
- [ ] Build pipeline copies all required artifacts
- [ ] .gitignore excludes build artifacts

**May skip:**
- [ ] Code formatting configuration details

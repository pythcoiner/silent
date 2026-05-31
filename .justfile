qontrol_repo := "https://github.com/pythcoiner/qontrol.git"
qontrol_commit := "7bc317d"

nix := env("NIX", "nix")
nix_flags := "--extra-experimental-features 'nix-command flakes'"

# Build Linux
build:
    {{nix}} {{nix_flags}} build .#linux

# Build all targets (Linux + Windows + macOS)
build-all: build build-win build-apple

# Build Windows (MinGW cross-compilation)
build-win:
    {{nix}} {{nix_flags}} build .#windows

# Build macOS (ARM + x86_64)
build-apple:
    {{nix}} {{nix_flags}} build .#aarch64-apple-darwin
    {{nix}} {{nix_flags}} build .#x86_64-apple-darwin

# Build macOS ARM only
build-apple-arm:
    {{nix}} {{nix_flags}} build .#aarch64-apple-darwin

# Build macOS x86_64 only
build-apple-x86:
    {{nix}} {{nix_flags}} build .#x86_64-apple-darwin

# Fetch and build qontrol into lib/qontrol
qontrol:
    rm -rf lib/qontrol
    git clone {{qontrol_repo}} lib/qontrol
    cd lib/qontrol && git checkout {{qontrol_commit}}
    rm -rf lib/qontrol/.git

# Build the Rust binding (runs build.sh)
binding:
    bash build.sh

# Build the full project locally (binding + cmake, no nix)
build-local: binding
    cmake -B build . && cmake --build build -j$(nproc)

# Run silent
run:
    ./build/silent

# Build locally + run
br: build-local
    just run

# Format all C++ source files
format:
    find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i

# Lint Rust binding
lint-rust:
    cargo fmt --manifest-path silent/Cargo.toml -- --check
    cargo clippy --manifest-path silent/Cargo.toml -- -D warnings

# Lint Rust on each commit in a range
lint-rust-commits range="HEAD~1...HEAD":
    scripts/lint/rust_lint_commits.sh {{range}}

# Lint full C++ app codebase (clang-tidy)
lint-cpp-full:
    if [ ! -f lib/include/silent.h ]; then just binding; fi
    if [ ! -f build/compile_commands.json ]; then cmake -B build .; fi
    bash -o pipefail -c "start=\$(date +%s%3N); if command -v run-clang-tidy >/dev/null 2>&1; then run-clang-tidy -p build -j\$(nproc) -quiet -header-filter='^src/.*' -source-filter='^src/.*' 'src/.*\\.cpp$'; else find src -type f -name '*.cpp' ! -path 'src/resources/*' -print0 | xargs -0 -P\$(nproc) -n1 clang-tidy -p build --quiet --warnings-as-errors='*' --header-filter='^src/.*' --exclude-header-filter='^src/resources/.*'; fi 2>&1 | { rg -v '^[0-9]+ warnings generated\\.$' || true; }; rc=\${PIPESTATUS[0]}; end=\$(date +%s%3N); elapsed=\$((end-start)); printf 'Lint full (%s files) in %d.%03dS\n' \"\$(find src -type f -name '*.cpp' ! -path 'src/resources/*' | wc -l)\" \$((elapsed/1000)) \$((elapsed%1000)); exit \$rc"

# Lint C++ only on per-commit diff
lint-cpp:
    if [ ! -f build/compile_commands.json ]; then cmake -B build .; fi
    scripts/lint/clang_tidy_diff_commits.sh "HEAD~1...HEAD" --worktree

# Ultra-fast local C++ lint on uncommitted changes only
lint-cpp-fast:
    if [ ! -f build/compile_commands.json ]; then cmake -B build .; fi
    scripts/lint/clang_tidy_diff_commits.sh "HEAD~1...HEAD" --worktree --worktree-only

# Fast lint (Rust + C++ commit diff)
lint: lint-rust lint-cpp

# Full-repo lint (Rust + full C++)
lint-full: lint-rust lint-cpp-full

# Auto-fix clang-tidy warnings
fix:
    find src -name '*.cpp' | xargs -P$(nproc) -n1 clang-tidy -p build --header-filter='^.*/silent/src/.*' --quiet --fix

# Clean all build artifacts (Rust + C++)
clean:
    cargo clean --manifest-path silent/Cargo.toml
    rm -rf build
    rm -f lib/libsilent.* lib/libtemplar.*
    rm -rf lib/include

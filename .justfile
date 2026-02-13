qontrol_repo := "https://github.com/pythcoiner/qontrol.git"
qontrol_commit := "108db79"

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

# Lint C++ source files (clang-tidy)
lint:
    find src -name '*.cpp' | xargs -P$(nproc) -n1 clang-tidy -p build --header-filter='^.*/silent/src/.*' --quiet

# Auto-fix clang-tidy warnings
fix:
    find src -name '*.cpp' | xargs -P$(nproc) -n1 clang-tidy -p build --header-filter='^.*/silent/src/.*' --quiet --fix

# Clean all build artifacts (Rust + C++)
clean:
    cargo clean --manifest-path silent/Cargo.toml
    rm -rf build
    rm -f lib/libsilent.* lib/libtemplar.*
    rm -rf lib/include

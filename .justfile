qontrol_repo := "https://github.com/pythcoiner/qontrol.git"
qontrol_commit := "4444fc3"

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
    cmake -B build . && cmake --build build

# Run silent
run:
    ./build/silent

# Build locally + run
br: build-local
    just run

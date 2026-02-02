qontrol_repo := "https://github.com/pythcoiner/qontrol.git"
qontrol_commit := "4444fc3"

# Build the full project (binding + cmake)
build: binding
    cmake -B build . && cmake --build build

# Fetch and build qontrol into lib/qontrol
qontrol:
    rm -rf lib/qontrol
    git clone {{qontrol_repo}} lib/qontrol
    cd lib/qontrol && git checkout {{qontrol_commit}}
    rm -rf lib/qontrol/.git

# Build the Rust binding (runs build.sh)
binding:
    bash build.sh

# Run templar
run:
    ./build/templar

# Build + run
br: build
    just run

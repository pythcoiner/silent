#!/bin/bash

echo "OFFLINE=$OFFLINE"

set -e

# Cleanup lib (preserve vendored qontrol)
rm -f ./lib/libsilent.*
rm -rf ./lib/include
mkdir -p ./lib/include

# Build silent Rust crate
cd silent
if [ -z "$OFFLINE" ]; then
    OFFLINE=false
fi
if [ "$OFFLINE" = false ]; then
    cargo build --release
else
    cargo build --release --offline
fi

cd ..

# Copy CXX bridge generated headers into ./lib/include/
cp -L ./silent/target/cxxbridge/silent/src/lib.rs.h ./lib/include/silent.h
cp -L ./silent/target/cxxbridge/rust/cxx.h ./lib/include/cxx.h

# Copy static library into ./lib/
cp ./silent/target/release/libsilent.a ./lib/libsilent.a
cp ./silent/target/release/libsilent.rlib ./lib/libsilent.rlib
cp ./silent/target/release/libsilent.d ./lib/libsilent.d

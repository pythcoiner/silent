#!/bin/bash

echo "OFFLINE=$OFFLINE"

set -e

# Cleanup lib (preserve vendored qontrol)
rm -f ./lib/libtemplar.*
rm -rf ./lib/include
mkdir -p ./lib/include

# Build templar Rust crate
cd templar
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
cp -L ./templar/target/cxxbridge/templar/src/lib.rs.h ./lib/include/templar.h
cp -L ./templar/target/cxxbridge/rust/cxx.h ./lib/include/cxx.h

# Copy static library into ./lib/
cp ./templar/target/release/libtemplar.a ./lib/libtemplar.a
cp ./templar/target/release/libtemplar.rlib ./lib/libtemplar.rlib
cp ./templar/target/release/libtemplar.so ./lib/libtemplar.so
cp ./templar/target/release/libtemplar.d ./lib/libtemplar.d

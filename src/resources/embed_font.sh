#!/usr/bin/env bash
# Converts a .ttf font file into a C++ header with a constexpr byte array.
#
# Usage: ./embed_font.sh <input.ttf> <output.h> <variable_name>
# Example: ./embed_font.sh NotoSans-Regular.ttf font/noto_sans.h NOTO_SANS

set -euo pipefail

if [ $# -ne 3 ]; then
    echo "Usage: $0 <input.ttf> <output.h> <variable_name>"
    exit 1
fi

INPUT="$1"
OUTPUT="$2"
VARNAME="$3"

if [ ! -f "$INPUT" ]; then
    echo "Error: $INPUT not found"
    exit 1
fi

SIZE=$(wc -c < "$INPUT")

{
    echo "#pragma once"
    echo ""
    echo "// Auto-generated from $(basename "$INPUT") — do not edit"
    echo "// Size: $SIZE bytes"
    echo ""
    echo "#include <cstddef>"
    echo "#include <cstdint>"
    echo ""
    echo "namespace embedded_font {"
    echo ""
    echo "constexpr size_t ${VARNAME}_SIZE = ${SIZE};"
    echo ""
    echo "alignas(4) constexpr uint8_t ${VARNAME}[] = {"
    xxd -i < "$INPUT" | sed 's/^/    /'
    echo "};"
    echo ""
    echo "} // namespace embedded_font"
} > "$OUTPUT"

echo "Generated $OUTPUT ($SIZE bytes)"

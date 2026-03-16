#!/usr/bin/env bash
# Downloads ALL Lucide icons and generates one C++ header per icon.
#
# Usage: ./fetch_icons.sh
# Output:
#   src/resources/icon/<name>.h     — one per SVG
#   src/resources/icon/icons.h      — master include

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ICON_DIR="$SCRIPT_DIR/icon"
TEMP_DIR=$(mktemp -d)

cleanup() { rm -rf "$TEMP_DIR"; }
trap cleanup EXIT

echo "Cloning lucide-icons/lucide (shallow, icons/ only)..."
git clone --depth 1 --filter=blob:none --sparse \
    https://github.com/lucide-icons/lucide.git "$TEMP_DIR/lucide" 2>/dev/null
cd "$TEMP_DIR/lucide"
git sparse-checkout set icons
cd - > /dev/null

SVG_DIR="$TEMP_DIR/lucide/icons"

if [ ! -d "$SVG_DIR" ]; then
    echo "Error: icons directory not found in cloned repo"
    exit 1
fi

# Clean previous generated headers
rm -rf "$ICON_DIR"
mkdir -p "$ICON_DIR"

COUNT=0
INCLUDES=""

for svg_file in "$SVG_DIR"/*.svg; do
    [ -f "$svg_file" ] || continue

    BASENAME=$(basename "$svg_file" .svg)

    # kebab-case → snake_case for filename and variable
    SNAKE=$(echo "$BASENAME" | tr '-' '_')
    UPPER=$(echo "$SNAKE" | tr '[:lower:]' '[:upper:]')

    SVG_CONTENT=$(cat "$svg_file")

    {
        echo "#pragma once"
        echo ""
        echo "// Auto-generated from ${BASENAME}.svg (Lucide Icons) — do not edit"
        echo ""
        echo "namespace embedded_icon {"
        echo ""
        echo "constexpr char ${UPPER}[] = R\"svg(${SVG_CONTENT})svg\";"
        echo ""
        echo "} // namespace embedded_icon"
    } > "$ICON_DIR/${SNAKE}.h"

    INCLUDES="${INCLUDES}#include \"${SNAKE}.h\"\n"
    COUNT=$((COUNT + 1))
done

# Generate master include header
{
    echo "#pragma once"
    echo ""
    echo "// Auto-generated — do not edit. Re-run fetch_icons.sh to update."
    echo ""
    echo -e "$INCLUDES"
} > "$ICON_DIR/icons.h"

echo "Generated $COUNT icon headers in $ICON_DIR"

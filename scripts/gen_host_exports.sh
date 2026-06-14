#!/usr/bin/env bash
# Generate a Windows (PE) export .def for the silent host binary.
#
# Plugins are built as DLLs that link only against the host import library, so a
# single static Qt lives in the host exe and plugins resolve the host ABI from it.
# PE export tables are capped at 64K ordinals, and a full static Qt (all modules)
# contributes >72K symbols, so the whole image cannot be exported. We export the
# stable plugin ABI instead:
#
#   tier 1: every symbol the host objects define (interfaces + host ABI)
#   tier 2: every symbol qontrol defines (the full plugin UI framework)
#   tier 3: the full QtCore/QtGui/QtWidgets/QtSvg ABI (the Qt modules plugins
#           build against), so a plugin may use any of those modules and not just
#           the subset the host happens to call
#
# The host CMake whole-archives those Qt modules + qontrol so every listed symbol
# is present in the image. The set is recomputed from the libraries every build,
# so it stays in sync with no manual maintenance. A plugin reaching outside these
# modules (e.g. QtNetwork) fails to link, which is the intended ABI boundary.
#
# Usage: gen_host_exports.sh <nm> <qontrol_lib> <qt_lib_dir> <out.def> <obj>...
set -euo pipefail

if [ "$#" -lt 5 ]; then
    echo "usage: $0 <nm> <qontrol_lib> <qt_lib_dir> <out.def> <obj>..." >&2
    exit 2
fi

nm_bin="$1"
qontrol_lib="$2"
qt_lib_dir="$3"
out_def="$4"
shift 4

tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT

# nm prints "<addr> <type> <name>" for defined symbols; the name is the last field.
names_defined() { "$nm_bin" -g --defined-only "$@" 2>/dev/null | awk 'NF >= 2 { print $NF }'; }

# The Qt modules a plugin builds against. Restricted to keep the export table
# under the PE 64K cap; exporting every Qt module would overflow it.
qt_archives=()
for m in Core Gui Widgets Svg; do
    archive="$qt_lib_dir/libQt6$m.a"
    if [ ! -f "$archive" ]; then
        echo "gen_host_exports: missing Qt archive $archive" >&2
        exit 1
    fi
    qt_archives+=("$archive")
done

# Tier 1 + 2 + 3: host + qontrol defined symbols, plus the full Qt module ABI.
names_defined "$@" "$qontrol_lib" "${qt_archives[@]}" | grep . | sort -u > "$tmp/all"

# Drop compiler-internal helper symbols (indirection thunks, DWARF refs, TLS
# emulation): they are not part of any ABI and only add export-table noise.
grep -vE '^\.|^DW\.ref\.|^__emutls_' "$tmp/all" > "$tmp/exports"

{
    echo "EXPORTS"
    cat "$tmp/exports"
} > "$out_def"

echo "gen_host_exports: $(wc -l < "$tmp/exports") symbols exported -> $out_def"

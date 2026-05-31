#!/usr/bin/env python3

"""Validate i18n IDs used in Qt sources.

Checks:
1) Extracts `tr("...")` and `qtTrId("...")` literals from C++ sources.
2) Verifies ID format:
   - lowercase unicode letters
   - digits
   - hyphen (`-`)
3) Verifies each ID exists in the English TS catalog.

By default, only `qtTrId("...")` values are treated as required IDs.
Use `--strict-tr` to enforce the same ID rules for every `tr("...")` string.
"""

from __future__ import annotations

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path


TR_RE = re.compile(r"\btr\(\s*\"((?:\\.|[^\"\\])*)\"")
TRID_RE = re.compile(r"\bqtTrId\(\s*\"((?:\\.|[^\"\\])*)\"")

CPP_SUFFIXES = {".h", ".hpp", ".hh", ".c", ".cc", ".cpp", ".cxx"}


def decode_cpp_string(s: str) -> str:
    # Keep this intentionally conservative for common escaped forms.
    return bytes(s, "utf-8").decode("unicode_escape")


def is_valid_i18n_id(value: str) -> bool:
    if not value:
        return False
    for ch in value:
        if ch == "-":
            continue
        if ch.isdigit():
            continue
        if ch.isalpha() and ch == ch.lower():
            continue
        return False
    return True


def collect_catalog_keys(ts_path: Path) -> set[str]:
    tree = ET.parse(ts_path)
    root = tree.getroot()
    keys: set[str] = set()
    for msg in root.iter("message"):
        msg_id = msg.attrib.get("id", "").strip()
        if msg_id:
            keys.add(msg_id)
        source = msg.findtext("source", default="").strip()
        if source:
            keys.add(source)
    return keys


def iter_source_files(root: Path) -> list[Path]:
    files: list[Path] = []
    for p in root.rglob("*"):
        if p.suffix in CPP_SUFFIXES and p.is_file():
            files.append(p)
    return files


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--src", default="src", help="Source directory to scan")
    parser.add_argument("--en-ts", default="i18n/silent_en.ts", help="English TS catalog")
    parser.add_argument(
        "--strict-tr",
        action="store_true",
        help="Treat tr(\"...\") values as IDs and enforce ID format + catalog presence",
    )
    args = parser.parse_args()

    src_root = Path(args.src)
    ts_path = Path(args.en_ts)

    if not src_root.exists():
        print(f"error: source path does not exist: {src_root}")
        return 2
    if not ts_path.exists():
        print(f"error: english TS file does not exist: {ts_path}")
        return 2

    catalog = collect_catalog_keys(ts_path)
    errors: list[str] = []

    for file_path in iter_source_files(src_root):
        text = file_path.read_text(encoding="utf-8", errors="replace")

        for m in TRID_RE.finditer(text):
            raw = decode_cpp_string(m.group(1))
            line = text.count("\n", 0, m.start()) + 1
            if not is_valid_i18n_id(raw):
                errors.append(
                    f"{file_path}:{line}: invalid qtTrId '{raw}' "
                    "(allowed: lowercase unicode letters, digits, '-')"
                )
            elif raw not in catalog:
                errors.append(f"{file_path}:{line}: missing id in {ts_path}: '{raw}'")

        if args.strict_tr:
            for m in TR_RE.finditer(text):
                raw = decode_cpp_string(m.group(1))
                line = text.count("\n", 0, m.start()) + 1
                if not is_valid_i18n_id(raw):
                    errors.append(
                        f"{file_path}:{line}: invalid tr id '{raw}' "
                        "(allowed: lowercase unicode letters, digits, '-')"
                    )
                elif raw not in catalog:
                    errors.append(f"{file_path}:{line}: missing id in {ts_path}: '{raw}'")

    if errors:
        print("i18n id check failed:")
        for err in errors:
            print(f"- {err}")
        return 1

    mode = "strict-tr" if args.strict_tr else "qtTrId-only"
    print(f"i18n id check passed ({mode})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

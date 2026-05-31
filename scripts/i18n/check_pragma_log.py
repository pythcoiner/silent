#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

ID_RE = re.compile(r"I18N_ID:([a-z0-9\-]+)")
IDN_RE = re.compile(r"I18N_IDN:([a-z0-9\-]+)")


def load_catalog(ts_file: Path) -> dict[str, bool]:
    tree = ET.parse(ts_file)
    root = tree.getroot()
    out: dict[str, bool] = {}
    for msg in root.iter("message"):
        msg_id = msg.attrib.get("id", "").strip()
        if not msg_id:
            continue
        is_plural = msg.attrib.get("numerus", "no") == "yes"
        out[msg_id] = is_plural
    return out


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--log", required=True, help="Compiler log file to parse")
    parser.add_argument("--en-ts", default="i18n/silent_en.ts", help="English TS file")
    args = parser.parse_args()

    log_path = Path(args.log)
    ts_path = Path(args.en_ts)

    if not log_path.exists():
        print(f"error: log file not found: {log_path}")
        return 2
    if not ts_path.exists():
        print(f"error: english TS file not found: {ts_path}")
        return 2

    text = log_path.read_text(encoding="utf-8", errors="replace")
    used_singular = set(ID_RE.findall(text))
    used_plural = set(IDN_RE.findall(text))
    catalog = load_catalog(ts_path)

    failures: list[str] = []
    warnings: list[str] = []

    for msg_id in sorted(used_singular):
        if msg_id not in catalog:
            failures.append(f"missing id in {ts_path}: {msg_id}")
            continue
        if catalog[msg_id]:
            failures.append(f"id used with TR() but catalog entry is plural: {msg_id}")

    for msg_id in sorted(used_plural):
        if msg_id not in catalog:
            failures.append(f"missing plural id in {ts_path}: {msg_id}")
            continue
        if not catalog[msg_id]:
            failures.append(f"id used with TRN() but catalog entry is non-plural: {msg_id}")

    used_any = used_singular | used_plural
    unused = sorted(set(catalog.keys()) - used_any)
    for msg_id in unused:
        warnings.append(f"unused id in {ts_path}: {msg_id}")

    if warnings:
        print("i18n warnings:")
        for warn in warnings:
            print(f"- {warn}")

    if failures:
        print("i18n pragma check failed:")
        for failure in failures:
            print(f"- {failure}")
        return 1

    print("i18n pragma check passed")
    print(f"singular ids used: {len(used_singular)}")
    print(f"plural ids used: {len(used_plural)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

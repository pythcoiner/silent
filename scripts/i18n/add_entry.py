#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
from pathlib import Path


ID_RE = re.compile(r"^[a-z0-9\-]+$")
ENTRY_RE = re.compile(r'^(?P<id>[a-z0-9\-]+)\s*=>')


def append_if_missing(file_path: Path, entry_lines: list[str], msg_id: str) -> None:
    lines = file_path.read_text(encoding="utf-8").splitlines()
    for raw in lines:
        stripped = raw.strip()
        if not stripped or stripped.startswith("#"):
            continue
        match = ENTRY_RE.match(stripped)
        if match and match.group("id") == msg_id:
            raise ValueError(f"{file_path}: id '{msg_id}' already exists")

    if lines and lines[-1] != "":
        lines.append("")
    lines.extend(entry_lines)
    file_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def escape_text(value: str) -> str:
    return value.replace("\\", "\\\\").replace("\n", "\\n").replace('"', '\\"')


def format_single(msg_id: str, english_text: str) -> list[str]:
    escaped_en = escape_text(english_text)
    inline = f'{msg_id} => "{escaped_en}"'
    if len(inline) > 85:
        return [msg_id, f'=> "{escaped_en}";;']
    return [inline]


def format_pair(msg_id: str, english_text: str, localized_text: str) -> list[str]:
    escaped_en = escape_text(english_text)
    escaped_localized = escape_text(localized_text)
    inline = f'{msg_id} => "{escaped_en}" => "{escaped_localized}"'
    if len(inline) > 85:
        return [msg_id, f'=> "{escaped_en}"', f'=> "{escaped_localized}";;']
    return [inline]


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Add a new i18n ID to english and placeholder NONE to locales"
    )
    parser.add_argument("msg_id", help="New translation id (lowercase, digits, hyphen)")
    parser.add_argument("english_text", help="English source text")
    parser.add_argument("--lang-dir", default="i18n")
    args = parser.parse_args()

    if ID_RE.match(args.msg_id) is None:
        raise ValueError("msg_id must use lowercase letters, digits, and hyphen only")

    repo_root = Path(__file__).resolve().parent.parent.parent
    lang_dir = (repo_root / args.lang_dir).resolve()

    en_file = lang_dir / "silent_en.lang"
    if not en_file.exists():
        raise FileNotFoundError(en_file)

    append_if_missing(en_file, format_single(args.msg_id, args.english_text), args.msg_id)

    for locale_file in sorted(lang_dir.glob("silent_*.lang")):
        if locale_file.name == "silent_en.lang":
            continue
        append_if_missing(
            locale_file,
            format_pair(args.msg_id, args.english_text, "NONE"),
            args.msg_id,
        )

    print(f"Added id '{args.msg_id}' to {en_file.name} and locale placeholders")
    print("Run: python3 scripts/i18n/sync_lang.py --lang-dir i18n")
    print("Run: just build-local")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
from pathlib import Path


INLINE_RE = re.compile(r'^(?P<id>[a-z0-9\-]+)\s*=>\s*(?P<rest>.+)$')
BLOCK_START_RE = re.compile(r'^(?P<id>[a-z0-9\-]+)\s*$')
BLOCK_VALUE_RE = re.compile(r'^=>\s*"(?P<text>(?:\\.|[^"\\])*)"\s*(?P<end>;;)?\s*$')
QUOTED_RE = re.compile(r'"((?:\\.|[^"\\])*)"')


def escape_text(value: str) -> str:
    return value.replace("\\", "\\\\").replace("\n", "\\n").replace('"', '\\"')


def unescape_text(value: str) -> str:
    out: list[str] = []
    i = 0
    while i < len(value):
        ch = value[i]
        if ch != "\\" or i + 1 >= len(value):
            out.append(ch)
            i += 1
            continue

        nxt = value[i + 1]
        if nxt == "n":
            out.append("\n")
        elif nxt == "t":
            out.append("\t")
        else:
            out.append(nxt)
        i += 2
    return "".join(out)


def parse_lang(file_path: Path) -> dict[str, tuple[str, str]]:
    result: dict[str, tuple[str, str]] = {}
    lines = file_path.read_text(encoding="utf-8").splitlines()
    idx = 1
    while idx <= len(lines):
        raw = lines[idx - 1]
        line = raw.strip()
        idx += 1
        if not line or line.startswith("#"):
            continue

        match = INLINE_RE.match(line)
        if match is not None:
            msg_id = match.group("id")
            quoted = QUOTED_RE.findall(match.group("rest"))
            if len(quoted) == 1:
                text = unescape_text(quoted[0])
                result[msg_id] = (text, text)
            elif len(quoted) == 2:
                en_text = unescape_text(quoted[0])
                localized = unescape_text(quoted[1])
                result[msg_id] = (en_text, localized)
            else:
                raise ValueError(
                    f"{file_path}:{idx - 1}: expected one or two quoted values (english and translation): {raw}"
                )
            continue

        start = BLOCK_START_RE.match(line)
        if start is None:
            raise ValueError(f"{file_path}:{idx - 1}: invalid line: {raw}")

        msg_id = start.group("id")
        values: list[str] = []
        ended = False
        while idx <= len(lines):
            next_raw = lines[idx - 1]
            next_line = next_raw.strip()
            idx += 1
            if not next_line or next_line.startswith("#"):
                continue
            value_match = BLOCK_VALUE_RE.match(next_line)
            if value_match is None:
                raise ValueError(f"{file_path}:{idx - 1}: invalid multiline value: {next_raw}")
            values.append(unescape_text(value_match.group("text")))
            if value_match.group("end") == ";;":
                ended = True
                break

        if not ended:
            raise ValueError(f"{file_path}:{idx - 1}: unterminated multiline entry for id '{msg_id}'")
        if len(values) == 1:
            text = values[0]
            result[msg_id] = (text, text)
        elif len(values) == 2:
            result[msg_id] = (values[0], values[1])
        else:
            raise ValueError(
                f"{file_path}:{idx - 1}: expected one or two multiline values (english and translation) for id '{msg_id}'"
            )

    return result


def render_lang(merged: dict[str, tuple[str, str]]) -> str:
    lines = [
        "# Translator-friendly catalog",
        "# Format: id => \"English\" => \"Translation\"",
        "# Use \\n for line breaks",
        "",
    ]
    for msg_id in sorted(merged.keys()):
        en_text, localized = merged[msg_id]
        inline = f'{msg_id} => "{escape_text(en_text)}" => "{escape_text(localized)}"'
        if len(inline) > 85:
            lines.append(msg_id)
            lines.append(f'=> "{escape_text(en_text)}"')
            lines.append(f'=> "{escape_text(localized)}";;')
        else:
            lines.append(inline)
    return "\n".join(lines) + "\n"


def sync_all(lang_dir: Path, check: bool) -> bool:
    en_path = lang_dir / "silent_en.lang"
    en_entries = parse_lang(en_path)
    en_map = {msg_id: values[1] for msg_id, values in en_entries.items()}
    clean = True

    for lang_path in sorted(lang_dir.glob("silent_*.lang")):
        if lang_path.name == "silent_en.lang":
            continue
        existing = parse_lang(lang_path)
        merged: dict[str, tuple[str, str]] = {}
        for msg_id, en_text in en_map.items():
            localized = existing.get(msg_id, (en_text, en_text))[1]
            merged[msg_id] = (en_text, localized)
        rendered = render_lang(merged)
        current = lang_path.read_text(encoding="utf-8")
        if current != rendered:
            clean = False
            if check:
                print(f"outdated: {lang_path}")
            else:
                lang_path.write_text(rendered, encoding="utf-8")

    return clean


def main() -> int:
    parser = argparse.ArgumentParser(description="Sync locale .lang files with english source text")
    parser.add_argument("--lang-dir", default="i18n")
    parser.add_argument("--check", action="store_true")
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent.parent
    lang_dir = (repo_root / args.lang_dir).resolve()
    clean = sync_all(lang_dir, check=args.check)
    if args.check and not clean:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

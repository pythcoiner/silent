#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
import sys
import xml.etree.ElementTree as ET
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


def load_lang(file_path: Path) -> dict[str, str]:
    result: dict[str, str] = {}
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
            if len(quoted) not in (1, 2):
                raise ValueError(
                    f"{file_path}:{idx - 1}: expected one or two quoted values (english and translation): {raw}"
                )
            text = unescape_text(quoted[-1])
            if text != "NONE":
                result[msg_id] = text
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
        if len(values) not in (1, 2):
            raise ValueError(
                f"{file_path}:{idx - 1}: expected one or two multiline values (english and translation) for id '{msg_id}'"
            )

        if values[-1] != "NONE":
            result[msg_id] = values[-1]

    return result


def write_lang(file_path: Path, messages: dict[str, str]) -> None:
    lines = [
        "# Translator-friendly catalog",
        "# Format: id => \"Text\"",
        "# Use \\n for line breaks",
        "",
    ]
    for msg_id in sorted(messages.keys()):
        lines.append(f'{msg_id} => "{escape_text(messages[msg_id])}"')
    file_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def load_ts(file_path: Path) -> dict[str, str]:
    tree = ET.parse(file_path)
    root = tree.getroot()
    messages: dict[str, str] = {}
    for msg in root.iter("message"):
        msg_id = msg.attrib.get("id", "").strip()
        if not msg_id:
            continue
        translation = msg.find("translation")
        text = "" if translation is None else (translation.text or "")
        messages[msg_id] = text
    return messages


def write_ts(file_path: Path, language: str, source_language: str, messages: dict[str, str]) -> None:
    root = ET.Element("TS", attrib={"version": "2.1", "language": language, "sourceLanguage": source_language})
    context = ET.SubElement(root, "context")
    name = ET.SubElement(context, "name")
    name.text = "i18n"

    for msg_id in sorted(messages.keys()):
        msg = ET.SubElement(context, "message", attrib={"id": msg_id})
        source = ET.SubElement(msg, "source")
        source.text = msg_id
        translation = ET.SubElement(msg, "translation")
        translation.text = messages[msg_id]

    ET.indent(root, space="  ")
    xml = ET.tostring(root, encoding="unicode")
    file_path.write_text('<?xml version="1.0" encoding="utf-8"?>\n<!DOCTYPE TS>\n' + xml + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description="Convert between .lang and Qt .ts catalogs")
    sub = parser.add_subparsers(dest="command", required=True)

    to_ts = sub.add_parser("to-ts", help="Generate .ts from .lang")
    to_ts.add_argument("--lang-file", required=True)
    to_ts.add_argument("--ts-file", required=True)
    to_ts.add_argument("--language", required=True)
    to_ts.add_argument("--source-language", default="en")

    to_lang = sub.add_parser("to-lang", help="Generate .lang from .ts")
    to_lang.add_argument("--ts-file", required=True)
    to_lang.add_argument("--lang-file", required=True)

    args = parser.parse_args()

    if args.command == "to-ts":
        messages = load_lang(Path(args.lang_file))
        write_ts(Path(args.ts_file), args.language, args.source_language, messages)
        return 0

    messages = load_ts(Path(args.ts_file))
    write_lang(Path(args.lang_file), messages)
    return 0


if __name__ == "__main__":
    sys.exit(main())

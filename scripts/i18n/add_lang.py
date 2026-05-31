#!/usr/bin/env python3

from __future__ import annotations

import argparse
import re
from pathlib import Path


ID_RE = re.compile(r'^(?P<id>[a-z0-9\-]+)\s*=>\s*"(?P<text>(?:\\.|[^"\\])*)"\s*$')


def validate_locale(locale: str) -> None:
    if not locale:
        raise ValueError("locale must not be empty")
    for ch in locale:
        if not (ch.islower() or ch.isdigit() or ch in {"_", "-"}):
            raise ValueError("locale must be lowercase and use [a-z0-9_-]")


def create_lang_file(repo_root: Path, locale: str) -> Path:
    i18n_dir = repo_root / "i18n"
    src = i18n_dir / "silent_en.lang"
    dst = i18n_dir / f"silent_{locale}.lang"

    if dst.exists():
        raise FileExistsError(f"{dst} already exists")

    lines: list[str] = []
    for raw in src.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        match = ID_RE.match(line)
        if match is None:
            lines.append(raw)
            continue
        msg_id = match.group("id")
        text = match.group("text")
        lines.append(f'{msg_id} => "{text}" => "{text}"')
    dst.write_text("\n".join(lines) + "\n", encoding="utf-8")
    return dst


def patch_settings_selector(repo_root: Path, locale: str, display_name: str) -> bool:
    settings_cpp = repo_root / "src" / "screens" / "Settings.cpp"
    text = settings_cpp.read_text(encoding="utf-8")

    locale_marker = f'"{locale}"'
    if locale_marker in text:
        return False

    anchor = '    m_language_selector->addItem("Espanol", "es");\n'
    insertion = f'    m_language_selector->addItem("{display_name}", "{locale}");\n'

    if anchor not in text:
        raise RuntimeError("could not find language selector anchor in Settings.cpp")

    settings_cpp.write_text(text.replace(anchor, insertion + anchor), encoding="utf-8")
    return True


def main() -> int:
    parser = argparse.ArgumentParser(description="Add a new i18n language boilerplate")
    parser.add_argument("locale", help="Locale code, e.g. fr, it, de, pt_BR")
    parser.add_argument("display_name", help="Autonym shown in language selector")
    args = parser.parse_args()

    validate_locale(args.locale)
    repo_root = Path(__file__).resolve().parent.parent.parent

    new_file = create_lang_file(repo_root, args.locale)
    updated_selector = patch_settings_selector(repo_root, args.locale, args.display_name)

    print(f"Created {new_file.relative_to(repo_root)}")
    if updated_selector:
        print("Updated src/screens/Settings.cpp language selector")
    else:
        print("Settings selector already had this locale; no change")

    print("Run: just build-local")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

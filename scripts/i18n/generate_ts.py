#!/usr/bin/env python3

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path


CATALOGS = [
    ("silent_en.lang", "silent_en.ts", "en_US", "en"),
    ("silent_fr.lang", "silent_fr.ts", "fr", "en"),
    ("silent_it.lang", "silent_it.ts", "it", "en"),
    ("silent_de.lang", "silent_de.ts", "de", "en"),
    ("silent_pt.lang", "silent_pt.ts", "pt", "en"),
    ("silent_es.lang", "silent_es.ts", "es", "en"),
]


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate all i18n TS catalogs from LANG files")
    parser.add_argument("--lang-dir", default="i18n")
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()

    repo_root = Path(__file__).resolve().parent.parent.parent
    lang_dir = (repo_root / args.lang_dir).resolve()
    out_dir = Path(args.out_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)

    script_path = repo_root / "scripts" / "i18n" / "catalog.py"
    sync_script = repo_root / "scripts" / "i18n" / "sync_lang.py"

    subprocess.run([
        sys.executable,
        str(sync_script),
        "--lang-dir",
        str(lang_dir),
    ], check=True)

    for lang_file, ts_file, language, source_language in CATALOGS:
        cmd = [
            sys.executable,
            str(script_path),
            "to-ts",
            "--lang-file",
            str(lang_dir / lang_file),
            "--ts-file",
            str(out_dir / ts_file),
            "--language",
            language,
            "--source-language",
            source_language,
        ]
        subprocess.run(cmd, check=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())

# AGENT: External Plugin Integration References

Single source of truth is maintained in existing docs.  
This file intentionally references canonical docs to avoid policy duplication.

## Canonical Docs

- Plugin architecture and lifecycle:
  - [docs/MODULES.md](/home/pyth/silent-module/docs/MODULES.md)
- i18n source format and tooling:
  - [docs/I18N.md](/home/pyth/silent-module/docs/I18N.md)
- External plugin example repository:
  - https://github.com/pythcoiner/silent-plugin

## Required Integration Baseline (Index Only)

- External plugin repo layout:
  - use `src/` for plugin source
  - use `lib/silent` as a git submodule (not symlink)
- Header integration:
  - consume interfaces from `lib/silent/src/interfaces/*`
- i18n:
  - author `.lang` sources, do not use JSON as source-of-truth
  - JSON, when needed, must be generated at build/pre-compile time
- Build:
  - reproducible Nix build target set aligned with Silent naming

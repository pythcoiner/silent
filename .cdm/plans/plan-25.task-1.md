flake.nix per-platform link flags

## Objective
Mirror the host-re-exports-Qt link configuration in the Nix release builds for Linux,
Windows (MinGW), and macOS.

## Files to Read
- `flake.nix` - `buildGui`, the per-platform derivations + `postUnpackExtra` sed
  patterns that already rewrite `target_link_libraries`.
- `CMakeLists.txt` - the link flags added in phase 24.
- `docs/PLUGINS.md` - the per-platform note (`--export-dynamic` / `-bundle_loader` /
  export-lib).

## Implementation Steps
1. **Linux**: ensure the static-Qt build passes `--whole-archive`/`--export-dynamic`
   (the CMake change from phase 24 should already apply; confirm it survives the Nix
   static link, add `extraCmakeFlags`/`NIX_LDFLAGS` if needed).
2. **Windows (MinGW)**: export an import lib from the `.exe` (`-Wl,--out-implib`) and
   ensure plugins link against the host's exported symbols; adjust the existing
   `postUnpackExtra` if the CMake guard needs per-platform branching.
3. **macOS**: plugins use `-bundle_loader <silent binary>` + `-undefined
   dynamic_lookup`; the host need not export-dynamic the same way. Document this in
   the flake (the plugin build, not the host, carries `-bundle_loader`).
4. Keep the supported-surface whole-archive consistent across platforms.

## Files to Modify
- `flake.nix`.

## Verification
- [ ] `just build` (Linux) produces a binary whose dynamic symbol table exports the
      Qt surface. (Win/macOS cross-builds at least configure + link.)

## Reviewer Criteria
- Must: each platform uses the correct mechanism (export-dynamic / out-implib /
  bundle_loader), not a copy-paste of the Linux flags.
- Must: no regression to existing release builds.
- May skip: actually loading a plugin on Win/macOS (Linux is the validation target).

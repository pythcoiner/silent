CMake whole-archive Qt + export-dynamic + QtAbiSurface.cpp

## Objective
Configure the host link so it re-exports Qt for runtime plugins: whole-archive the
supported Qt surface + qontrol, `--export-dynamic`, and add non-elidable ABI anchors.

## Files to Read
- `docs/PLUGINS.md` - "Runtime linking - host re-exports Qt" + the QtAbiSurface note.
- `CMakeLists.txt` - current `target_link_libraries`.

## Implementation Steps
1. **`src/plugin/QtAbiSurface.cpp`**
   - Non-elidable anchors so `--export-dynamic` publishes the supported surface, e.g.
     `namespace { volatile const void *anchors[] = { (const void*)&QObject::staticMetaObject,
     (const void*)&QWidget::staticMetaObject, /* Core/Gui/Widgets/Svg + qontrol types */ }; }`
   - Comment: must take addresses (survive `-O2`/`--gc-sections`); paired with
     `--export-dynamic`.
2. **CMakeLists.txt** (Linux/GCC/Clang)
   - Wrap the Qt + qontrol static libs in
     `-Wl,--whole-archive ... -Wl,--no-whole-archive` and add `-Wl,--export-dynamic`
     to the `silent` link (generator-expression / `target_link_options`).
   - Add `src/plugin/QtAbiSurface.cpp` to SOURCES.
   - Keep it behind a guard so non-ELF toolchains are handled in phase 25 (Nix).

## Files to Create
- `src/plugin/QtAbiSurface.cpp`.

## Files to Modify
- `CMakeLists.txt`.

## Verification
- [ ] Local build links; `nm -D ./build/silent | grep -c ' T '` shows exported Qt
      symbols (e.g. `QWidget`), proving the surface is published.

## Reviewer Criteria
- Must: anchors take addresses (non-elidable); not just `sizeof`.
- Must: whole-archive limited to the documented surface (Core/Gui/Widgets/Svg+qontrol).
- May skip: Windows/macOS variants (phase 25).

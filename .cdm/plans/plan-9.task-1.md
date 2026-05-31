app.json: enabled-plugin set + active theme

## Objective
Persist app-level state separate from accounts: the set of enabled plugin ids and
the active theme name, stored in `~/.silent/app.json`.

## Files to Read
- `silent/src/config.rs` - `datadir()`, serde + file IO patterns to mirror.
- `silent/src/lib.rs` - bridge export style for free functions.

## Implementation Steps
1. **New `silent/src/app_state.rs`** (or extend config.rs)
   - `struct AppState { enabled_plugins: Vec<String>, active_theme: String }`,
     serde, `load()` from `datadir()/app.json` (default empty + theme "light"),
     `save()`.
2. **`silent/src/lib.rs`** - export FFI free functions:
   - `app_enabled_plugins() -> Vec<String>`, `app_set_plugin_enabled(id: String,
     enabled: bool)`, `app_active_theme() -> String`, `app_set_active_theme(name:
     String)`. Keep them simple (load-modify-save).
3. Regenerate headers with `./build.sh`.

## Files to Create
- `silent/src/app_state.rs` (if separate); register module in `lib.rs`.

## Files to Modify
- `silent/src/lib.rs` (bridge exports), `silent/src/config.rs` (if extending there).

## Verification
- [ ] `cargo build` + `cargo test` pass; a roundtrip test writes/reads app.json.
- [ ] `./build.sh` regenerates headers.

## Reviewer Criteria
- Must: missing app.json yields sane defaults (no panic).
- Must: enable/disable is idempotent (no duplicate ids).
- May skip: migration of legacy state (none exists yet).

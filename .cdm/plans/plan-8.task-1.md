Add plugin_id to Config and thread through FFI

## Objective
Persist which module owns each account so the registry can route an account to the
right module on reopen. Add `plugin_id` (default `"sp"`) to the Rust `Config`.

## Files to Read
- `silent/src/config.rs` - `Config` struct, `new`, `new_config`, serde, `to_file`,
  `from_file`, `list_configs`.
- `silent/src/lib.rs` - the `#[cxx::bridge]` `new_config(...)` signature + Config
  method exports.
- `build.sh` - regenerates CXX headers into `lib/include/`.

## Implementation Steps
1. **`silent/src/config.rs`**
   - Add `#[serde(default = "default_plugin_id")] pub plugin_id: String` to `Config`
     with `fn default_plugin_id() -> String { "sp".into() }` (backward compatible with
     existing config.json files).
   - Add it as a param to `Config::new(...)` (or default it there); add a getter
     `get_plugin_id(&self) -> String` and `set_plugin_id(&mut self, ...)`.
2. **`silent/src/lib.rs`**
   - Extend the `new_config(...)` FFI signature with `plugin_id: String` (or add a
     separate setter) and export `get_plugin_id`/`set_plugin_id` on `Config`.
3. **Regenerate headers**: run `./build.sh`; confirm `lib/include/silent.h` updates.

## Files to Modify
- `silent/src/config.rs`, `silent/src/lib.rs`.

## Verification
- [ ] `cargo build --manifest-path silent/Cargo.toml` passes.
- [ ] `cargo test --manifest-path silent/Cargo.toml` passes (config roundtrip keeps
      working; old config.json without plugin_id still loads as "sp").
- [ ] `./build.sh` regenerates `lib/include/silent.h` with the new exports.

## Reviewer Criteria
- Must: `#[serde(default)]` so pre-existing config.json files still deserialize.
- Must: default is `"sp"`.
- Must: stays in silent's config layer (not pushed to spdk/bwk).

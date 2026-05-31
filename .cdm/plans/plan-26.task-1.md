Example plugin skeleton + metadata + CMake

## Objective
Create an out-of-tree example plugin proving the boundary: an `IPlugin` with one
module, its own CMake (links only the SDK headers + host-provided Qt), and a sidecar
`<id>.json`.

## Files to Read
- `docs/PLUGINS.md` - Distribution Unit + ABI rules (plugins link no Qt of their own).
- `src/plugin/sdk/*` - the interfaces to implement.
- `src/plugin/host/PluginRegistry.*` - what discovery expects from the sidecar.

## Implementation Steps
1. **`contrib/example-plugin/`** (separate CMake project)
   - `CMakeLists.txt`: build a shared library; include the SDK headers; do NOT link Qt
     (resolve Qt + qontrol from the host at load); set `Q_PLUGIN_METADATA` json.
   - `ExamplePlugin.cpp`: `class ExamplePlugin : public QObject, public IPlugin` with
     `Q_OBJECT`, `Q_PLUGIN_METADATA(IID SILENT_IPlugin_IID FILE "example.json")`,
     `Q_INTERFACES(IPlugin)`; `meta()`/`modules()` returning one `ExampleModule`.
   - `ExampleModule`: minimal `IModule` (`list()` returns one startable instance;
     `startInstance` constructs a trivial instance).
   - `example.json` sidecar: id="example", name, version, capabilities.
2. Output layout matches `~/.silent/plugins/example/{libexample.so, example.json}`.

## Files to Create
- `contrib/example-plugin/CMakeLists.txt`, `contrib/example-plugin/ExamplePlugin.cpp`,
  `contrib/example-plugin/example.json`.

## Verification
- [ ] The plugin builds against the SDK headers with no Qt linked into it.
- [ ] Dropping it in `~/.silent/plugins/example/` makes it appear in Settings as
      discovered + disabled (no code run).

## Reviewer Criteria
- Must: plugin links no Qt of its own (host re-export model).
- Must: `Q_PLUGIN_METADATA` IID matches the SDK `SILENT_IPlugin_IID`.
- Must: sidecar json present and read before load.

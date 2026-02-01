# Task Plan Template

Task plan files live in `.cm/plans/` and provide detailed instructions for individual tasks. Each task references its plan via the `plan_file` field in `tasks.json`.

## Relationship to Phase Plans

- **Phase Plan** (`Phase.plan` field): High-level strategy for an entire phase
- **Task Plan** (`.cm/plans/*.md`): Specific instructions agents receive for execution

Agents receive the task plan content directly in their prompt. They have no visibility into phase-level context beyond what's in their plan file.

---

## Required Sections (All Task Types)

### 1. Title Line

Single line describing the task:

```markdown
Implement error handling for config loading
```

### 2. Objective

What this specific task accomplishes. Be precise:

```markdown
## Objective

Add error types and handling for configuration file loading, covering missing files, parse errors, and invalid values.
```

### 3. Implementation Steps

Numbered list with:
- **What to do** - Specific action
- **Where to do it** - File path with line numbers
- **How to do it** - Approach description

```markdown
## Implementation Steps

1. **Add ConfigError enum** (`src/config/error.rs:1-30`)
   - Create new file with typed error variants
   - Include `MissingFile`, `ParseError`, `InvalidValue` variants
   - Derive `thiserror::Error` for each variant

2. **Update load_config function** (`src/config/mod.rs:45-80`)
   - Replace `unwrap()` calls with `?` operator
   - Return `Result<Config, ConfigError>`
   - Add context to errors using `map_err()`

3. **Propagate errors to caller** (`src/cli/mod.rs:120-135`)
   - Handle ConfigError at CLI entry point
   - Display user-friendly error messages
   - Exit with appropriate error codes
```

### 4. Files to Read

List files the agent should read for context, with line ranges for large files:

```markdown
## Files to Read

- `src/config/mod.rs` - Current config loading implementation
- `src/state/error.rs:1-50` - Existing error pattern to follow
- `src/cli/mod.rs:100-150` - CLI error handling context
```

### 5. Files to Modify

List files with specific changes:

```markdown
## Files to Modify

- `src/config/mod.rs:45-80` - Update load_config return type and error handling
- `src/cli/mod.rs:120-135` - Handle ConfigError at entry point
```

### 6. Files to Create

New files with their purpose:

```markdown
## Files to Create

- `src/config/error.rs` - ConfigError enum with typed variants
```

### 7. Verification

Commands and checks to verify completion:

```markdown
## Verification

- [ ] `cargo build` passes
- [ ] `cargo clippy` passes with no warnings
- [ ] `cargo test` passes
- [ ] Error messages are user-friendly
```

---

## Task-Type-Specific Sections

### Implementation Tasks

Add requirements and code examples:

```markdown
## Requirements

- Must handle all error cases (missing file, parse error, invalid value)
- Error messages must include file path and line number where applicable
- Must use project's existing error handling patterns (see `src/state/error.rs`)

## Code Examples

Error enum pattern to follow:

```rust
#[derive(Debug, thiserror::Error)]
pub enum ConfigError {
    #[error("Config file not found: {path}")]
    MissingFile { path: PathBuf },

    #[error("Failed to parse config at {path}: {source}")]
    ParseError {
        path: PathBuf,
        #[source]
        source: toml::de::Error,
    },
}
```
```

### Fix Tasks

Add bug analysis:

```markdown
## Bug Details

- **Observed**: Program panics when config file is missing
- **Expected**: Graceful error message and exit
- **Location**: `src/config/mod.rs:52` - `unwrap()` on missing file

## Root Cause

The `fs::read_to_string()` result is unwrapped without checking if the file exists. When the file is missing, this causes a panic instead of a handled error.

## Regression Prevention

Add test case:
- Test config loading with missing file
- Verify error type is `ConfigError::MissingFile`
- Verify error message contains the path
```

### Test Tasks

Add test specifications:

```markdown
## Test Cases

1. **test_config_missing_file**
   - Input: Non-existent path `/tmp/nonexistent.toml`
   - Expected: `ConfigError::MissingFile` with path in message

2. **test_config_parse_error**
   - Input: File with invalid TOML syntax
   - Expected: `ConfigError::ParseError` with line number

3. **test_config_invalid_value**
   - Input: Valid TOML with out-of-range timeout value
   - Expected: `ConfigError::InvalidValue` with field name
```

---

## Reviewer Criteria

When writing plans, include specific criteria for the reviewer to check:

```markdown
## Reviewer Criteria

**Must check:**
- [ ] All error variants are covered
- [ ] Error messages include actionable information (file paths, line numbers)
- [ ] No `unwrap()` or `expect()` on fallible operations
- [ ] Tests cover all error paths

**May skip if time-constrained:**
- [ ] Documentation comments on public items (can be added later)
- [ ] Exhaustive edge case tests (cover main paths first)
```

---

## Complete Example: Implementation Task Plan

```markdown
Implement phase plan storage in Phase struct

## Objective

Add a `plan` field to the Phase struct to store the detailed implementation plan for each phase. This enables agents to receive phase-level context.

## Files to Read

- `src/state/tasks.rs:80-120` - Current Phase struct definition
- `src/state/validate.rs:40-70` - JSON schema validation patterns
- `src/agent/prompt.rs:500-560` - How prompts are built

## Implementation Steps

1. **Add plan field to Phase struct** (`src/state/tasks.rs:83-105`)
   - Add `pub plan: String` field after `name` field
   - Use `#[serde(default)]` for backward compatibility
   - Initialize to empty string in any Default impl

2. **Update JSON schema** (`src/state/validate.rs:45-60`)
   - Add `plan` as optional string field in Phase schema
   - Maintain backward compatibility with existing JSON

3. **Include plan in agent prompts** (`src/agent/prompt.rs:509-550`)
   - Check if `phase.plan` is non-empty
   - Add "### Phase Plan" section to IMPLEM prompt
   - Pass plan content to agents

## Code Examples

Field addition pattern:

```rust
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Phase {
    pub id: String,
    pub name: String,
    #[serde(default)]
    pub plan: String,  // Add here
    pub status: PhaseStatus,
    pub tasks: Vec<Task>,
}
```

## Files to Modify

- `src/state/tasks.rs:83-105` - Add plan field to Phase
- `src/state/validate.rs:45-60` - Update schema
- `src/agent/prompt.rs:509-550` - Include plan in prompts

## Files to Create

None

## Verification

- [ ] `cargo build && cargo clippy` pass
- [ ] `cargo test` passes
- [ ] Existing tasks.json files still load (backward compatibility)
- [ ] New phases can include plan field

## Reviewer Criteria

**Must check:**
- [ ] Backward compatibility with existing JSON files
- [ ] Plan content appears in agent prompts when non-empty
- [ ] Serde attributes are correct

**May skip:**
- [ ] Integration tests (unit tests sufficient for this change)
```

---

## Complete Example: Fix Task Plan

```markdown
Fix: Panic on missing config file

## Objective

Replace panic with graceful error handling when config file is missing.

## Bug Details

- **Observed**: Program panics with "called `Result::unwrap()` on an `Err` value"
- **Expected**: User-friendly error message: "Config file not found: /path/to/config.toml"
- **Location**: `src/config/mod.rs:52`

## Root Cause

`fs::read_to_string(path).unwrap()` panics when file doesn't exist. No error handling exists for this common case.

## Files to Read

- `src/config/mod.rs:45-60` - Current implementation with unwrap
- `src/config/error.rs` - Existing error types (if any)

## Fix Steps

1. **Replace unwrap with proper error handling** (`src/config/mod.rs:50-55`)
   - Change `unwrap()` to `map_err()` with ConfigError::MissingFile
   - Include path in error message

2. **Add ConfigError::MissingFile variant** (`src/config/error.rs:15`)
   - Already exists, just needs to be used

## Code Examples

Before:
```rust
let content = fs::read_to_string(&path).unwrap();
```

After:
```rust
let content = fs::read_to_string(&path)
    .map_err(|e| ConfigError::MissingFile {
        path: path.clone(),
        source: e
    })?;
```

## Files to Modify

- `src/config/mod.rs:50-55` - Replace unwrap with error handling

## Testing

1. Add unit test `test_load_missing_config`:
   - Input: Path to non-existent file
   - Expected: `Err(ConfigError::MissingFile(_))`
   - Verify error message contains the path

2. Run existing tests to check for regressions

## Verification

- [ ] Bug no longer reproduces (missing config shows error, not panic)
- [ ] `cargo test` passes including new test
- [ ] Error message is user-friendly

## Reviewer Criteria

**Must check:**
- [ ] Panic is eliminated
- [ ] Error message includes the file path
- [ ] Test covers the fix

**May skip:**
- [ ] Testing all possible IO errors (just the missing file case is sufficient)
```

---

## Guidelines

### What Agents Expect

Agents receive this plan content directly (see `IMPLEMENTER.md`). They:
- Have NO access to phase-level context beyond this plan
- Cannot ask clarifying questions
- Must complete ALL items in the plan

### Plan Compliance Rules

1. **Be complete** - Don't assume agents know anything not in this plan
2. **Be specific** - Include exact file paths and line numbers
3. **Be ordered** - Steps should be in implementation order
4. **Include tests** - If tests are needed, explicitly list them (agents will skip unstated tests)
5. **Add code examples** - Show patterns to follow, but not complete implementations

### Common Mistakes

- **Missing file paths**: "Update the config loader" (where?)
- **Vague steps**: "Add error handling" (for what cases?)
- **Unstated tests**: Assuming agents will add tests without explicit instruction
- **Missing context**: Referencing code patterns without showing examples
- **No line ranges**: For large files, agents waste context reading irrelevant parts

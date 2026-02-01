# Phase Plan Template

When generating a phase plan for `/feat` or `/fix`, you MUST produce a detailed, actionable implementation plan following this structure. The plan is stored in the phase's `plan` field in tasks.json and passed to IMPLEM/REVIEW/FIX agents.

## Required Sections

### 1. Objective

A clear, single-paragraph statement of what this phase accomplishes. Be specific about the end result.

### 2. Background

Explain WHY this change is needed:
- What problem does it solve?
- What is the current state?
- What are the consequences of not doing this?

This context helps agents understand the motivation behind implementation decisions.

### 3. Implementation Steps

Numbered list of concrete steps. Each step MUST include:
- **What to do** - The specific action
- **Where to do it** - File path with line numbers when known (e.g., `src/state/tasks.rs:83-105`)
- **How to do it** - Brief description of the approach

Example:
```
1. **Add plan field to Phase struct** (`src/state/tasks.rs:83-105`)
   - Add `pub plan: String` field after the `name` field
   - Use `#[serde(default)]` for backward compatibility with existing JSON
   - Update any Default implementations if present
```

### 4. Files to Modify

List each file that needs changes with a brief description:
```
- `src/state/tasks.rs:83-105` - Add `plan: String` to Phase struct
- `src/agent/prompt.rs:509-601` - Include phase.plan in prompt building
```

Include line numbers when you know them from exploration.

### 5. Files to Create

List new files to be created with their purpose:
```
- `src/generate/tasks_md.rs` - TASKS.md generator module
- `assets/templates/PLAN.md` - Phase plan template
```

### 6. Key Decisions

Document architectural choices and their rationale:
```
- **Plan is required, not optional**: Every phase must have a plan to ensure agents always have context.
- **Plan lives on Phase, not Task**: The detailed plan is phase-level because it describes how all tasks work together.
```

### 7. Success Criteria

Checkboxes for verifiable outcomes:
```
- [ ] `cargo build && cargo clippy` pass
- [ ] `cargo test` passes with updated tests
- [ ] New feature works as specified
```

### 8. Verification

Concrete commands to verify the implementation:
```bash
cargo build && cargo clippy && cargo test
```

## Guidelines for Writing Good Plans

1. **Be Specific**: Use exact file paths, function names, and line numbers
2. **Be Complete**: Cover all aspects - don't leave gaps for agents to guess
3. **Be Ordered**: Steps should be in implementation order
4. **Be Actionable**: Each step should be directly executable
5. **Explain Decisions**: Document WHY, not just WHAT
6. **Include Context**: Agents only see this plan - give them everything they need

## Example: Adding a New Feature

```markdown
## Objective

Add a `--quiet` flag to the CLI that suppresses all non-error output, enabling cm to be used in scripts and CI pipelines without verbose logging.

## Background

Currently, cm always outputs progress information to stderr. In CI environments and shell scripts, this verbose output clutters logs and makes it harder to parse actual errors. Users have requested a quiet mode that only shows errors.

The `--verbose` flag already exists for increased output, so `--quiet` provides the inverse capability, completing the output control spectrum.

## Implementation Steps

1. **Add quiet flag to CLI struct** (`src/cli/mod.rs:45-80`)
   - Add `#[arg(short, long)]` decorated `quiet: bool` field
   - Conflicts with `verbose` flag (can't use both)

2. **Thread quiet flag through ManagerConfig** (`src/manager/mod.rs:30-50`)
   - Add `quiet: bool` to ManagerConfig struct
   - Add `.quiet()` builder method

3. **Update emit_cm helper** (`src/manager/mod.rs:150-160`)
   - Check `self.config.quiet` before writing to stderr
   - Always emit errors regardless of quiet flag

4. **Update TUI initialization** (`src/cli/mod.rs:200-250`)
   - When quiet=true, skip TUI entirely and run in daemon mode
   - Suppress status output in daemon mode when quiet=true

5. **Add integration test** (`tests/integration.rs`)
   - Test that `--quiet` suppresses normal output
   - Test that errors still appear with `--quiet`

## Files to Modify

- `src/cli/mod.rs:45-80` - Add quiet flag to Cli struct
- `src/cli/mod.rs:200-250` - Handle quiet flag in execution path
- `src/manager/mod.rs:30-50` - Add quiet to ManagerConfig
- `src/manager/mod.rs:150-160` - Check quiet before emitting
- `tests/integration.rs` - Add quiet mode tests

## Files to Create

None - this is a modification to existing code only.

## Key Decisions

- **Quiet suppresses info, not errors**: Errors always show because silent failure is worse than verbose success.
- **Quiet implies daemon mode**: TUI makes no sense in quiet mode, so we force daemon mode.
- **Quiet conflicts with verbose**: These are mutually exclusive to avoid confusing combinations.

## Success Criteria

- [ ] `cm --quiet` runs without any stdout/stderr output on success
- [ ] `cm --quiet` shows error messages on failure
- [ ] `cm --quiet --verbose` errors with conflict message
- [ ] Integration tests pass

## Verification

```bash
cargo build && cargo clippy && cargo test
cm --quiet --step  # Should be silent on success
cm --quiet --step 2>&1 | wc -l  # Should be 0 lines on success
```
```

## For Bug Fix Phases

When generating plans for `/fix`, include additional sections:

### Root Cause Analysis
What is causing the bug? Be specific about the code path and conditions.

### Fix Strategy
How will you fix it? Describe the approach before diving into steps.

### Regression Prevention
How do you prevent this bug from recurring? What tests or guards are needed?

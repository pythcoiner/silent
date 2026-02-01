# Expand Plans Wizard

This command expands minimal plan files in `.cm/plans/` with detailed implementation instructions, file references, and reviewer criteria.

## Prerequisites

Requires:
- `.cm/` directory with `tasks.json`
- At least one plan file in `.cm/plans/`

If no plan files exist, inform the user to run `/feat` or `/fix` first.

## CRITICAL: Interactive Approach

**Ask questions first, expand later.**

Before expanding any plan, you MUST:
1. Read and understand each plan file
2. Identify gaps or ambiguities
3. **Ask the user clarifying questions** about unclear requirements
4. Only then proceed with expansion

Do NOT assume requirements. If something is unclear, ask.

---

## CRITICAL: Agent-Only Execution

**All tasks will be executed by autonomous agents that run WITHOUT human interaction.**

IMPLEM, REVIEW, and FIX agents:
- Run completely autonomously
- **CANNOT ask questions or request clarification**
- **CANNOT pause and wait for user input**
- Must have ALL information they need in the plan

Plans must be written with this constraint in mind:

1. **Agents can only do code work** - reading files, writing code, running commands
2. **Agents cannot**:
   - Ask for clarification (they will make assumptions or fail)
   - Access external services requiring authentication (unless credentials are in env)
   - Perform manual UI testing
   - Make decisions requiring human judgment
   - Access resources outside the codebase

3. **Plans must be 100% unambiguous** - If an agent might need to ask "which approach?", the plan must already specify the answer

4. **Document manual steps separately** - If a task requires manual work:
   - Clearly mark it as `## Manual Steps (Out of Scope for Agents)`
   - Explain what the user must do BEFORE or AFTER agent execution
   - Do NOT include manual steps in the agent's implementation steps

Example:
```markdown
## Manual Steps (Out of Scope for Agents)

Before running this phase:
- [ ] Create API key at https://example.com/api
- [ ] Add `API_KEY` to `.env` file

After agent completes:
- [ ] Manually test the OAuth flow in browser
- [ ] Verify email notifications are received
```

When expanding plans, **ask the user** if any steps require manual intervention.

---

## Step 1: Discover Plan Files

List all plan files in `.cm/plans/`:

```bash
ls .cm/plans/*.md
```

If no files found:
> No plan files found in `.cm/plans/`. Please run `/feat` or `/fix` first to create tasks with plan files.

Present the list to user:
> Found [N] plan file(s) to expand:
> - `.cm/plans/plan-1.md`
> - `.cm/plans/plan-2.md`
> ...
>
> I'll analyze each plan and ask questions if needed before expanding.

---

## Step 2: Analyze Each Plan

For each plan file:

### 2.1 Read the Plan

Read the current content and identify:
- What task(s) it covers
- Current level of detail
- Missing sections (per TASK_PLAN_TEMPLATE.md)

### 2.2 Explore the Codebase

Search the codebase to understand:
- Files that need to be read for context
- Files that will be modified
- Existing patterns to follow
- Line ranges for large files

### 2.3 Identify Gaps and Ask Questions

**This is critical.** Before expanding, ask the user about:

**Unclear requirements:**
> The plan mentions "add error handling" but doesn't specify which error cases. Which of these should be handled?
> - File not found
> - Parse errors
> - Invalid values
> - Permission denied

**Multiple approaches:**
> For implementing [feature], I see two approaches:
> 1. [Approach A] - simpler but less flexible
> 2. [Approach B] - more complex but extensible
>
> Which approach do you prefer?

**Missing context:**
> The plan references "existing patterns" but I found multiple patterns in the codebase:
> - Pattern A in `src/foo.rs`
> - Pattern B in `src/bar.rs`
>
> Which pattern should this follow?

**Test requirements:**
> Should this task include tests? If so, what level?
> - Unit tests only
> - Unit + integration tests
> - Specific edge cases to cover

**Manual steps (agents cannot do these):**
> Are there any steps that require manual intervention?
> - External service setup (API keys, OAuth apps, etc.)
> - Manual UI/UX testing
> - Deployment or infrastructure changes
> - Decisions requiring human judgment
>
> If yes, I'll document these separately so you know what to do before/after the agent runs.

Wait for user responses before proceeding.

---

## Step 3: Expand the Plan

After gathering all needed information, expand each plan with:

### 3.1 Files to Read

Add comprehensive list of files the agent should read:

```markdown
## Files to Read

- `src/config/mod.rs:45-80` - Current implementation to modify
- `src/state/error.rs:1-50` - Error pattern to follow
- `src/cli/mod.rs:100-150` - How errors are handled at CLI level
```

**Important:** Use line ranges for large files to save agent context.

### 3.2 Detailed Implementation Steps

Expand each step with:
- Exact file paths and line numbers
- Code examples showing the pattern (not complete code)
- Specific functions/structs to modify

```markdown
## Implementation Steps

1. **Add ConfigError enum** (`src/config/error.rs:1-30`)

   Create new file with typed error variants following the pattern in `src/state/error.rs`:

   ```rust
   #[derive(Debug, thiserror::Error)]
   pub enum ConfigError {
       #[error("Config file not found: {path}")]
       MissingFile { path: PathBuf },
       // ... add other variants
   }
   ```

   - Include `MissingFile`, `ParseError`, `InvalidValue` variants
   - Derive `thiserror::Error` for each variant
   - Add `#[source]` for wrapped errors
```

### 3.3 Reviewer Criteria

Add specific checks for the reviewer:

```markdown
## Reviewer Criteria

**Must check:**
- [ ] All error variants are covered (MissingFile, ParseError, InvalidValue)
- [ ] Error messages include actionable information
- [ ] No `unwrap()` or `expect()` on fallible operations in modified code
- [ ] Tests cover the main error paths

**May skip if time-constrained:**
- [ ] Documentation comments (can be added in follow-up)
- [ ] Exhaustive edge case tests (main paths sufficient for now)
```

**When to use "May skip":**
- Non-critical improvements that can be deferred
- Exhaustive testing when basic coverage exists
- Documentation that doesn't affect functionality
- Style improvements in unchanged code

---

## Step 4: Write Expanded Plans

After expanding all plans, write them back to `.cm/plans/`:

For each plan:
1. Show the expanded content to the user
2. Write to the original file (in-place update)

> **Expanded `.cm/plans/plan-1.md`:**
>
> [Show expanded content]
>
> Writing expanded plan...

---

## Step 5: Summary

After all plans are expanded:

> **Expansion complete!**
>
> **Plans expanded:**
> - `.cm/plans/plan-1.md` - Added [N] file references, [M] code examples
> - `.cm/plans/plan-2.md` - Added [N] file references, [M] code examples
>
> **Changes made:**
> - Added `Files to Read` sections with line ranges
> - Expanded implementation steps with code examples
> - Added reviewer criteria with skip conditions
>
> The plans are now ready for agent execution. Run `cm` when ready.

---

## What to Expand

### Always Add

1. **Files to Read** with line ranges for large files (>200 lines)
2. **Exact file paths** with line numbers in implementation steps
3. **Code examples** showing patterns to follow (not complete implementations)
4. **Reviewer criteria** with must-check and may-skip items
5. **Manual Steps section** if any work requires human intervention

### Add When Relevant

1. **Root cause analysis** for bug fixes
2. **Test cases** with specific inputs and expected outputs
3. **Error handling requirements** with specific error types
4. **Migration notes** if changing data formats
5. **Prerequisites** that user must complete before agent runs

### Never Add

1. Complete code implementations (agents should write the code)
2. Time estimates
3. Speculative future requirements
4. Unrelated improvements
5. **Manual tasks mixed with agent tasks** - keep them in separate sections
6. Steps requiring external authentication agents can't perform

---

## Example Expansion

### Before (Minimal Plan)

```markdown
Implement config validation

## Objective

Add validation for config file values.

## Steps

1. Add validation function
2. Call it when loading config
3. Add tests
```

### After (Expanded Plan)

```markdown
Implement config validation

## Objective

Add validation for config file values, ensuring timeout is positive, paths exist, and required fields are present.

## Files to Read

- `src/config/mod.rs:1-100` - Current config loading
- `src/config/types.rs:20-60` - Config struct definition
- `src/state/validate.rs:1-80` - Existing validation patterns

## Implementation Steps

1. **Add validate_config function** (`src/config/mod.rs:85-120`)

   Add validation after parsing, following pattern in `src/state/validate.rs`:

   ```rust
   fn validate_config(config: &Config) -> Result<(), ConfigError> {
       if config.timeout <= 0 {
           return Err(ConfigError::InvalidValue {
               field: "timeout".to_string(),
               reason: "must be positive".to_string(),
           });
       }
       // ... more validations
       Ok(())
   }
   ```

2. **Call validation in load_config** (`src/config/mod.rs:50-55`)

   After `toml::from_str()`, add:
   ```rust
   let config: Config = toml::from_str(&content)?;
   validate_config(&config)?;  // Add this line
   Ok(config)
   ```

3. **Add validation tests** (`src/config/mod.rs` - test module)

   Test cases:
   - `test_validate_negative_timeout` - timeout = -1, expect InvalidValue
   - `test_validate_missing_required` - omit required field, expect MissingField
   - `test_validate_valid_config` - valid config, expect Ok

## Files to Modify

- `src/config/mod.rs:50-55` - Add validation call
- `src/config/mod.rs:85-120` - Add validate_config function
- `src/config/mod.rs` (test module) - Add validation tests

## Verification

- [ ] `cargo build && cargo clippy` pass
- [ ] `cargo test` passes with new tests
- [ ] Invalid configs produce clear error messages

## Reviewer Criteria

**Must check:**
- [ ] All config fields are validated
- [ ] Error messages specify which field failed and why
- [ ] Tests cover invalid timeout, missing fields, valid config

**May skip:**
- [ ] Testing every possible invalid value combination
- [ ] Documentation for internal validate_config function
```

---

## Error Handling

### No Plan Files

> No plan files found in `.cm/plans/`.
>
> Plan files are created when you use `/feat` or `/fix` to add tasks.
> Please run one of those commands first, then return here to expand the plans.

### Plan Already Detailed

If a plan already has all sections:

> `.cm/plans/plan-1.md` already appears fully detailed:
> - Has Files to Read section
> - Has code examples
> - Has reviewer criteria
>
> Skip expansion? (yes/no)

### Cannot Determine Requirements

If requirements are truly ambiguous and user can't clarify:

> I cannot determine the requirements for [specific item].
>
> Options:
> 1. Leave this section minimal (agent will make assumptions)
> 2. Skip this plan file entirely
> 3. Add a TODO marker for later clarification
>
> Which option?

---

## Tips for Good Expansions

1. **Ask questions NOW** - You (the /expand agent) can ask the user questions, but IMPLEM/REVIEW/FIX agents CANNOT. Resolve all ambiguities during expansion.
2. **Leave no room for interpretation** - If there are two valid approaches, the plan must pick one. Agents cannot ask "which one?"
3. **Use line ranges** - Large files should have ranges like `:45-80`
4. **Show patterns, not solutions** - Code examples demonstrate style, not complete implementations
5. **Be specific about checks** - "No unwrap()" is better than "good error handling"
6. **Mark skippable items** - Helps reviewers prioritize when time is limited
7. **Reference existing code** - Point to patterns in the codebase agents should follow
8. **Separate manual work** - Agents run autonomously; manual steps must be in a separate section for the user

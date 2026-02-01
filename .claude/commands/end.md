# End Session Wizard

This command finalizes a `/feat`, `/fix`, or `/cm` conversational session by saving all gathered information to the planning files without starting implementation.

## Use Cases

1. **After `/feat` or `/fix`**: Add new tasks to existing tasks.json/roadmap.json
2. **After `/cm` (or `/cm` + `/split`)**: Generate initial tasks.json and roadmap.json from PLAN.md

## Prerequisites

Before using this command, ensure:
- You are completing a `/feat`, `/fix`, or `/cm` session
- A `.cm/` directory exists
- For `/feat` or `/fix`: tasks.json and roadmap.json exist
- For `/cm`: PLAN.md exists
- The user has confirmed all details

If prerequisites are not met, inform the user.

## CRITICAL: Scope Limitations

This command ONLY updates planning files:
- `.cm/tasks.json` - Add new tasks (or generate initial file for `/cm` sessions)
- `.cm/roadmap.json` - Add new items (or generate initial file for `/cm` sessions)
- `.cm/PLAN.md` - Add feature documentation (for `/feat` sessions only)

This command does NOT:
- Implement any code
- Run `cm run` or execute tasks
- Make changes outside `.cm/` directory

After this command completes, the user must manually run `cm run` when ready to implement.

## Important: Session Context

**For `/feat` or `/fix` sessions:**
This command assumes you have already:
- Gathered all feature/fix requirements from the user
- Confirmed task breakdown and approach
- Received user approval for the plan
- Identified phase placement for new tasks

**For `/cm` sessions:**
This command assumes:
- `/cm` wizard has created PLAN.md with phases
- User has optionally refined it with `/split`
- User is ready to generate JSON files

DO NOT use this command to start a new planning session. Use `/feat`, `/fix`, or `/cm` for that.

---

## Step 1: Detect Session Type

First, determine what type of session you're finalizing:

1. **Check if `.cm/tasks.json` exists:**
   - **If YES** → This is a `/feat` or `/fix` session (go to Step 1A)
   - **If NO** → This is a `/cm` session (go to Step 1B)

### Step 1A: Recognize /feat or /fix Session

Determine which type of session you're finalizing:

**For /feat sessions:**
- Review the feature name, description, and requirements gathered
- Identify all tasks that were discussed
- Note phase placement decisions
- Confirm what should be added to PLAN.md

**For /fix sessions:**
- Review the bug description and severity
- Identify the fix task that was discussed
- Note task placement and priority
- No PLAN.md updates needed (fixes don't modify architectural docs)

Confirm with the user:

> I'm going to save the [feature/fix] we discussed to the planning files.
>
> **Summary:**
> - [Feature name/Bug summary]
> - [N] tasks to add
> - Phase: [phase-name]
>
> Proceed? (yes/no)

Wait for confirmation before proceeding to Step 2.

### Step 1B: Recognize /cm Session

For `/cm` sessions, you are generating initial JSON files from PLAN.md:

1. Check if `.cm/PLAN.md` exists:
   - **If NO** → Error: "Please run `/cm` first to create PLAN.md"
   - **If YES** → Continue

2. Confirm with the user:

> I'm going to generate tasks.json and roadmap.json from your PLAN.md.
>
> This will create the initial project structure based on the phases you've defined.
>
> Proceed? (yes/no)

Wait for confirmation before proceeding to Step 1C.

### Step 1C: Parse PLAN.md for /cm Session

Read and parse `.cm/PLAN.md` to extract:

1. **Project metadata:**
   - Project name (from title line)
   - Description (from subtitle line)
   - Overview text

2. **Phases section:**
   - Parse all "### Phase N:" sections
   - Extract phase names, goals, tasks, and deliverables
   - Note task dependencies if mentioned

3. **Success criteria:**
   - Extract from "## Success Criteria" section if present

Present summary:

> **Parsed from PLAN.md:**
>
> **Project:** [name]
> **Description:** [description]
>
> **Phases to create:**
> 1. Phase 1: [name] - [N tasks]
> 2. Phase 2: [name] - [M tasks]
> ...
>
> **Total:** [X] phases, [Y] tasks
>
> Does this look correct? Reply "yes" to generate JSON files, or "no" to review PLAN.md.

Wait for confirmation before proceeding to Step 2CM.

---

## Step 2: Update tasks.json

> **Note:** See `.cm/SCHEMA.md` for the complete authoritative schema reference.

Add the new task definitions to `.cm/tasks.json`:

### 2.1 Read Current tasks.json

Read the current `.cm/tasks.json` file to:
- Find the appropriate phase to add tasks to
- Check for ID conflicts
- Ensure dependencies are valid

### 2.2 Add New Tasks

Insert new tasks following the schema:

```json
{
  "id": "phase-X.feat-[name].task-1",
  "name": "Task Name",
  "type": "implement",
  "status": "pending",
  "depends_on": [],
  "context": {
    "files_to_read": ["relevant/files.rs"],
    "code_style_excerpt": null
  },
  "plan_file": ".cm/plans/plan-X.md"
}
```

**Task ID Format:**
- For features: `phase-X.feat-[name].task-N`
- For fixes: `phase-X.fix-[name]` or `phase-X.task-N.fix`

**Required Fields:**
- `id`: Unique task identifier
- `name`: Human-readable task name
- `type`: One of "implement", "review", "fix", "test"
- `status`: Should be "pending" for new tasks
- `context`: Must include at minimum an empty object
- `plan_file`: Path to the plan file (e.g., ".cm/plans/plan-X.md")

**Dependencies:**
- Ensure `depends_on` references only existing task IDs
- Tasks can only depend on tasks from the same phase or earlier phases
- Do not create circular dependencies

### 2.3 Add roadmap_item_id Links

For each task, if it corresponds to a roadmap item, add the `roadmap_item_id` field to link them:

```json
{
  "id": "phase-1.task-1",
  "roadmap_item_id": "item-1",
  ...
}
```

This ensures roadmap checkboxes sync with task completion.

---

## Step 3: Update roadmap.json

Add new roadmap items to `.cm/roadmap.json`:

### 3.1 Read Current roadmap.json

Read the current `.cm/roadmap.json` file to:
- Find the appropriate phase
- Generate unique item IDs
- Ensure consistency

### 3.2 Add New Items

Insert new roadmap items following the schema:

```json
{
  "id": "item-X",
  "name": "Item Name",
  "completed": false,
  "sub_items": [
    { "name": "Sub-item name", "completed": false }
  ],
  "linked_task_ids": ["phase-1.task-1", "phase-1.task-2"]
}
```

**Item Structure:**
- `id`: Unique identifier for the item
- `name`: Human-readable name (matches task names or groups them)
- `completed`: Always `false` for new items
- `sub_items`: Optional array of sub-items with checkboxes
- `linked_task_ids`: Array of task IDs that this item represents

**Linking Strategy:**
- One roadmap item can represent multiple tasks
- Use `linked_task_ids` to track which tasks complete this item
- The item is marked completed when ALL linked tasks are completed

---

## Step 4: Update PLAN.md (Features Only)

**ONLY for /feat sessions:** Add feature documentation to `.cm/PLAN.md`.

**SKIP this step for /fix sessions** - bug fixes do not require PLAN.md updates.

### 4.1 Determine Placement

Analyze the PLAN.md structure and determine where to add the feature:
- Under an existing module section
- As a new module section
- As a subsection of an existing feature

### 4.2 Add Feature Documentation

Add a new section with this structure:

```markdown
### [Feature Name]

**Purpose:** [Feature description]

**Key files:**
- `path/to/file1.rs` - [Description]
- `path/to/file2.rs` - [Description]

**Requirements:**
- [Requirement 1]
- [Requirement 2]

**Implementation notes:**
- [Note 1]
- [Note 2]
```

Follow the existing style and formatting in PLAN.md.

---

## Step 5: Regenerate Markdown Files

After updating the JSON files, regenerate the markdown documentation to ensure synchronization:

```bash
cm --regenerate
```

This command:
- Regenerates `ROADMAP.md` from `roadmap.json`
- Regenerates `TASKS.md` from `tasks.json`
- Updates summary tables and progress counts

**Expected output:**
```
Regenerating ROADMAP.md from roadmap.json...
Regenerating TASKS.md from tasks.json...
✓ Files regenerated successfully
```

If the command fails, investigate the error and fix any JSON formatting issues.

---

## Step 6: Validate Changes (MANDATORY GATE)

After updating all files, you MUST run validation and fix ALL issues before proceeding.

### 6.1 Run sanity check

```bash
cm --sanity-check
```

This checks:
- JSON syntax is valid
- All task IDs are unique
- All dependencies reference existing tasks
- All `roadmap_item_id` references point to valid items
- All `linked_task_ids` in roadmap point to valid tasks
- Every uncompleted roadmap item has `linked_task_ids` pointing to tasks
- Required fields are present

### 6.2 If sanity check fails: FIX and RE-RUN

You MUST loop until the sanity check passes:

1. Read the error/warning output carefully
2. Fix the issues in the JSON files (tasks.json and/or roadmap.json)
3. Re-run `cm --sanity-check`
4. **Repeat from step 1 until ALL errors are resolved**

**Common issues and fixes:**
- **Duplicate task IDs**: Change one of the conflicting IDs
- **Invalid cross-references**: Ensure `roadmap_item_id` points to an existing roadmap item ID
- **Missing required fields**: Add the missing fields to task definitions
- **Invalid dependencies**: Check that `depends_on` references existing task IDs
- **Circular dependencies**: Remove dependency loops
- **Uncompleted roadmap item with no linked tasks**: Add `linked_task_ids` to the roadmap item pointing to the task(s) you created, OR add a new task in tasks.json and link it

### 6.3 Also check warnings

Warnings (e.g., orphaned roadmap items) indicate roadmap items that no task will ever complete. For each warning:
- If the roadmap item should be completed by a task you just added, add its task ID to `linked_task_ids`
- If the roadmap item needs a NEW task, go back to Step 2 and add one

**DO NOT proceed to Step 7 until `cm --sanity-check` reports zero errors AND zero warnings.**

---

## Step 2CM: Generate tasks.json for /cm Session

**This step is ONLY for /cm sessions.** If you're in a `/feat` or `/fix` session, skip to Step 2.

Create `.cm/tasks.json` from PLAN.md phases:

### 2CM.1 Generate Project Metadata

```json
{
  "version": "1.0.0",
  "project": {
    "name": "[project-name from PLAN.md title]",
    "description": "[description from PLAN.md subtitle]",
    "created_at": "[current ISO 8601 timestamp]"
  },
  "global_context": {
    "plan_summary": "[overview text from PLAN.md]"
  }
}
```

### 2CM.2 Generate Phases and Tasks

For each phase in PLAN.md:

1. Create a phase object with sequential ID (`phase-1`, `phase-2`, etc.)
2. For each task listed under that phase:
   - Create task with ID format: `phase-N.task-M`
   - Set type to "implement"
   - Set status to "pending"
   - Use the task description from PLAN.md as the name
   - Set `plan_file` to reference the plan file for that phase (e.g., ".cm/plans/plan-1.md")
   - Add relevant files to `files_to_read` if mentioned in PLAN.md

Example:

```json
{
  "phases": [
    {
      "id": "phase-1",
      "name": "Foundation",
      "status": "pending",
      "tasks": [
        {
          "id": "phase-1.task-1",
          "name": "Initialize project structure",
          "type": "implement",
          "status": "pending",
          "depends_on": [],
          "context": {
            "files_to_read": []
          },
          "plan_file": ".cm/plans/plan-1.md",
          "roadmap_item_id": "phase-1-item-1"
        }
      ]
    }
  ]
}
```

### 2CM.3 Add Empty History

```json
{
  "current_phase": null,
  "current_task": null,
  "agent_history": [],
  "log_records": []
}
```

### 2CM.4 Write tasks.json

Write the complete JSON structure to `.cm/tasks.json`.

### 2CM.5 Create Plan Files

For each phase, create a plan file at `.cm/plans/plan-N.md` containing detailed instructions for that phase's tasks.

**Reference:** See `.cm/agents/TASK_PLAN_TEMPLATE.md` for the complete template structure.

1. Create the `.cm/plans/` directory if it doesn't exist
2. For each phase, create a plan file following the template with:
   - **Objective** - What the phase/task accomplishes
   - **Files to Read** - Context files with line ranges for large files
   - **Implementation Steps** - Numbered steps with file paths and line numbers
   - **Files to Modify/Create** - Specific files with changes
   - **Verification** - Commands and checks
   - **Reviewer Criteria** - Must-check and may-skip items

**Tip:** After creating minimal plans, run `/expand` to enrich them with detailed file references, code examples, and reviewer criteria.

Example `.cm/plans/plan-1.md`:
```markdown
# Phase 1: Foundation

## Objective
Set up the initial project structure and dependencies.

## Files to Read
- None (new project)

## Implementation Steps

### Task 1: Initialize project structure

1. **Create project** (project root)
   - Run `cargo init`
   - Verify Cargo.toml is created

2. **Add dependencies** (`Cargo.toml:6-12`)
   - Add clap = "4"
   - Add serde = { version = "1", features = ["derive"] }

3. **Create CLI entry point** (`src/main.rs:1-30`)
   - Set up basic Clap argument parsing
   - Add help and version flags

## Verification
- [ ] `cargo build` succeeds
- [ ] All dependencies resolve correctly

## Reviewer Criteria
**Must check:**
- [ ] Project builds without warnings
- [ ] Dependencies are at latest stable versions
```

---

## Step 3CM: Generate roadmap.json for /cm Session

**This step is ONLY for /cm sessions.** If you're in a `/feat` or `/fix` session, skip to Step 3.

Create `.cm/roadmap.json` from PLAN.md phases:

### 3CM.1 Generate Roadmap Structure

```json
{
  "version": "1.0.0",
  "title": "[Project name from PLAN.md]",
  "phases": []
}
```

### 3CM.2 Generate Roadmap Phases

For each phase in PLAN.md, create a roadmap phase with items:

```json
{
  "id": "phase-1",
  "number": "1",
  "name": "Foundation",
  "items": [
    {
      "id": "phase-1-item-1",
      "name": "Initialize project structure",
      "completed": false,
      "linked_task_ids": ["phase-1.task-1"]
    },
    {
      "id": "phase-1-item-2",
      "name": "Create CLI argument parsing",
      "completed": false,
      "linked_task_ids": ["phase-1.task-2"]
    }
  ]
}
```

**Linking strategy:**
- Each task in tasks.json should have a corresponding roadmap item
- Use `roadmap_item_id` in tasks.json to link to the item
- Use `linked_task_ids` in roadmap.json to link back to tasks

### 3CM.3 Write roadmap.json

Write the complete JSON structure to `.cm/roadmap.json`.

---

## Step 4CM: Create Plan Files for /cm Session (Optional)

**This step is ONLY for /cm sessions.**

If the project structure includes individual plan files (`.cm/plans/plan-N.md`), create them:

For each phase:
1. Create `.cm/plans/plan-N.md` where N is the phase number
2. Include phase details, task breakdown, and implementation notes

**Note:** This is optional. Many projects don't use separate plan files.

---

## Step 5CM: Regenerate and Validate for /cm Session

**This step is ONLY for /cm sessions.**

After generating JSON files:

1. Run regeneration:
```bash
cm --regenerate
```

This generates:
- `ROADMAP.md` from roadmap.json
- `TASKS.md` from tasks.json

2. Run validation:
```bash
cm --sanity-check
```

**If validation fails:**
- Review error messages
- Fix issues in tasks.json or roadmap.json
- Re-run `cm --sanity-check`
- Repeat until all errors are resolved

**DO NOT proceed until validation passes.**

After validation passes, proceed to Step 7CM.

---

## Step 7CM: Completion Summary for /cm Session

**This step is ONLY for /cm sessions.**

After all files are generated and validated, inform the user:

> **Project initialized successfully!**
>
> **Generated files:**
> - `.cm/tasks.json` - [N] phases, [M] tasks
> - `.cm/roadmap.json` - [N] phases, [M] items
> - `.cm/ROADMAP.md` - Generated from roadmap.json
> - `.cm/TASKS.md` - Generated from tasks.json
>
> **Phases created:**
> 1. Phase 1: [name] - [X] tasks
> 2. Phase 2: [name] - [Y] tasks
> ...
>
> **Validation:** ✓ All checks passed
>
> **Next steps:**
> 1. Review the generated files to ensure accuracy
> 2. When ready to implement, run `cm` to start executing tasks
> 3. Or run `cm --step` to execute tasks one at a time
>
> Your project is now ready for automated implementation!

**END of /cm session flow. The steps below (Step 7) are for /feat and /fix sessions only.**

---

## Step 7: Completion Summary (for /feat and /fix sessions)

**This step is ONLY for /feat and /fix sessions.** For /cm sessions, see Step 7CM above.

After all files are updated and validated, inform the user:

> **Session finalized successfully!**
>
> **Changes saved:**
> - `.cm/tasks.json` - Added [N] new task(s)
> - `.cm/roadmap.json` - Added [M] new roadmap item(s)
> [For /feat only:] - `.cm/PLAN.md` - Added [feature name] documentation
>
> **New tasks added:**
> - `[task-id-1]`: [task-name-1]
> - `[task-id-2]`: [task-name-2]
> - `[task-id-3]`: [task-name-3]
>
> **Validation:** ✓ All checks passed
>
> **Next steps:**
> 1. Review the updated files to ensure accuracy
> 2. When ready to implement, run `cm` to start executing tasks
> 3. Or run `cm --step` to execute tasks one at a time
>
> The feature/fix has been saved to your planning files and is ready for implementation whenever you choose to run it.

---

## Error Handling

If the wizard encounters issues:

### Missing .cm/ Directory

> I couldn't find the `.cm/` directory. This command requires an initialized cm project.
>
> Please run `cm init` and then `/cm` to initialize the project.

### Missing PLAN.md (for /cm sessions)

> I couldn't find `.cm/PLAN.md`. This file is required to generate tasks.json and roadmap.json.
>
> Please run `/cm` first to create the project plan, then return here to finalize with `/end`.

### tasks.json already exists (for /cm sessions)

> Warning: `tasks.json` already exists. The `/end` command after `/cm` is meant for initial project setup.
>
> It looks like your project is already initialized. Use `/feat` or `/fix` to add new features or fixes instead.

### Invalid tasks.json

> The current `tasks.json` file appears to be invalid or corrupted.
>
> Please run `cm --sanity-check` to identify issues, then fix them before using `/end`.

### Conflicting Task IDs

> Task ID "[id]" already exists in tasks.json. I'll use "[new-id]" instead.
>
> **Original:** [id]
> **Updated:** [new-id]
>
> Is this acceptable? (yes/no)

Wait for user confirmation before proceeding.

### Invalid Phase Reference

> Phase "[phase-id]" not found in tasks.json.
>
> Available phases:
> - [phase-1-id]: [phase-1-name]
> - [phase-2-id]: [phase-2-name]
>
> Which phase should I add the tasks to?

Wait for user to select a valid phase.

### Invalid Roadmap Item ID

> Roadmap item ID "[id]" doesn't exist in roadmap.json.
>
> The task references this item via `roadmap_item_id`, but it's not defined.
>
> Options:
> 1. Remove the `roadmap_item_id` field (no linking)
> 2. Create the roadmap item "[id]"
>
> Which option? (1/2)

Wait for user choice.

### Cross-Reference Validation Failures

If `cm --sanity-check` reports cross-reference errors:

> **Validation failed with cross-reference errors:**
>
> [Display error messages from cm --sanity-check]
>
> **Fixing issues:**
> - [Describe fix for error 1]
> - [Describe fix for error 2]
>
> I'll update the files to resolve these issues...

Then fix the errors and re-run validation until it passes.

---

## Schema References

**See `/cm` for full tasks.json and roadmap.json schemas.**

Quick reference for task ID naming:
- **Feature tasks:** `phase-X.feat-[name].task-N`
- **Fix tasks:** `phase-X.fix-[name]`
- **Review/Test tasks:** `phase-X.task-N.review` / `phase-X.task-N.test`

**Dependency rule:** Tasks can only depend on tasks from the same or earlier phases. No circular dependencies.

---

## Tips for Effective Session Finalization

1. **Verify completeness** - Ensure all discussed tasks are included
2. **Check consistency** - Task names and descriptions should match what was discussed
3. **Validate dependencies** - Make sure task ordering makes sense
4. **Link roadmap items** - Use `roadmap_item_id` to keep checkboxes in sync
5. **Test validation** - Always run `cm --sanity-check` before finishing
6. **Clear summary** - Provide a comprehensive summary so the user knows what was saved
7. **Review opportunity** - Remind users to review files before running `cm run`

---

## Example Usage

### Example: /feat Session

```
User: Save the export feature to planning files.

Agent: Saving export feature:
- 3 tasks (CSV, JSON, PDF export)
- Phase: phase-2
Proceed? (yes/no)

User: yes

Agent: [Updates tasks.json, roadmap.json, PLAN.md]
Agent: [Runs cm --regenerate && cm --sanity-check]

**Session finalized!**
- Added 3 tasks to tasks.json
- Added roadmap item with linked_task_ids
- Validation: ✓ passed

Run `cm` when ready to implement.
```

### Example: /fix Session

```
User: Save the memory leak fix.

Agent: Saving fix:
- 1 task: Fix memory leak in connection pool
- Phase: phase-1 (priority)
Proceed? (yes/no)

User: yes

Agent: [Updates tasks.json, roadmap.json]
Agent: [Runs cm --regenerate && cm --sanity-check]

**Session finalized!**
- Added `phase-1.fix-memory-leak` task
- Validation: ✓ passed

Run `cm` when ready to implement.
```

---

## Summary

The `/end` command bridges the gap between planning and execution by:

**For /feat and /fix sessions:**
1. Capturing all details from conversational sessions
2. Updating planning files (tasks.json, roadmap.json, PLAN.md)
3. Ensuring consistency with validation
4. Providing clear summary of what was saved

**For /cm sessions:**
1. Parsing PLAN.md to extract phases and tasks
2. Generating initial tasks.json and roadmap.json
3. Creating the foundation for automated implementation
4. Validating all cross-references and structure

This allows users to have thoughtful planning conversations, save the results, and execute them later when ready - without losing context or requiring re-discussion.

## Workflow Summary

```
/cm           → Creates high-level PLAN.md (interactive wizard)
    ↓
/split        → (optional) Refines PLAN.md into detailed phases
    ↓
/end          → Finalizes session, generates tasks.json + roadmap.json
    ↓
cm            → Executes tasks with automated agents
```

Or for adding to existing projects:

```
/feat or /fix → Discuss feature/fix requirements
    ↓
/end          → Add tasks to existing tasks.json/roadmap.json
    ↓
cm            → Execute new tasks
```

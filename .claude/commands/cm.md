# CM Project Wizard

This command guides users through creating a complete cm (Claude Code Manager) project setup. The wizard collects information through a conversational flow and then generates all necessary artifacts in the `.cm/` directory.

## Important: Interactive Flow

You MUST follow this wizard flow step by step. Do NOT skip steps or generate files until you have gathered all the required information and received user confirmation.

---

## Step 0: Prerequisites Check

Before starting the wizard, verify that `cm init` has been run:

1. Check if `.claude/commands/cm.md` exists (you're reading this, so it does)
2. Check if `.cm/agents/` directory exists with template files

If `.cm/agents/` is missing or incomplete, inform the user:

> **Prerequisites not met.**
>
> Please run `cm init` in your terminal first to set up the required files:
>
> ```bash
> cm init
> ```
>
> This will create:
> - `.claude/commands/` - Claude Code command files
> - `.cm/agents/` - Agent template files
>
> After running `cm init`, return here and run `/cm` again.

Wait for user confirmation that they've run `cm init` before proceeding.

If prerequisites are met, proceed to Step 1.

---

## Step 1: Project Definition

Ask these questions in sequence, waiting for each response:

1. **Project name**: "What is the **project name**? (e.g., my-app, api-server)"
2. **Description**: "What does this project do? (one sentence)"
3. **Goal**: "What is the **overall goal**? (What problem does it solve?)"

---

## Step 2: Scope and Goals

Ask these questions in sequence, waiting for each response:

1. **Success criteria**: "How will we know when the project is **complete**? (2-4 measurable criteria)"
2. **Key features**: "What are the **key features**? (Main capabilities)"
3. **Constraints**: "Any **constraints**? (Technical, deadlines - or 'none')"
4. **Out of scope**: "What should we **NOT include**? (Things to avoid - or 'nothing specific')"

---

## Step 3: Reference Implementation (Optional)

**Ask the user:**

> Do you have any reference implementations or existing code to analyze?
>
> This could be:
> - An existing codebase in this repo to follow patterns from
> - A library or framework you want to integrate with
> - Documentation or specifications to follow
> - Similar projects to use as inspiration
>
> If yes, please provide file paths or URLs. If no, just say "skip" or "none".

If the user provides references:
- Analyze the code structure and patterns
- Identify coding conventions and styles
- Note any architectural patterns to follow
- Extract relevant type definitions or interfaces

Wait for the user's response before proceeding.

---

## Step 4: Phase Breakdown

**Ask the user:**

> Let's break this down into phases. What major phases do you see?
>
> Consider phases like:
> - Setup/scaffolding
> - Core implementation
> - Features/modules
> - Testing
> - Integration
> - Documentation
>
> For each phase, briefly describe:
> - What gets built
> - Dependencies on other phases

If the user needs help, suggest a reasonable phase breakdown based on the project description.

Wait for the user's response before proceeding.

---

## Step 5: Project Structure

**Ask the user:**

> **Describe the project structure:**
>
> - What are the main directories? (e.g., src/, tests/, docs/)
> - What is the entry point? (e.g., main.rs, index.ts)
> - Any key configuration files? (e.g., Cargo.toml, package.json)
>
> This will be used to customize `.cm/STRUCTURE.md`. You can skip this to use the default template.

Wait for the user's response before proceeding.

**If the user provides structure information:**
- Store it for STRUCTURE.md customization in Step 8
- Plan to populate the template sections with specific details

**If the user skips:**
- Use the default STRUCTURE.md template as-is
- The template sections will remain as placeholders for manual editing later

---

## Step 6: Build & Test Actions

**Ask the user:**

> **What commands should be used for:**
>
> - Building the project? (e.g., `cargo build`, `npm run build`)
> - Running tests? (e.g., `cargo test`, `npm test`)
> - Linting/checking? (e.g., `cargo clippy`, `npm run lint`)
>
> These commands will be:
> 1. Stored in `.cm/config.toml` as `build_commands` for automatic verification after each phase
> 2. Documented in `.cm/ACTIONS.md` for reference
>
> If you skip this step, build verification will be disabled (phases complete without running any build commands).

Wait for the user's response before proceeding.

**If the user provides build/test/lint commands:**
- Store the build and lint commands (NOT test commands) in `build_commands` for config.toml
- Store all commands for ACTIONS.md customization in Step 8
- Example: If user says "cargo build, cargo clippy, cargo test", store `["cargo build", "cargo clippy"]` in config.toml (exclude tests - they run separately)

**If the user skips:**
- Leave `build_commands` empty in config.toml (build verification will be skipped)
- Use the default ACTIONS.md template with placeholder text

---

## Step 7: Confirmation

**Present a summary to the user:**

> ## Project Summary
>
> **Project:** [name]
> **Description:** [description]
>
> **Goals:**
> - [goal 1]
> - [goal 2]
>
> **Phases:**
> 1. [Phase 1 name] - [brief description]
> 2. [Phase 2 name] - [brief description]
> ...
>
> **Files to generate:**
> - `.cm/PLAN.md` - High-level project plan
> - `.cm/ROADMAP.md` - Detailed checklist with checkboxes
> - `.cm/tasks.json` - Machine-readable task definitions
> - `.cm/TASKS.md` - Task status overview (generated from tasks.json)
>
> **Project Structure:**
> - Directories: [list]
> - Entry point: [file]
> - Config files: [list]
>
> **Build & Test:**
> - Build: `[command]`
> - Test: `[command]`
> - Lint: `[command]`
>
> Does this look correct? Reply "yes" to generate the files, or provide corrections.

Wait for explicit user confirmation before generating files.

---

## Step 8: Generate Artifacts

Once confirmed, generate all files in the `.cm/` directory:

1. Create the `.cm/` directory if it doesn't exist
2. Generate `PLAN.md` using the PLAN.md Template below
3. Generate `roadmap.json` using the roadmap.json Schema below
4. Generate `ROADMAP.md` from roadmap.json (or use template for initial creation)
5. Generate `tasks.json` using the tasks.json Schema below
6. Generate `TASKS.md` from tasks.json
7. Generate `.cm/agents/IMPLEMENTER.md` using the Implementer Agent Template below
8. Generate `.cm/agents/REVIEWER.md` using the Reviewer Agent Template below
9. Generate `.cm/STRUCTURE.md`:
    - If user provided project structure info in Step 5: customize template with specific directories, entry points, and config files
    - If user skipped Step 5: use default STRUCTURE.md template with placeholder text
    - If file doesn't exist, create it from template
10. Generate `.cm/ACTIONS.md`:
    - If user provided build/test/lint commands in Step 6: customize template with actual commands
    - If user skipped Step 6: use default ACTIONS.md template with placeholder text like `[build command]`
    - If file doesn't exist, create it from template
11. Generate `.cm/config.toml`:
    - If user provided build/lint commands in Step 6: include them as `build_commands`
    - If user skipped Step 6: create empty config (build verification will be skipped)
    - Example config.toml:
      ```toml
      # CM Configuration
      # Build commands to run for verification after each phase
      # If empty or not specified, build verification is skipped
      build_commands = ["cargo build", "cargo clippy"]
      ```

**Note:** JSON files (tasks.json, roadmap.json) are the source of truth. Markdown files (ROADMAP.md, TASKS.md) can be regenerated from JSON at any time using `cm --regenerate`.

After generation, inform the user:

> CM project initialized successfully!
>
> Created files:
> - `.cm/PLAN.md` - Review and refine the high-level plan
> - `.cm/roadmap.json` - Source of truth for roadmap progress
> - `.cm/ROADMAP.md` - Human-readable roadmap (generated from roadmap.json)
> - `.cm/tasks.json` - Used by cm to orchestrate agents
> - `.cm/TASKS.md` - Task status overview (generated from tasks.json)
> - `.cm/config.toml` - Configuration (build commands for verification)
> - `.cm/agents/IMPLEMENTER.md` - Implementer agent instructions
> - `.cm/agents/REVIEWER.md` - Reviewer agent instructions
> - `.cm/STRUCTURE.md` - Project structure documentation
> - `.cm/ACTIONS.md` - Build and test commands
>
> Proceeding to validation...

---

## Step 9: Validate Generated Files

After generating the files, run validation to ensure all JSON files are correct:

```bash
cm --sanity-check
```

Check the output:
- If validation **passes**: Inform the user and proceed to Step 10
- If validation **fails**:
  1. Review the error messages
  2. Fix the issues in the JSON files (tasks.json or roadmap.json)
  3. Re-run `cm --sanity-check`
  4. Repeat until all errors are resolved

**Important:** Do NOT proceed to the next step until validation passes. Common issues include:
- Invalid cross-references (roadmap_item_id pointing to non-existent item)
- Duplicate IDs in tasks or roadmap items
- Missing required fields

---

## Step 10: Generate Commit Message

Generate a commit message for the initial setup:

```
cm: Initialize [project-name] with [N] phases and [M] tasks
```

**Show the user:**

> Proposed commit message:
> ```
> cm: Initialize [project-name] with [N] phases and [M] tasks
> ```
>
> Would you like to use this message, or provide your own?

Wait for user to confirm or provide alternative.

---

## Step 11: Commit Changes

**Ask the user:**

> Ready to commit the `.cm/` directory with the message:
> ```
> [commit message]
> ```
>
> Proceed with commit? (yes/no)

If **yes**:
1. Stage the `.cm/` directory: `git add .cm/`
2. Commit with the message: `git commit -m "[message]"`
3. Inform user: "Committed successfully!"

If **no**:
> No problem! You can commit manually later with:
> ```bash
> git add .cm/
> git commit -m "cm: Initialize [project-name]"
> ```

---

## Templates

### PLAN.md Template

```markdown
# [Project Name]

> [One-sentence description]

## Overview

[2-3 paragraph overview explaining what this project does, why it exists, and the high-level approach]

## Goals

- [Primary goal]
- [Secondary goal]
- [Additional goals...]

## Success Criteria

- [ ] [Criterion 1]
- [ ] [Criterion 2]
- [ ] [Criterion 3]

## Architecture

[Describe the high-level architecture, main components, and how they interact]

### Components

1. **[Component 1]** - [Purpose and responsibility]
2. **[Component 2]** - [Purpose and responsibility]
3. **[Component 3]** - [Purpose and responsibility]

### Data Flow

[Describe how data flows through the system]

## Modules

### [Module 1 Name]

**Purpose:** [What this module does]

**Key files:**
- `path/to/file1.rs` - [Description]
- `path/to/file2.rs` - [Description]

**Dependencies:** [What this module depends on]

### [Module 2 Name]

[Same structure as above]

## Phases

### Phase 1: [Name]

**Goal:** [What this phase accomplishes]

**Tasks:**
- [Task 1]
- [Task 2]

**Deliverables:**
- [Deliverable 1]
- [Deliverable 2]

### Phase 2: [Name]

[Same structure as above]

## Technical Decisions

### [Decision 1]

**Context:** [Why this decision was needed]
**Decision:** [What was decided]
**Rationale:** [Why this choice was made]

## Out of Scope

- [Item 1]
- [Item 2]

## References

- [Reference 1]
- [Reference 2]
```

### ROADMAP.md Template

```markdown
# [Project Name] - Roadmap

This document tracks implementation progress. Check off items as they are completed.

## Phase 1: [Phase Name]

Status: [ ] Not Started / [ ] In Progress / [ ] Complete

### [Section 1]

- [ ] [Task 1.1] - [Brief description]
- [ ] [Task 1.2] - [Brief description]
  - [ ] [Subtask 1.2.1]
  - [ ] [Subtask 1.2.2]
- [ ] [Task 1.3] - [Brief description]

### [Section 2]

- [ ] [Task 2.1] - [Brief description]
- [ ] [Task 2.2] - [Brief description]

---

## Phase 2: [Phase Name]

Status: [ ] Not Started / [ ] In Progress / [ ] Complete

### [Section 1]

- [ ] [Task 1] - [Brief description]
- [ ] [Task 2] - [Brief description]

---

## Phase 3: [Phase Name]

[Continue pattern...]

---

## Summary

| Phase | Status | Progress |
|-------|--------|----------|
| Phase 1: [Name] | Not Started | 0/X |
| Phase 2: [Name] | Not Started | 0/X |
| Phase 3: [Name] | Not Started | 0/X |
| **Total** | | 0/X |
```

### tasks.json Schema

The `tasks.json` file follows this schema (based on `src/state/tasks.rs`):

```json
{
  "version": "1.0.0",
  "project": {
    "name": "project-name",
    "description": "Project description",
    "created_at": "2024-01-15T10:30:00Z"
  },
  "global_context": {
    "plan_summary": "High-level summary of what this project does and how"
  },
  "phases": [
    {
      "id": "phase-1",
      "name": "Phase 1 Name",
      "status": "pending",
      "tasks": [
        {
          "id": "phase-1.task-1",
          "name": "Task Name",
          "type": "implement",
          "status": "pending",
          "depends_on": [],
          "context": {
            "files_to_read": ["src/relevant/file.rs"],
            "code_style_excerpt": "Relevant style guidelines if any"
          },
          "plan_file": ".cm/plans/plan-1.md"
        }
      ]
    }
  ],
  "current_phase": null,
  "current_task": null,
  "agent_history": []
}
```

#### Field Definitions

> **Note:** See `.cm/SCHEMA.md` for the complete authoritative schema reference.

**TasksState (root)**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| version | string | Yes | Schema version (use "1.0.0") |
| project | Project | Yes | Project metadata |
| global_context | GlobalContext | No | Shared context for all tasks |
| phases | Phase[] | Yes | List of project phases |
| current_phase | string | No | ID of active phase |
| current_task | string | No | ID of active task |
| agent_history | AgentInvocation[] | No | History of agent runs |
| log_records | LogRecord[] | No | Structured log records for audit trail |

**Project**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | string | Yes | Project identifier |
| description | string | Yes | Project description |
| created_at | datetime | No | ISO 8601 timestamp |

**GlobalContext**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| plan_summary | string | Yes | High-level plan summary |

**Phase**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| id | string | Yes | Unique phase ID (e.g., "phase-1") |
| name | string | Yes | Human-readable name |
| status | PhaseStatus | Yes | "pending", "in_progress", or "completed" |
| tasks | Task[] | Yes | Tasks in this phase |

**Task**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| id | string | Yes | Unique task ID (e.g., "phase-1.task-1") |
| name | string | Yes | Human-readable name |
| type | TaskType | Yes | "implement", "review", "fix", or "test" |
| status | TaskStatus | Yes | "pending", "in_progress", "completed", or "deferred" |
| depends_on | string[] | No | IDs of tasks this depends on |
| context | TaskContext | Yes | Context for the agent |
| plan_file | string | Yes | Path to plan file (e.g., ".cm/plans/plan-1.md") |
| attempts | TaskAttempt[] | No | Execution history |
| roadmap_item_id | string | No | ID of linked roadmap item (for roadmap sync) |

**TaskContext**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| files_to_read | string[] | No | Files to read for context |
| code_style_excerpt | string | No | Relevant style guidelines |
| prior_review_issues | string[] | No | Issues from prior reviews |

#### Task ID Convention

Use the format `phase-{n}.task-{m}` for task IDs:
- `phase-1.task-1` - First task in phase 1
- `phase-1.task-2` - Second task in phase 1
- `phase-2.task-1` - First task in phase 2

For review/fix cycles, extend with a suffix:
- `phase-1.task-1.review` - Review of task 1
- `phase-1.task-1.fix` - Fix issues from review

#### Plan Files

Tasks reference plan files via the `plan_file` field. Plan files contain detailed implementation instructions and are stored at `.cm/plans/`:

- `.cm/plans/plan-1.md` - Plan for phase 1 tasks
- `.cm/plans/plan-2.md` - Plan for phase 2 tasks

**When generating tasks.json, also create the corresponding plan files** with detailed instructions for each phase.

#### Dependency Rules

- Tasks can only depend on tasks from the same phase or earlier phases
- Do not create circular dependencies
- Use dependencies to ensure proper ordering (e.g., implementation before testing)

### roadmap.json Schema

The `roadmap.json` file is the source of truth for ROADMAP.md:

```json
{
  "version": "1.0.0",
  "title": "Project Name",
  "phases": [
    {
      "id": "phase-1",
      "number": "1",
      "name": "Phase Name",
      "items": [
        {
          "id": "item-1",
          "name": "Item Name",
          "completed": false,
          "sub_items": [
            { "name": "Sub-item name", "completed": false }
          ],
          "linked_task_ids": ["phase-1.task-1"]
        }
      ]
    }
  ]
}
```

**RoadmapState (root)**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| version | string | Yes | Schema version (use "1.0.0") |
| title | string | Yes | Project title |
| phases | RoadmapPhase[] | Yes | List of phases |

**RoadmapPhase**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| id | string | Yes | Unique phase ID |
| number | string | Yes | Phase number (e.g., "1", "2", "0.5") |
| name | string | Yes | Phase name |
| items | RoadmapItem[] | Yes | Items in this phase |

**RoadmapItem**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| id | string | Yes | Unique item ID |
| name | string | Yes | Item name |
| completed | boolean | Yes | Whether item is completed |
| sub_items | RoadmapSubItem[] | No | Sub-items |
| linked_task_ids | string[] | No | IDs of linked tasks |

**RoadmapSubItem**
| Field | Type | Required | Description |
|-------|------|----------|-------------|
| name | string | Yes | Sub-item name |
| completed | boolean | Yes | Whether sub-item is completed |

---
```

### IMPLEMENTER.md Template

```markdown
# Implementer Agent

This agent implements code changes for [Project Name].

## Role
- Write new code based on task instructions
- Modify existing code as directed
- Follow project code style
- Ensure builds pass

## Key Files
[List from STRUCTURE.md]

## Guidelines
1. Read context files before implementing
2. Follow existing code patterns
3. Run `[build command]` before completing
4. Run `[lint command]` to check style
```

### REVIEWER.md Template

```markdown
# Reviewer Agent

This agent reviews code changes for [Project Name].

## Role
- Review implementations for correctness
- Check code style compliance
- Identify bugs and issues
- Suggest improvements

## Guidelines
1. Check all changed files
2. Verify requirements are met
3. Run tests: `[test command]`
4. Report issues with specific locations
```

### STRUCTURE.md Template

Default template (used when user skips customization):

```markdown
# Project Structure

## Directories
[List main directories and their purposes]

## Entry Points
[Main entry points]

## Configuration
[Key config files]
```

Customized template (when user provides structure info):

```markdown
# Project Structure

## Directories
- `[directory1]/` - [purpose]
- `[directory2]/` - [purpose]
- `[directory3]/` - [purpose]

## Entry Points
- `[entry_point_file]` - [description]

## Configuration
- `[config_file1]` - [description]
- `[config_file2]` - [description]
```

### ACTIONS.md Template

Default template (used when user skips customization):

```markdown
# Build & Test Actions

## Build
```bash
[build command]
```

## Test
```bash
[test command]
```

## Lint
```bash
[lint command]
```
```

Customized template (when user provides commands):

```markdown
# Build & Test Actions

## Build
```bash
[actual build command provided by user]
```

## Test
```bash
[actual test command provided by user]
```

## Lint
```bash
[actual lint command provided by user]
```
```

---

## Example Generation

Example output for a CLI tool project:

### Example PLAN.md

```markdown
# my-cli-tool

> A command-line tool for processing JSON files

## Overview

Rust CLI that reads JSON, applies transformations, and outputs in multiple formats.

## Goals

- Parse and validate JSON input
- Apply transformations (filter, map, aggregate)
- Output as JSON, CSV, or table

## Phases

### Phase 1: Foundation
- Initialize Cargo project with clap, serde, serde_json
- Create basic CLI structure

### Phase 2: Core Implementation
- JSON parser module
- Transformation operations
- Error handling
```

### Example tasks.json

```json
{
  "version": "1.0.0",
  "project": {
    "name": "my-cli-tool",
    "description": "CLI tool for processing JSON files"
  },
  "global_context": {
    "plan_summary": "Rust CLI for JSON transformations with multiple output formats"
  },
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
          "context": { "files_to_read": [] },
          "plan_file": ".cm/plans/plan-1.md"
        }
      ]
    }
  ],
  "current_phase": null,
  "current_task": null,
  "agent_history": []
}
```

---

## Tips for Good Task Definitions

1. **Be specific** - Include exact file paths, function names, and requirements
2. **One task, one goal** - Each task should have a single clear objective
3. **Include context** - List files the agent should read for understanding
4. **Specify success criteria** - How will we know the task is complete?
5. **Handle dependencies** - Ensure tasks are ordered correctly
6. **Keep plan files actionable** - Use imperative language in plan files ("Create X", "Implement Y")

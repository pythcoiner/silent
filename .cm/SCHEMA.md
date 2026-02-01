# CM JSON Schemas

This document is the authoritative reference for all CM JSON schemas. Skills (`/cm`, `/end`, `/feat`, `/fix`) should reference this file for schema definitions.

---

## tasks.json Schema

The `tasks.json` file is the source of truth for task state. Markdown files (TASKS.md) are regenerated from this JSON.

### Example Structure

```json
{
  "version": "1.0.0",
  "project": {
    "name": "project-name",
    "description": "Project description"
  },
  "global_context": {
    "plan_summary": "High-level summary of the project plan"
  },
  "phases": [
    {
      "id": "phase-1",
      "name": "Phase Name",
      "plan": "",
      "status": "pending",
      "tasks": [
        {
          "id": "phase-1.task-1",
          "name": "Task Name",
          "type": "implement",
          "status": "pending",
          "depends_on": [],
          "context": {
            "files_to_read": ["src/relevant/file.rs"]
          },
          "plan_file": ".cm/plans/plan-1.md",
          "roadmap_item_id": "item-1"
        }
      ]
    }
  ]
}
```

### Field Definitions

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
| plan | string | No | Phase-level plan content |
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

### Task ID Convention

Use the format `phase-{n}.task-{m}` for task IDs:
- `phase-1.task-1` - First task in phase 1
- `phase-1.task-2` - Second task in phase 1
- `phase-2.task-1` - First task in phase 2

For features/fixes, use descriptive names:
- `phase-1.feat-auth.impl-1` - First implementation task for auth feature
- `phase-1.fix-login` - Fix for login issue

### Dependency Rules

- Tasks can only depend on tasks from the same phase or earlier phases
- Do not create circular dependencies
- Use dependencies to ensure proper ordering (e.g., implementation before testing)

---

## Plan Files

Plan files contain the detailed instructions for agents. They are stored separately from `tasks.json` to keep the JSON clean and allow for richer markdown content.

### Location

Store plan files at: `.cm/plans/plan-{number}.md`

Examples:
- `.cm/plans/plan-1.md` - Plan for all tasks in phase 1
- `.cm/plans/plan-2.md` - Plan for all tasks in phase 2

### Referencing

Each task must have a `plan_file` field pointing to its plan file:

```json
{
  "id": "phase-1.task-1",
  "plan_file": ".cm/plans/plan-1.md",
  ...
}
```

Multiple tasks in the same phase can share a plan file.

### Plan File Content

Plan files should contain:
- Detailed implementation instructions
- Specific requirements and expected behavior
- Edge cases to handle
- Files to create or modify
- Success criteria

Example plan file:
```markdown
# Phase 1: Project Setup

## Tasks

### Task 1: Initialize Rust Project

1. Create the project structure with `cargo init`
2. Add dependencies to Cargo.toml:
   - clap = "4"
   - serde = { version = "1", features = ["derive"] }
   - serde_json = "1"
3. Create src/main.rs with basic CLI setup
4. Ensure `cargo build` succeeds

### Success Criteria
- Project builds without errors
- All dependencies resolve correctly
```

---

## roadmap.json Schema

The `roadmap.json` file is the source of truth for ROADMAP.md:

### Example Structure

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

### Field Definitions

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

### Linking Tasks to Roadmap Items

- One roadmap item can represent multiple tasks
- Use `linked_task_ids` to track which tasks complete this item
- The item is marked completed when ALL linked tasks are completed
- Tasks reference roadmap items via `roadmap_item_id` field

# Bug Fix Wizard

This command guides users through adding a bug fix task to an existing cdm (Codex Manager) project. The wizard collects bug details, investigates autonomously, and presents a complete fix plan for the user to confirm via `/end`.

## Prerequisites

Requires: `.cdm/` directory with `tasks.json`. If not present, run `/cdm` first.

## CRITICAL: Scope Limitations

This command ONLY updates planning files:
- `.cdm/tasks.json` - Add fix task definition
- `.cdm/ROADMAP.md` - Optionally add checkbox entry

This command does NOT:
- Implement any code fixes
- Run `cdm run` or execute tasks
- Make changes outside `.cdm/` directory

After the wizard completes, the user must manually run `cdm run` to start the fix.

## CRITICAL: Minimal Interaction

The wizard has exactly TWO user interactions:
1. **Step 1** — User describes the bug
2. **Step 5** — Handoff to `/end` (user confirms by running `/end`)

Everything in between (classification, investigation, fix approach, phase plan, task placement) is done AUTONOMOUSLY by the assistant. Do NOT ask for confirmation at intermediate steps. Do NOT ask "does this match?", "does this look correct?", "agree?", etc. Just do the work and present the final result.

If the user wants to correct something, they will say so after seeing the summary. You do NOT need to ask.

---

## Step 1: Bug Description

**Ask the user:**

> **Describe the bug you want to fix.**
>
> Include whatever is relevant:
> - What's happening incorrectly
> - What should happen instead
> - How to reproduce (if known)

Wait for the user's response before proceeding.

---

## Step 2: Autonomous Analysis

After receiving the bug description, do ALL of the following WITHOUT asking for confirmation:

### 2.1: Classify the Bug

Infer severity and category from the description:
- **Severity**: Critical / High / Medium / Low
- **Category**: Logic error / Crash/panic / Performance / UI/UX / Data corruption / Security / Other

### 2.2: Investigate the Codebase

1. Read relevant files mentioned by the user
2. Search for related code patterns
3. Identify the root cause location
4. Determine affected components

### 2.3: Design the Fix

1. Determine the fix approach
2. List required changes
3. Assess risk and testing strategy

### 2.4: Determine Task Placement

Analyze existing phases in `tasks.json` and decide placement:
- If there's a current active phase with related work, add to it
- Otherwise, create a new phase
- Use your judgment — don't ask the user

### 2.5: Generate Phase Plan

If creating a new phase, generate a detailed phase plan following the `.cdm/agents/PLAN.md` template.

---

## Step 3: Present Combined Summary

Present ALL results in a single message:

> ## Fix Summary
>
> **Bug:** [summary]
> **Severity:** [severity] | **Category:** [category]
>
> ### Investigation
> **Root cause:** [brief explanation]
> **Location:** `path/to/file.rs` - [specific function or line range]
>
> ### Fix Approach
> **Changes required:**
> 1. [File 1]: [Change description]
> 2. [File 2]: [Change description]
>
> **Risk:** [Low/Medium/High]
>
> ### Task
> **Task ID:** [generated-task-id]
> **Placement:** [phase and position]
>
> **Plan file:** `.cdm/plans/plan-[phase].md`
>
> **Task definition:**
> ```json
> {
>   "id": "[task-id]",
>   "name": "Fix: [summary]",
>   "type": "fix",
>   "status": "pending",
>   "depends_on": [],
>   "context": {
>     "files_to_read": ["affected/files.rs"],
>     "prior_review_issues": ["[bug description]"]
>   },
>   "plan_file": ".cdm/plans/plan-[phase].task-[n].md"
> }
> ```

Do NOT ask for confirmation here. Proceed directly to Step 4.

---

## Step 4: Handoff to /end

Immediately after the summary:

> Run `/end` to save changes to tasks.json and roadmap.json.
> Then run `cdm` when ready to implement.

Do NOT modify files. Wait for `/end`.

---

## Phase Template (for new fix phases)

When creating a new phase for a fix, use this JSON template. The `plan` field contains the detailed fix plan generated in Step 2.5.

```json
{
  "id": "phase-X",
  "name": "Fix: [Bug Summary]",
  "plan": "[Detailed fix plan following PLAN.md template - includes Objective, Root Cause Analysis, Fix Strategy, Implementation Steps, Files to Modify, Regression Prevention, and Verification]",
  "status": "pending",
  "tasks": [
    // Tasks go here (see template below)
  ]
}
```

---

## Fix Task Template

**IMPORTANT:** Create a **separate plan file per task** at `.cdm/plans/plan-X.task-Y.md` (where X is the phase number and Y is the task number). Each task's `plan_file` field must point to its own dedicated file containing only that task's instructions. **Never** point multiple tasks to the same plan file — this causes prompt bloat and review failures.

Create plan file at `.cdm/plans/plan-X.task-1.md`:
```markdown
Fix [bug summary]:

## Bug Details
- Observed: [behavior]
- Expected: [behavior]
- Location: [file:line or function]

## Root Cause
[Explanation of why this bug occurs]

## Fix Steps
1. [Step 1]
2. [Step 2]
3. [Step 3]

## Testing
1. Add unit test for the bug case:
   - Test input: [input]
   - Expected output: [output]
2. Verify existing tests still pass
3. Run `cargo build` and `cargo clippy`

## Verification
- [ ] Bug no longer reproduces
- [ ] No regressions in related functionality
- [ ] All tests pass
```

Task JSON:
```json
{
  "id": "phase-X.fix-[name]",
  "name": "Fix: [bug summary]",
  "type": "fix",
  "status": "pending",
  "depends_on": [],
  "context": {
    "files_to_read": [
      "path/to/affected/file.rs"
    ],
    "prior_review_issues": [
      "Bug: [description]",
      "Observed: [behavior]",
      "Expected: [behavior]"
    ]
  },
  "plan_file": ".cdm/plans/plan-X.task-1.md"
}
```

---

## Error Handling

If the wizard encounters issues:

### Missing Prerequisites
> I couldn't find `.cdm/tasks.json`. Please run `/cdm` first to initialize the project, then try `/fix` again.

### Invalid tasks.json
> The tasks.json file appears to be invalid. Please run `cdm --validate` to check for errors.

### Conflicting Task IDs
> Task ID "[id]" already exists. I'll use "[new-id]" instead.

### No Active Phase
> No phase is currently active. I'll add the fix to phase "[first-pending-phase]".

---

## Tips for Effective Bug Fixes

1. **Reproduce first** - Ensure the bug can be reliably reproduced
2. **Root cause** - Fix the cause, not just the symptom
3. **Minimal changes** - Make the smallest change that fixes the bug
4. **Add tests** - Every bug fix should include a test
5. **Check regressions** - Verify related functionality still works
6. **Document** - Note why the fix works in code comments

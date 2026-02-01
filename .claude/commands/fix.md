# Bug Fix Wizard

This command guides users through adding a bug fix task to an existing cm (Claude Code Manager) project. The wizard collects bug details through a conversational flow and then updates the project artifacts (tasks.json, optionally ROADMAP.md).

## Prerequisites

Requires: `.cm/` directory with `tasks.json`. If not present, run `/cm` first.

## CRITICAL: Scope Limitations

This command ONLY updates planning files:
- `.cm/tasks.json` - Add fix task definition
- `.cm/ROADMAP.md` - Optionally add checkbox entry

This command does NOT:
- Implement any code fixes
- Run `cm run` or execute tasks
- Make changes outside `.cm/` directory

After the wizard completes, the user must manually run `cm run` to start the fix.

## Important: Interactive Flow

You MUST follow this wizard flow step by step. Do NOT skip steps or modify files until you have gathered all the required information and received user confirmation.

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

## Step 2: Bug Classification

### Step 2.1: Severity

**Ask the user:**

> What is the **severity**?
>
> - **Critical**: System crash, data loss, security vulnerability
> - **High**: Major feature broken, no workaround
> - **Medium**: Feature impaired, workaround exists
> - **Low**: Minor issue, cosmetic problem

Wait for the user's response before proceeding.

### Step 2.2: Category

**Ask the user:**

> What **category** is this bug?
>
> - Logic error
> - Crash/panic
> - Performance issue
> - UI/UX problem
> - Data corruption
> - Security issue
> - Other: [specify]

Wait for the user's response before proceeding.

---

## Step 3: Investigation

**Analyze the codebase to locate the bug:**

1. Read relevant files mentioned by the user
2. Search for related code patterns
3. Identify the root cause location
4. Determine affected components

**Present your findings:**

> ## Investigation Results
>
> **Likely location:**
> - `path/to/file.rs` - [specific function or line range]
>
> **Root cause analysis:**
> [Brief explanation of what's causing the bug]
>
> **Affected components:**
> - [Component 1]
> - [Component 2]
>
> **Related files to review:**
> - `file1.rs` - [why relevant]
> - `file2.rs` - [why relevant]
>
> Does this match your understanding? Any additional context?

Wait for the user's response before proceeding.

---

## Step 4: Fix Approach

**Propose a fix approach:**

> ## Proposed Fix
>
> **Approach:**
> [Description of how to fix the bug]
>
> **Changes required:**
> 1. [File 1]: [Change description]
> 2. [File 2]: [Change description]
>
> **Testing strategy:**
> - [ ] Unit test for the specific bug case
> - [ ] Regression tests for related functionality
> - [ ] Manual verification steps
>
> **Risk assessment:**
> - Impact scope: [Low/Medium/High]
> - Regression risk: [Low/Medium/High]
>
> Does this approach look correct? Any concerns?

Wait for the user's response before proceeding.

---

## Step 5: Generate Phase Plan

**If creating a new phase for the fix, generate a detailed phase plan following the PLAN.md template.**

Read the `.cm/agents/PLAN.md` template for the required format. For bug fixes, the plan should include:

1. **Objective** - What the fix accomplishes
2. **Background** - Bug description, impact, current state
3. **Root Cause Analysis** - Why the bug occurs
4. **Fix Strategy** - How the fix works
5. **Implementation Steps** - Numbered list with file paths
6. **Files to Modify** - Each file with specific changes
7. **Success Criteria** - Bug no longer reproduces, tests pass
8. **Regression Prevention** - Tests or guards to prevent recurrence
9. **Verification** - Commands to verify the fix

**Present the generated plan to the user:**

> Here's the detailed fix plan for this phase:
>
> [Generated plan content following PLAN.md template]
>
> Does this plan look correct? Would you like any modifications?

Wait for the user's response before proceeding. (Skip this step if adding to an existing phase.)

---

## Step 6: Task Placement

**Analyze existing tasks and ask:**

> Where should this fix be placed in the task queue?
>
> **Options:**
> 1. **Immediate**: Add as next pending task (high priority)
> 2. **Current phase**: Add to end of current phase
> 3. **Specific phase**: Add to [phase-name]
> 4. **Deferred**: Add to backlog for later
>
> **Current phase:** [phase-name] ([N] pending tasks)
>
> Which priority level?

Wait for the user's response before proceeding.

---

## Step 7: Confirmation

**Present the complete fix task:**

> ## Fix Task Summary
>
> **Bug:** [summary]
> **Severity:** [severity]
> **Task ID:** [generated-task-id]
>
> **Plan file to create:** `.cm/plans/plan-[task-id].md`
> (containing detailed fix instructions)
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
>   "plan_file": ".cm/plans/plan-[task-id].md"
> }
> ```
>
> **Placement:** [phase and position]
>
> Does this look correct? Reply "yes" to add the fix task, or provide corrections.

Wait for explicit user confirmation before modifying files.

---

## Step 8: Handoff to /end

After confirmation:

> Ready to save. Run `/end` to write changes to tasks.json and roadmap.json.
> Then run `cm` when ready to implement.

Do NOT modify files. Wait for `/end`.

---

## Phase Template (for new fix phases)

When creating a new phase for a fix, use this JSON template. The `plan` field contains the detailed fix plan generated in Step 5.

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

**IMPORTANT:** For each fix phase, create a plan file at `.cm/plans/plan-X.md` (where X is the phase number) containing the detailed fix instructions. All tasks in the phase reference this plan file via the `plan_file` field.

Create plan file at `.cm/plans/plan-X.md`:
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
  "plan_file": ".cm/plans/plan-X.md"
}
```

---

## Error Handling

If the wizard encounters issues:

### Missing Prerequisites
> I couldn't find `.cm/tasks.json`. Please run `/cm` first to initialize the project, then try `/fix` again.

### Invalid tasks.json
> The tasks.json file appears to be invalid. Please run `cm --validate` to check for errors.

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

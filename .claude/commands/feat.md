# Feature Wizard

This command guides users through adding a new feature to an existing cm (Claude Code Manager) project. The wizard collects feature requirements through a conversational flow and then updates the project artifacts (PLAN.md, ROADMAP.md, tasks.json).

## Prerequisites

Requires: `.cm/` directory with `tasks.json`, `PLAN.md`, and `ROADMAP.md`. If not present, run `/cm` first.

## CRITICAL: Scope Limitations

This command ONLY updates planning files:
- `.cm/PLAN.md` - Add feature documentation
- `.cm/roadmap.json` - Add roadmap items
- `.cm/tasks.json` - Add task definitions

This command does NOT:
- Implement any code
- Run `cm run` or execute tasks
- Make changes outside `.cm/` directory

After the wizard completes, the user must manually run `cm run` to start
implementation.

## Important: Interactive Flow

You MUST follow this wizard flow step by step. Do NOT skip steps or modify files
until you have gathered all the required information and received user confirmation.

---

## Step 1: Feature Description

**Ask the user:**

> **Describe the feature you want to add.**
>
> Include:
> - What it does
> - Why it's needed
> - Any key requirements

Wait for the user's response before proceeding.

---

## Step 2: Requirements Gathering

### Step 2.1: Functional Requirements

**Ask the user:**

> What should this feature **do**?
>
> (List the functional requirements)

Wait for the user's response before proceeding.

### Step 2.2: Non-Functional Requirements

**Ask the user:**

> Any **non-functional requirements**?
>
> (Performance, security, accessibility - or "none")

Wait for the user's response before proceeding.

### Step 2.3: User Interface

**Ask the user:**

> Any **UI/UX considerations**?
>
> (Interface requirements - or "N/A")

Wait for the user's response before proceeding.

### Step 2.4: Data Requirements

**Ask the user:**

> What **data** does this feature need?
>
> (Data storage, formats, sources - or "none specific")

Wait for the user's response before proceeding.

---

## Step 3: Technical Analysis

**Analyze the existing codebase:**

1. Read the current `.cm/PLAN.md` to understand project architecture
2. Read the current `.cm/tasks.json` to understand task structure
3. Identify which modules/components will be affected
4. Determine if new modules need to be created

**Ask the user:**

> Based on the project structure, here's my technical analysis:
>
> **Affected components:**
> - [List components that will be modified]
>
> **New components needed:**
> - [List new files/modules to create]
>
> **Dependencies:**
> - [List any new dependencies required]
>
> Do you have any additional technical requirements or constraints?

Wait for the user's response before proceeding.

---

## Step 4: Task Breakdown

**Ask the user:**

> Let's break this feature into implementation tasks.
>
> I suggest the following breakdown:
>
> 1. **[Task 1]** - [Description, estimated complexity]
> 2. **[Task 2]** - [Description, estimated complexity]
> 3. **[Task 3]** - [Description, estimated complexity]
> ...
>
> Each task should be:
> - Completable in a single agent session
> - Independently testable
> - Have clear success criteria
>
> Would you like to modify this breakdown?

Wait for the user's response before proceeding.

---

## Step 5: Generate Phase Plan

**Using the information gathered, generate a detailed phase plan following the PLAN.md template.**

Read the `.cm/agents/PLAN.md` template for the required format. The plan must include:

1. **Objective** - What the phase accomplishes
2. **Background** - Why this change is needed, current state, consequences
3. **Implementation Steps** - Numbered list with file paths and line numbers where known
4. **Files to Modify** - Each file with specific changes
5. **Files to Create** - New files with their purpose
6. **Key Decisions** - Architectural choices and rationale
7. **Success Criteria** - Verifiable checkboxes
8. **Verification** - Commands to verify implementation

**Present the generated plan to the user:**

> Here's the detailed implementation plan for this phase:
>
> [Generated plan content following PLAN.md template]
>
> Does this plan look correct? Would you like any modifications?

Wait for the user's response before proceeding.

---

## Step 6: Integration Planning

### Step 6.1: Entry Points

**Ask the user:**

> Where will users **access** this feature?
>
> (UI locations, API endpoints, commands)

Wait for the user's response before proceeding.

### Step 6.2: Dependencies

**Ask the user:**

> Which existing **tasks must complete first**?
>
> (Dependencies from tasks.json - or "none")

Wait for the user's response before proceeding.

### Step 6.3: Testing

**Ask the user:**

> What **tests** are needed?
>
> (Unit tests, integration tests, manual testing)

Wait for the user's response before proceeding.

### Step 6.4: Documentation

**Ask the user:**

> What **documentation** updates are required?
>
> (README, API docs, user guides - or "none")

Wait for the user's response before proceeding.

---

## Step 7: Phase Placement

**Analyze existing phases in tasks.json and ask:**

> Where should this feature be placed in the project phases?
>
> **Current phases:**
> [List existing phases with their status]
>
> **Options:**
> 1. Add to existing phase: [phase-name]
> 2. Create new phase: [suggested-phase-name]
> 3. Create as a sub-phase after: [phase-name]
>
> Which option do you prefer?

Wait for the user's response before proceeding.

---

## Step 8: Confirmation

**Present a complete summary:**

> ## Feature Summary
>
> **Feature:** [name]
> **Description:** [description]
>
> **Tasks to add:**
> 1. [Task 1] - [type: implement/test/review]
> 2. [Task 2] - [type: implement/test/review]
> 3. [Task 3] - [type: implement/test/review]
>
> **Phase placement:** [phase info]
>
> **Dependencies:**
> - [dependency 1]
> - [dependency 2]
>
> **Files to update:**
> - `.cm/PLAN.md` - Add feature documentation
> - `.cm/ROADMAP.md` - Add feature tasks with checkboxes
> - `.cm/tasks.json` - Add task definitions
>
> Does this look correct? Reply "yes" to update the files, or provide corrections.

Wait for explicit user confirmation before modifying files.

---

## Step 9: Handoff to /end

After confirmation:

> Ready to save. Run `/end` to write changes to tasks.json, roadmap.json, and PLAN.md.
> Then run `cm` when ready to implement.

Do NOT modify files. Wait for `/end`.

---

## Phase Template

When creating a new phase, use this JSON template. The `plan` field contains the detailed implementation plan generated in Step 5.

```json
{
  "id": "phase-X",
  "name": "[Feature Name]",
  "plan": "[Detailed plan generated following PLAN.md template - includes Objective, Background, Implementation Steps, Files to Modify, Files to Create, Key Decisions, Success Criteria, and Verification]",
  "status": "pending",
  "tasks": [
    // Tasks go here (see templates below)
  ]
}
```

---

## Task Templates

**IMPORTANT:** For each phase, create a plan file at `.cm/plans/plan-X.md` (where X is the phase number) containing the detailed instructions for all tasks in that phase. All tasks in the phase reference this same plan file via the `plan_file` field.

### Implementation Task Template

Create plan file at `.cm/plans/plan-X.md`:
```markdown
Implement [component] for [feature]:

1. Create [file path]
2. Implement [function/struct] that:
   - [Requirement 1]
   - [Requirement 2]
3. Add error handling for:
   - [Error case 1]
   - [Error case 2]
4. Ensure `cargo build` and `cargo clippy` pass
```

Task JSON:
```json
{
  "id": "phase-X.feat-[name].impl-[n]",
  "name": "Implement [component]",
  "type": "implement",
  "status": "pending",
  "depends_on": [],
  "context": {
    "files_to_read": [],
    "code_style_excerpt": null
  },
  "plan_file": ".cm/plans/plan-X.md"
}
```

### Test Task Template

Plan file at `.cm/plans/plan-X.md` (same file as implementation):
```markdown
Add tests for [component]:

1. Add unit tests in [file]:
   - Test [scenario 1]
   - Test [scenario 2]
   - Test error handling for [case]
2. Add integration tests if needed
3. Ensure `cargo test` passes with all new tests
```

Task JSON:
```json
{
  "id": "phase-X.feat-[name].test-[n]",
  "name": "Test [component]",
  "type": "test",
  "status": "pending",
  "depends_on": ["phase-X.feat-[name].impl-[n]"],
  "context": {
    "files_to_read": ["src/component.rs"]
  },
  "plan_file": ".cm/plans/plan-X.md"
}
```

### Review Task Template

Plan file at `.cm/plans/plan-X.md` (same file as implementation):
```markdown
Review [feature] implementation:

1. Check code quality and style consistency
2. Verify all requirements are met
3. Check error handling completeness
4. Review test coverage
5. Document any issues found for fixing
```

Task JSON:
```json
{
  "id": "phase-X.feat-[name].review",
  "name": "Review [feature] implementation",
  "type": "review",
  "status": "pending",
  "depends_on": ["phase-X.feat-[name].impl-1", "phase-X.feat-[name].test-1"],
  "context": {
    "files_to_read": ["src/feature/"]
  },
  "plan_file": ".cm/plans/plan-X.md"
}
```

---

## Tips for Good Feature Tasks

1. **Atomic tasks** - Each task should do one thing well
2. **Clear dependencies** - Ensure proper task ordering
3. **Testable outcomes** - Each task should have verifiable results
4. **Context matters** - Include relevant files in `files_to_read`
5. **Detailed instructions** - Be specific about what to implement
6. **Error handling** - Always include error cases in requirements
7. **Build verification** - End tasks with build/test verification

---

## Error Handling

If the wizard encounters issues:

### Missing Prerequisites
> I couldn't find `.cm/tasks.json`. Please run `/cm` first to initialize the
project, then try `/feat` again.

### Invalid tasks.json
> The tasks.json file appears to be invalid. Please run `cm --validate` to check for
errors.

### Conflicting Task IDs
> Task ID "[id]" already exists. I'll use "[new-id]" instead.

### Phase Not Found
> Phase "[phase-id]" not found. Available phases: [list phases]. Which phase should
I use?

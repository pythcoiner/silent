# Split Wizard

This skill refines an existing cdm project plan by splitting high-level phases into detailed sub-phases.

## Prerequisites

Before using this wizard, ensure:
- A `.cdm/` directory exists with `PLAN.md`
- `PLAN.md` was created by `/cdm` and contains a "## Phases" section
- `tasks.json` should NOT exist yet (this is a pre-generation step)

If prerequisites are not met, inform the user and suggest running `/cdm` first.

## CRITICAL: Scope Limitations

This skill ONLY updates:
- `.cdm/PLAN.md` - Refine phases into detailed sub-phases

This skill does NOT:
- Generate tasks.json or roadmap.json (that comes later with `/end`)
- Implement any code
- Run `cdm run` or execute tasks

**When to use:** After `/cdm` creates PLAN.md but before running `/end` to generate JSON files.

---

## Step 1: Load Current Plan

**Read the current PLAN.md:**

1. Read `.cdm/PLAN.md` to understand existing phases
2. Parse the "## Phases" section

**Present summary to user:**

> **Current High-Level Plan:**
>
> **Phases from PLAN.md:**
> 1. Phase 1: [name] - [brief description]
> 2. Phase 2: [name] - [brief description]
> ...
>
> Would you like me to analyze and split these into more detailed phases?

Wait for user confirmation.

---

## Step 2: Analyze Phases

For each phase, evaluate:
- Current scope and complexity
- Number and size of tasks
- Dependencies between tasks
- **Estimated changeset size in LoC** (rough order-of-magnitude — used to drive split recommendations against the sizing rules below)
- Whether it can be meaningfully split

**Present analysis:**

> **Phase Analysis:**
>
> **Phase 1: [phase-name]**
> - Current tasks: [N]
> - Estimated LoC: ~[X] (rough)
> - Complexity: [low/medium/high]
> - Recommendation: [Keep as-is / Split into N sub-phases]
> - Suggested split: [Brief description of how to split]
>
> [Repeat for each phase]
>
> Do you want me to proceed with splitting? You can also specify which phases to split.

Wait for user input.

---

## Step 3: Generate Detailed Phases

For each phase being split:

1. **Identify logical sub-divisions:**
   - By component/module
   - By feature area
   - By dependency order
   - By complexity layer (foundation → advanced)

2. **Create new phase definitions:**
   - New phase IDs: `phase-N.1`, `phase-N.2`, etc. (or new sequential numbers)
   - Clear, specific names
   - Detailed task breakdown
   - Proper dependencies

3. **Preserve existing work:**
   - Keep completed phases intact
   - Maintain task history
   - Update dependencies correctly

**Present proposed split:**

> **Proposed Phase Split:**
>
> **Original:** Phase 1 "Core Implementation" (5 tasks)
>
> **Split into:**
> - Phase 1.1 "Data Models" (2 tasks)
>   - task-1: Define structs
>   - task-2: Add serialization
>
> - Phase 1.2 "Business Logic" (2 tasks)
>   - task-3: Core algorithms
>   - task-4: Error handling
>
> - Phase 1.3 "Integration" (1 task)
>   - task-5: Wire components together
>
> Does this look correct? Reply "yes" to update files, or provide corrections.

Wait for user confirmation.

---

## Step 4: Update PLAN.md

After confirmation, update `.cdm/PLAN.md`:

### 4.1 Replace Phases Section

Replace the original "## Phases" section with detailed sub-phases:

```markdown
## Phases

### Phase 1: Data Models

**Goal:** Define core data structures

**Tasks:**
- Define User struct with fields
- Define Project struct with relationships
- Add serde serialization

**Deliverables:**
- Complete data model definitions
- Serialization tests passing

### Phase 2: Business Logic

**Goal:** Implement core algorithms

**Tasks:**
- Validation functions
- Error handling
- State transitions

**Deliverables:**
- Working business logic
- Unit tests for all functions

### Phase 3: Integration

**Goal:** Wire components together

**Tasks:**
- Connect data layer to logic
- Add API endpoints

**Deliverables:**
- Integrated system
- Integration tests passing
```

### 4.2 Preserve Other Sections

Keep all other PLAN.md sections intact:
- Overview, Goals, Success Criteria
- Architecture, Modules
- Technical Decisions, Out of Scope

### 4.3 No JSON Generation Yet

**Important:** Do NOT generate tasks.json or roadmap.json at this step.
Tell the user the next step is to run `/end` to generate JSON files.

---

## Step 5: Completion Summary

> **Split completed successfully!**
>
> **Changes to PLAN.md:**
> - Original phases: [N]
> - New detailed phases: [M]
> - Total tasks outlined: [T]
>
> **New phase structure:**
> 1. Phase 1: [name]
> 2. Phase 2: [name]
> ...
>
> **Next steps:**
> 1. Review `.cdm/PLAN.md` for the detailed structure
> 2. Run `/end` to generate tasks.json and roadmap.json from the refined plan
> 3. Or make further refinements to PLAN.md if needed

---

## Phase Split Guidelines

### Phase sizing rules (apply when proposing or evaluating a split)

- Each phase is a **minimal meaningful changeset** — one reviewable commit, not a sprint.
- Follow the **"show me the work"** philosophy: every phase produces a visible, demoable artifact a reviewer can run or inspect.
- **80–300 LoC** is a guideline, not a hard cap. Going over is fine when the changeset is genuinely cohesive and would lose meaning if split. Past ~300 LoC it just gets harder for a reviewer to hold in context, so prefer splitting unless splitting hurts the work.
- Phases must be independently reviewable — do not bundle unrelated concerns into one phase to "save a round-trip".
- **Default to one task per resulting child phase.** When you split, each new phase should contain exactly one task unless the user explicitly asks for a phase to keep multiple tasks. A phase that "needs" multiple tasks is usually a signal that the phase itself should be split further.

### When to Split a Phase

Split when:
- Estimated changeset clearly exceeds ~300 LoC of cohesive change, and a reviewer would struggle to hold it in context
- Phase bundles unrelated concerns that could be reviewed independently
- Tasks span multiple components or layers that could ship separately
- Tasks have complex interdependencies that obscure what's being demonstrated
- Phase scope is too broad to track easily

Conversely, do **not** split when the work is genuinely one cohesive change — splitting an integral changeset into artificial slices makes review harder, not easier.

### How to Split

1. **By Component:** Group tasks that touch the same module/file
2. **By Layer:** Foundation tasks first, then features, then polish
3. **By Feature:** Separate distinct feature areas
4. **By Dependency:** Independent tasks in parallel phases

### Naming Conventions

- Use descriptive names: "User Authentication" not "Phase 2"
- Keep names concise: 3-5 words max
- Indicate scope: "API Endpoints" vs "Full Backend"

### ID Conventions

Option A - Sub-numbering:
- `phase-1` → `phase-1.1`, `phase-1.2`, `phase-1.3`

Option B - Sequential:
- Insert new phases with new numbers
- Renumber subsequent phases if needed

Choose the approach that best fits the project structure.

---

## Error Handling

### No PLAN.md found

> I couldn't find `.cdm/PLAN.md`. Please run `/cdm` first to create the initial plan.

### tasks.json already exists

> Warning: `tasks.json` already exists. `/split` is meant to be run before JSON generation.
>
> Options:
> 1. Delete tasks.json and roadmap.json, then run /split
> 2. Use `/feat` to add features to the existing plan instead
>
> Which option do you prefer?

### No phases to split

> All phases in PLAN.md are already well-detailed. No further splitting recommended.

### PLAN.md has no Phases section

> PLAN.md doesn't have a "## Phases" section. Please ensure /cdm completed properly.

Continue working on this cm-managed project.

## Context Files

Read these files to understand the current state:
- `.cm/PLAN.md` - Project plan and architecture
- `.cm/ROADMAP.md` - Progress checklist
- `.cm/TASKS.md` - Task status overview

## How cm Works

The `cm` CLI tool orchestrates task execution:
1. Reads `tasks.json` for the next pending task
2. Spawns an IMPLEM agent with isolated context
3. Runs build verification (`cargo build` + `cargo clippy`)
4. Spawns a REVIEW agent to check the implementation
5. If issues found: spawns FIX agent, then re-reviews (max 5 cycles)
6. Marks task complete and updates JSON files
7. Regenerates markdown files from JSON

## Your Role

You are continuing manual work on this project. Check ROADMAP.md for uncompleted items, or run `cm --status` to see current progress.

To run automated implementation: `cm` or `cm --step` (one task at a time).

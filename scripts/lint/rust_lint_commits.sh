#!/usr/bin/env bash
set -euo pipefail

range="${1:-}"

if [[ -z "$range" ]]; then
    if [[ "${GITHUB_EVENT_NAME:-}" == "pull_request" ]]; then
        range="origin/${GITHUB_BASE_REF}...${GITHUB_SHA}"
    elif [[ -n "${GITHUB_SHA:-}" && -n "${GITHUB_EVENT_BEFORE:-}" && "${GITHUB_EVENT_BEFORE}" != "0000000000000000000000000000000000000000" ]]; then
        range="${GITHUB_EVENT_BEFORE}...${GITHUB_SHA}"
    else
        range="HEAD~1...HEAD"
    fi
fi

mapfile -t commits < <(git rev-list --reverse --no-merges "$range")
if [[ ${#commits[@]} -eq 0 ]]; then
    echo "No commits in range: $range"
    exit 0
fi

orig_ref="$(git rev-parse --abbrev-ref HEAD)"
orig_sha="$(git rev-parse HEAD)"
repo_root="$(git rev-parse --show-toplevel)"
worktree_root="$repo_root"
temp_worktree=""
dirty=0

if ! git diff --quiet || ! git diff --cached --quiet || [[ -n "$(git ls-files --others --exclude-standard)" ]]; then
    dirty=1
    temp_worktree="$(mktemp -d /tmp/opencode/rust-lint-worktree.XXXXXX)"
    git worktree add --detach "$temp_worktree" "$orig_sha" >/dev/null
    worktree_root="$temp_worktree"
fi

restore_ref() {
    if [[ -n "$temp_worktree" ]]; then
        git worktree remove --force "$temp_worktree" >/dev/null 2>&1 || true
    fi
}
trap restore_ref EXIT

for commit in "${commits[@]}"; do
    if ! parent=$(git -C "$worktree_root" rev-parse "${commit}^" 2>/dev/null); then
        continue
    fi

    if ! git -C "$worktree_root" diff --quiet "$parent" "$commit" -- ':(glob)silent/**/*.rs' 'silent/Cargo.toml' 'silent/Cargo.lock'; then
        echo "lint rust commit ${commit}"
        git -C "$worktree_root" checkout --detach "$commit" >/dev/null
        cargo fmt --manifest-path "$worktree_root/silent/Cargo.toml" -- --check
        cargo clippy --manifest-path "$worktree_root/silent/Cargo.toml" -- -D warnings
    fi
done

if [[ $dirty -eq 1 ]]; then
    echo "lint rust working tree (uncommitted changes)"
    cargo fmt --manifest-path "$repo_root/silent/Cargo.toml" -- --check
    cargo clippy --manifest-path "$repo_root/silent/Cargo.toml" -- -D warnings
fi

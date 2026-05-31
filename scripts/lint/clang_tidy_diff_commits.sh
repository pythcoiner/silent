#!/usr/bin/env bash
set -euo pipefail

tidy_bin="clang-tidy"

now_ms() {
    date +%s%3N
}

run_tidy_files() {
    local label="$1"
    shift
    local files=("$@")
    if [[ ${#files[@]} -eq 0 ]]; then
        return 0
    fi

    local start end elapsed rc
    local seconds millis
    start=$(now_ms)
    set +e
    printf '%s\0' "${files[@]}" | xargs -0 -P"$(nproc)" -n1 "$tidy_bin" -p build --quiet --warnings-as-errors='*' --header-filter='^src/.*' --exclude-header-filter='^src/resources/.*' 2>&1 | { rg -v '^[0-9]+ warnings generated\.$' || true; }
    rc=${PIPESTATUS[1]}
    set -e
    end=$(now_ms)
    elapsed=$((end - start))
    seconds=$((elapsed / 1000))
    millis=$((elapsed % 1000))
    printf "Lint %s (%s files) in %d.%03dS\n" "$label" "${#files[@]}" "$seconds" "$millis"
    return "$rc"
}

tidy_diff_cmd=""
if command -v clang-tidy-diff.py >/dev/null 2>&1; then
    tidy_diff_cmd="clang-tidy-diff.py"
elif command -v clang-tidy-diff >/dev/null 2>&1; then
    tidy_diff_cmd="clang-tidy-diff"
fi

range="${1:-}"
check_worktree="${2:-}"
worktree_only="${3:-}"

if [[ -z "$range" ]]; then
    if [[ "${GITHUB_EVENT_NAME:-}" == "pull_request" ]]; then
        range="origin/${GITHUB_BASE_REF}...${GITHUB_SHA}"
    elif [[ -n "${GITHUB_SHA:-}" && -n "${GITHUB_EVENT_BEFORE:-}" && "${GITHUB_EVENT_BEFORE}" != "0000000000000000000000000000000000000000" ]]; then
        range="${GITHUB_EVENT_BEFORE}...${GITHUB_SHA}"
    else
        range="HEAD~1...HEAD"
    fi
fi

mapfile -t commits < <(git rev-list --reverse "$range")

if [[ ${#commits[@]} -eq 0 ]]; then
    echo "No commits in range: $range"
    exit 0
fi

if [[ "$worktree_only" != "--worktree-only" ]]; then
for commit in "${commits[@]}"; do
    if ! parent=$(git rev-parse "${commit}^" 2>/dev/null); then
        continue
    fi

    echo "lint commit ${commit}"
    diff_output=$(git diff -U0 "$parent" "$commit" -- src || true)
    if [[ -z "$diff_output" ]]; then
        continue
    fi

    mapfile -t changed_files < <(git diff --name-only "$parent" "$commit" -- ':(glob)src/**/*.cpp' ':(glob)src/**/*.h')
    if [[ ${#changed_files[@]} -eq 0 ]]; then
        continue
    fi
    run_tidy_files "commit:${commit}" "${changed_files[@]}"
done
fi

if [[ "$check_worktree" == "--worktree" ]]; then
    echo "lint working tree (HEAD vs index+worktree)"
    diff_output=$(git diff -U0 HEAD -- src || true)
    if [[ -z "$diff_output" ]]; then
        exit 0
    fi

    mapfile -t changed_files < <(git diff --name-only HEAD -- ':(glob)src/**/*.cpp' ':(glob)src/**/*.h')
    if [[ ${#changed_files[@]} -eq 0 ]]; then
        exit 0
    fi
    run_tidy_files "worktree" "${changed_files[@]}"
fi

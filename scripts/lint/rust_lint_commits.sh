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

mapfile -t commits < <(git rev-list --reverse "$range")
if [[ ${#commits[@]} -eq 0 ]]; then
    echo "No commits in range: $range"
    exit 0
fi

orig_ref="$(git rev-parse --abbrev-ref HEAD)"
orig_sha="$(git rev-parse HEAD)"

restore_ref() {
    if [[ "$orig_ref" == "HEAD" ]]; then
        git checkout --detach "$orig_sha" >/dev/null 2>&1 || true
    else
        git checkout "$orig_ref" >/dev/null 2>&1 || true
    fi
}
trap restore_ref EXIT

for commit in "${commits[@]}"; do
    if ! parent=$(git rev-parse "${commit}^" 2>/dev/null); then
        continue
    fi

    if ! git diff --quiet "$parent" "$commit" -- ':(glob)silent/**/*.rs' 'silent/Cargo.toml' 'silent/Cargo.lock'; then
        echo "lint rust commit ${commit}"
        git checkout --detach "$commit" >/dev/null
        cargo fmt --manifest-path silent/Cargo.toml -- --check
        cargo clippy --manifest-path silent/Cargo.toml -- -D warnings
    fi
done

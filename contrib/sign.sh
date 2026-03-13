#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

STAGING_DIR="$REPO_ROOT/release-artifacts"

die() { echo "ERROR: $1" >&2; exit 1; }

# --- Determine tag ---

if [[ $# -ge 1 ]]; then
    tag="$1"
else
    tag="$(git describe --tags --abbrev=0 2>/dev/null)" || die "No tags found. Pass tag as argument."
fi

echo "=== Silent Sign Script ==="
echo "Tag: $tag"
echo

# --- Preconditions ---

command -v gh >/dev/null || die "'gh' CLI not found."
command -v gpg >/dev/null || die "'gpg' not found."
gh auth status >/dev/null 2>&1 || die "'gh' not authenticated."

# Check local artifacts exist
[[ -d "$STAGING_DIR" ]] || die "Staging directory not found: $STAGING_DIR"

local_files=(
    "silent-${tag}-linux-x86_64"
    "silent-${tag}-windows-x86_64.exe"
    "silent-${tag}-macos-aarch64"
    "silent-${tag}-macos-x86_64"
)

for f in "${local_files[@]}"; do
    [[ -f "$STAGING_DIR/$f" ]] || die "Local artifact missing: $STAGING_DIR/$f"
done

# --- Download remote SHA256SUMS ---

echo "Downloading SHA256SUMS from GitHub release..."
tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

gh release download "$tag" --pattern "SHA256SUMS" --dir "$tmpdir" || \
    die "Failed to download SHA256SUMS from release $tag."

# --- Verify hashes ---

echo "Verifying local artifacts against GitHub release..."
echo

mismatch=false
while IFS='  ' read -r remote_hash remote_file; do
    [[ -z "$remote_hash" ]] && continue
    local_path="$STAGING_DIR/$remote_file"
    if [[ ! -f "$local_path" ]]; then
        echo "  MISSING: $remote_file"
        mismatch=true
        continue
    fi
    local_hash="$(sha256sum "$local_path" | cut -d' ' -f1)"
    if [[ "$local_hash" == "$remote_hash" ]]; then
        echo "  OK: $remote_file"
    else
        echo "  MISMATCH: $remote_file"
        echo "    local:  $local_hash"
        echo "    remote: $remote_hash"
        mismatch=true
    fi
done < "$tmpdir/SHA256SUMS"

echo

if [[ "$mismatch" == true ]]; then
    die "Hash verification failed. Local build does not match GitHub release."
fi

echo "All hashes match."
echo

# --- Sign ---

echo "Signing SHA256SUMS with GPG..."
cp "$tmpdir/SHA256SUMS" "$STAGING_DIR/SHA256SUMS"
gpg --detach-sign --armor --output "$STAGING_DIR/SHA256SUMS.asc" "$STAGING_DIR/SHA256SUMS"

echo "Uploading signature to GitHub release..."
gh release upload "$tag" "$STAGING_DIR/SHA256SUMS.asc" --clobber

echo
echo "=== Release $tag signed successfully ==="

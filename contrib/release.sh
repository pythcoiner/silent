#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$REPO_ROOT"

NIX="${NIX:-nix}"
NIX_FLAGS="--extra-experimental-features 'nix-command flakes'"
STAGING_DIR="$REPO_ROOT/release-artifacts"

# --- Helpers ---

die() { echo "ERROR: $1" >&2; exit 1; }

confirm() {
    local prompt="$1"
    read -rp "$prompt [y/N] " answer
    [[ "$answer" =~ ^[Yy]$ ]] || die "Aborted."
}

select_option() {
    local prompt="$1"
    shift
    local options=("$@")
    echo "$prompt"
    echo
    for i in "${!options[@]}"; do
        echo "  $((i + 1))) ${options[$i]}"
    done
    echo
    local choice
    read -rp "Select [1-${#options[@]}]: " choice
    if [[ ! "$choice" =~ ^[0-9]+$ ]] || (( choice < 1 || choice > ${#options[@]} )); then
        die "Invalid selection."
    fi
    SELECTED="$((choice - 1))"
}

read_version() {
    grep '^version' silent/Cargo.toml | head -1 | sed 's/.*"\(.*\)".*/\1/'
}

bump_patch() {
    local v="$1"
    local major minor patch
    IFS='.' read -r major minor patch <<< "$v"
    echo "$major.$minor.$((patch + 1))"
}

bump_minor() {
    local v="$1"
    local major minor patch
    IFS='.' read -r major minor patch <<< "$v"
    echo "$major.$((minor + 1)).0"
}

bump_major() {
    local v="$1"
    local major minor patch
    IFS='.' read -r major minor patch <<< "$v"
    echo "$((major + 1)).0.0"
}

update_version() {
    local new_version="$1"

    # silent/Cargo.toml
    sed -i "0,/^version = \".*\"/s//version = \"$new_version\"/" silent/Cargo.toml

    # CMakeLists.txt
    sed -i "s/project(\${BIN} VERSION .* LANGUAGES/project(\${BIN} VERSION $new_version LANGUAGES/" CMakeLists.txt

    # flake.nix (all 3 occurrences)
    sed -i "s/version = \"[0-9]*\.[0-9]*\.[0-9]*\";/version = \"$new_version\";/g" flake.nix
}

# --- Mode Selection ---

command -v $NIX >/dev/null || die "'nix' not found."

echo "=== Silent Release Script ==="
echo

select_option "What do you want to do?" \
    "build only (verify reproducibility)" \
    "release"
MODE="$SELECTED"

if [[ "$MODE" == "0" ]]; then
    current_version="$(read_version)"
    last_tag="$(git describe --tags --abbrev=0 2>/dev/null || echo "v${current_version}")"
    tag="$last_tag"
    echo
    echo "Building artifacts for tag: $tag"
else
    # --- Preconditions ---

    # Clean git tree
    [[ -z "$(git status --porcelain)" ]] || die "Working tree is dirty. Commit or stash changes first."

    # On master
    branch="$(git branch --show-current)"
    [[ "$branch" == "master" ]] || die "Must be on master branch (currently on '$branch')."

    has_gh=true
    if ! command -v gh >/dev/null 2>&1; then
        echo "WARNING: 'gh' CLI not found. GitHub release will be skipped."
        has_gh=false
    elif ! gh auth status >/dev/null 2>&1; then
        echo "WARNING: 'gh' not authenticated. GitHub release will be skipped."
        has_gh=false
    fi

# --- Version Selection ---

current_version="$(read_version)"
last_tag="$(git describe --tags --abbrev=0 2>/dev/null || echo "")"

echo "Current version: $current_version"
[[ -n "$last_tag" ]] && echo "Last tag: $last_tag"
echo

if [[ "$last_tag" == *"-rc"* ]]; then
    # Previous release was an rc — offer to bump rc or finalize
    # Extract base version from rc tag: v0.2.0-rc1 -> 0.2.0
    base_version="${last_tag#v}"
    base_version="${base_version%%-rc*}"
    # Extract current rc number
    prev_rc="${last_tag##*-rc}"

    select_option "Previous release was $last_tag:" \
        "new rc (v${base_version}-rc$((prev_rc + 1)))" \
        "release (v${base_version})"

    new_version="$base_version"
    if [[ "$SELECTED" == "0" ]]; then
        tag="v${base_version}-rc$((prev_rc + 1))"
        is_rc=true
    else
        tag="v${base_version}"
        is_rc=false
    fi
else
    # No rc in progress — ask for version bump type
    select_option "Version type:" \
        "patch (${current_version} -> $(bump_patch "$current_version"))" \
        "minor (${current_version} -> $(bump_minor "$current_version"))" \
        "major (${current_version} -> $(bump_major "$current_version"))"

    if [[ "$SELECTED" == "0" ]]; then
        new_version="$(bump_patch "$current_version")"
    elif [[ "$SELECTED" == "1" ]]; then
        new_version="$(bump_minor "$current_version")"
    else
        new_version="$(bump_major "$current_version")"
    fi

    echo
    echo "New version: $new_version"
    echo

    select_option "Release type:" \
        "release candidate (v${new_version}-rc1)" \
        "release (v${new_version})"

    if [[ "$SELECTED" == "0" ]]; then
        tag="v${new_version}-rc1"
        is_rc=true
    else
        tag="v${new_version}"
        is_rc=false
    fi
fi

# --- Summary ---

echo
echo "=== Release Summary ==="
echo "  Current version: $current_version"
echo "  New version:     $new_version"
echo "  Tag:             $tag"
echo
confirm "Proceed?"

# --- Version Bump (non-rc only) ---

if [[ "$is_rc" == false ]]; then
    echo
    echo "Updating version to $new_version..."
    update_version "$new_version"
    git add silent/Cargo.toml CMakeLists.txt flake.nix
    git commit -m "release: $tag"
fi

# --- Tag & Push ---

echo "Creating tag $tag..."
git tag -a "$tag" -m "$tag"

echo "Pushing to origin..."
if ! git push origin master 2>&1; then
    echo "WARNING: Failed to push to origin. Push manually later."
fi
if ! git push origin "$tag" 2>&1; then
    echo "WARNING: Failed to push tag. Push manually with: git push origin $tag"
fi

fi # end of release section

# --- Build ---

echo
echo "Building all platforms..."
rm -rf "$STAGING_DIR"
mkdir -p "$STAGING_DIR"

echo "  Building Linux..."
eval "$NIX $NIX_FLAGS build .#linux"
cp result/bin/silent "$STAGING_DIR/silent-${tag}-linux-x86_64"

echo "  Building Windows..."
eval "$NIX $NIX_FLAGS build .#windows"
cp result/bin/silent.exe "$STAGING_DIR/silent-${tag}-windows-x86_64.exe"

echo "  Building macOS ARM..."
eval "$NIX $NIX_FLAGS build .#aarch64-apple-darwin"
cp result/bin/silent "$STAGING_DIR/silent-${tag}-macos-aarch64"

echo "  Building macOS x86_64..."
eval "$NIX $NIX_FLAGS build .#x86_64-apple-darwin"
cp result/bin/silent "$STAGING_DIR/silent-${tag}-macos-x86_64"

# --- Checksums ---

echo "Generating checksums..."
cd "$STAGING_DIR"
sha256sum silent-${tag}-* > SHA256SUMS
cd "$REPO_ROOT"

# --- GitHub Release ---

if [[ "$MODE" == "0" ]]; then
    echo
    echo "=== Build complete ==="
    echo "Artifacts in: $STAGING_DIR"
    cat "$STAGING_DIR/SHA256SUMS"
    exit 0
fi

if [[ "$has_gh" == true ]]; then
    echo "Creating GitHub release..."
    prerelease_flag=""
    if [[ "$is_rc" == true ]]; then
        prerelease_flag="--prerelease"
    fi

    if gh release create "$tag" \
        --title "$tag" \
        $prerelease_flag \
        "$STAGING_DIR"/silent-${tag}-* \
        "$STAGING_DIR/SHA256SUMS"; then
        echo
        echo "=== Release $tag created successfully ==="
    else
        echo
        echo "WARNING: GitHub release creation failed."
        echo "Create it manually with:"
        echo "  gh release create $tag $prerelease_flag --title $tag $STAGING_DIR/silent-${tag}-* $STAGING_DIR/SHA256SUMS"
    fi
else
    echo
    echo "Skipping GitHub release (gh CLI not available)."
    echo "Create it manually with:"
    echo "  gh release create $tag --title $tag $STAGING_DIR/silent-${tag}-* $STAGING_DIR/SHA256SUMS"
fi

echo
echo "Artifacts in: $STAGING_DIR"
echo "Run 'contrib/sign.sh $tag' to verify and sign."

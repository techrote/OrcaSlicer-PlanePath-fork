#!/usr/bin/env bash
set -euo pipefail

UPSTREAM_URL="https://github.com/OrcaSlicer/OrcaSlicer.git"
UPSTREAM_REMOTE="${ORCA_UPSTREAM_REMOTE:-upstream}"
UPSTREAM_REF="${1:-main}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "error: run this script from a PlanePath git checkout" >&2
  exit 2
fi

if git remote get-url "$UPSTREAM_REMOTE" >/dev/null 2>&1; then
  actual="$(git remote get-url "$UPSTREAM_REMOTE")"
  if [ "$actual" != "$UPSTREAM_URL" ]; then
    echo "error: remote '$UPSTREAM_REMOTE' points to '$actual', expected '$UPSTREAM_URL'" >&2
    exit 2
  fi
else
  git remote add "$UPSTREAM_REMOTE" "$UPSTREAM_URL"
fi

git fetch --prune "$UPSTREAM_REMOTE" "$UPSTREAM_REF"

sha="$(git rev-parse "$UPSTREAM_REMOTE/$UPSTREAM_REF")"
echo "Fetched $UPSTREAM_REMOTE/$UPSTREAM_REF at $sha"
echo
echo "Create a dedicated synchronization branch, merge this commit with --no-ff,"
echo "run required CI, and submit the result through a PR. This helper does not"
echo "modify the current branch automatically."

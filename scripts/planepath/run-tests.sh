#!/usr/bin/env bash
# Focused PlanePath regression gate.
# Uses the libslic3r_tests binary already produced by Orca's normal build jobs.
set -euo pipefail

TEST_DIR="${1:-build/tests}"
CONFIG="${2:-Release}"

candidates=(
  "${TEST_DIR}/libslic3r/${CONFIG}/libslic3r_tests"
  "${TEST_DIR}/libslic3r/${CONFIG}/libslic3r_tests.exe"
  "${TEST_DIR}/libslic3r/libslic3r_tests"
  "${TEST_DIR}/libslic3r/libslic3r_tests.exe"
)

exe=""
for candidate in "${candidates[@]}"; do
  if [[ -f "${candidate}" ]]; then
    exe="${candidate}"
    break
  fi
done

if [[ -z "${exe}" ]]; then
  echo "error: libslic3r_tests not found under '${TEST_DIR}' (config '${CONFIG}')" >&2
  printf 'checked: %s\n' "${candidates[@]}" >&2
  exit 2
fi

echo "PlanePath focused gate: ${exe} [FillPlanePath]"
"${exe}" '[FillPlanePath]' --order decl

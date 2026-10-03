#!/usr/bin/env bash
# Focused PlanePath regression gate.
# Uses test binaries already produced by Orca's normal build jobs.
set -euo pipefail

TEST_DIR="${1:-build/tests}"
CONFIG="${2:-Release}"

find_test_exe() {
  local suite="$1"
  local candidates=(
    "${TEST_DIR}/${suite}/${CONFIG}/${suite}_tests"
    "${TEST_DIR}/${suite}/${CONFIG}/${suite}_tests.exe"
    "${TEST_DIR}/${suite}/${suite}_tests"
    "${TEST_DIR}/${suite}/${suite}_tests.exe"
  )

  local candidate
  for candidate in "${candidates[@]}"; do
    if [[ -f "${candidate}" ]]; then
      printf '%s\n' "${candidate}"
      return 0
    fi
  done

  echo "error: ${suite}_tests not found under '${TEST_DIR}' (config '${CONFIG}')" >&2
  printf 'checked: %s\n' "${candidates[@]}" >&2
  return 2
}

libslic3r_exe="$(find_test_exe libslic3r)"
echo "PlanePath focused generator gate: ${libslic3r_exe} [FillPlanePath]"
"${libslic3r_exe}" '[FillPlanePath]' --order decl

fff_print_exe="$(find_test_exe fff_print)"
echo "PlanePath focused integration gate: ${fff_print_exe} [PlanePathConformance]"
"${fff_print_exe}" '[PlanePathConformance]' --order decl

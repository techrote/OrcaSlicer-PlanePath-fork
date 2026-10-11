# Windows x64 PlanePath test build

This workflow produces a development/testing build of the PlanePath derivative. It is **not an official OrcaSlicer release**.

## Repository authority and provenance

The active PlanePath implementation repository is `techrote/OrcaSlicer-PlanePath-fork`, the genuine OrcaSlicer fork established by the PlanePath repository migration.

This workflow was reconciled from legacy `techrote/OrcaSlicer-PlanePath` PR #20 after PP-001/PP-002 had already been migrated into the corrected fork. The legacy repository is retained only as historical provenance; new implementation work and test builds belong here.

The build always reflects the selected commit. Do not infer which PP milestones are present from the workflow name or this document: use the Actions run commit SHA and the repository's merged PR history as the authority.

## Run and download

1. Open **Actions → PlanePath Windows x64 Test Build** in this repository.
2. Either:
   - run it manually with **Run workflow** on the branch/commit you want to test; or
   - use a `test-build/**` branch, which triggers the workflow on push.
3. Select a successful run.
4. Under **Artifacts**, download the `OrcaSlicer_Windows_*_x64_portable` artifact. An installer artifact may also be present when packaging is enabled, but the portable artifact is preferred for development testing.
5. Extract the artifact into a **new directory**, preserving its complete layout, and run `OrcaSlicer.exe` from the extracted application directory.

GitHub requires authentication to download Actions artifacts. Artifacts expire according to the repository's Actions retention policy.

## What the workflow qualifies

The workflow deliberately reuses the fork's normal build machinery rather than introducing a separate Windows build path:

- `scripts/test_build_win.ps1` validates Windows build-script option handling before the expensive compilation starts;
- `.github/workflows/build_check_cache.yml` and its reusable build chain compile the Windows x64 clang-cl application, tests and packaging outputs;
- `.github/workflows/unit_tests.yml` runs the focused PlanePath test runner followed by the broader OrcaSlicer unit-test suite from the same build artifact.

This is complementary to `planepath_ci.yml`: normal PlanePath pull requests already receive Windows and Linux compilation/testing, while this workflow provides an explicit downloadable Windows test build on demand.

## Smoke checks on real Windows hardware

For a candidate build:

- confirm the application launches with its bundled DLLs/resources and completes the normal profile wizard;
- import and slice a simple STL;
- exercise the PlanePath/native pattern behavior relevant to the candidate commit and inspect G-code preview;
- slice at least one ordinary non-PlanePath model to check for obvious regression;
- where geometry behavior is under test, include a complex/holed model as well as a simple rectangular model.

Back up important profiles/projects and keep the development build separate from an everyday OrcaSlicer installation.

Do **not** use experimental output for unattended printing without inspecting G-code and first-layer behavior.

When filing a defect, record the exact Actions run commit SHA and include screenshots plus an STL/3MF or minimal reproduction where useful. Development artifacts are unsigned test output, not published/signed releases.

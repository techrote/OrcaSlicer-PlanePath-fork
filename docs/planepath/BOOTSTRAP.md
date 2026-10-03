# PP-001 bootstrap provenance

## Imported histories

PP-001 joins the pre-existing PlanePath planning repository to OrcaSlicer without rewriting either history.

The import merge is:

- merge commit: `316aa44382bb04a0b368cd565435c1912f8b1951`
- PlanePath/planning parent: `b4c70facd91fef41d64532f0de0f7caa1f17f88b`
- OrcaSlicer upstream parent: `23c77f15cfa38696d89552344d887ce938ac39d4`
- upstream URL: `https://github.com/OrcaSlicer/OrcaSlicer.git`

The merge has exactly two parents. The upstream commit remains reachable as a real Git ancestor; the source was not copied into a disconnected snapshot.

## Workflow import boundary

GitHub's ephemeral Actions token correctly refused to introduce upstream workflow files while importing the unrelated history. PP-001 therefore kept `.github/workflows/` unchanged in the history-joining merge, then restored selected upstream CI workflows through the repository API.

Active/restored upstream CI primitives:

- `build_check_cache.yml`
- `build_deps.yml`
- `build_orca.yml`
- `unit_tests.yml`
- `unit_tests_flatpak.yml`
- `check_locale.yml`
- `check_profiles.yml`
- `shellcheck.yml`

`build_all.yml` is retained as a manual full multi-platform validation workflow. It is intentionally not an automatic PR/nightly workflow in this fork.

PlanePath PRs use `planepath_ci.yml`, which performs the Linux x86_64 upstream build, runs the focused `[FillPlanePath]` Catch2 gate first, then runs the complete upstream unit-test suite from the same built test artifact.

Not activated from upstream:

- issue assignment/deduplication bots;
- PR merge/label bots;
- release/winget publishing;
- translation update automation;
- upstream scheduled/nightly publishing jobs.

Those are repository-governance/deployment automation, not source-build prerequisites, and enabling them in a derivative fork would be unsafe and noisy.

## Planning material

The root `README.md` follows upstream OrcaSlicer to reduce sync conflicts.

PlanePath-specific context is preserved at:

- `docs/PROJECT-PLANNING-README.md`
- `docs/rag/`
- `docs/planepath/`

## Baseline test entry point

Focused already-built test command:

```bash
bash scripts/planepath/run-tests.sh build/tests Release
```

The runner finds the platform-appropriate `libslic3r_tests` executable and executes Catch2 tag `[FillPlanePath]`.

For a clean full build, use Orca's normal dependency/build flow or the `PlanePath CI` workflow; the focused script deliberately does not invent a second build system.

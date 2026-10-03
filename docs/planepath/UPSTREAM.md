# Upstream synchronization

PlanePath is maintained as an OrcaSlicer-derived repository with the planning/RAG history preserved in the same Git graph.

## Imported base

- Upstream repository: `https://github.com/OrcaSlicer/OrcaSlicer.git`
- PP-001 pinned base: `23c77f15cfa38696d89552344d887ce938ac39d4`
- Upstream branch at selection time: `main`
- Selection date: 2026-09-18

The PP-001 bootstrap import is a two-parent merge joining the pre-existing PlanePath planning history to this exact upstream commit. Import merge: `316aa44382bb04a0b368cd565435c1912f8b1951`. It does not squash, copy-only, or replace either history. See `BOOTSTRAP.md` for the parent SHAs and workflow activation boundary.

## Local remote setup

After cloning this repository:

```bash
git remote add upstream https://github.com/OrcaSlicer/OrcaSlicer.git
git fetch upstream
```

If `upstream` already exists, verify it points to the URL above.

## External regression-suite pin

The Linux build runs `OrcaSlicer/orca-test-repo` as an additional regression gate. That suite is versioned independently from OrcaSlicer, so PlanePath pins an exact compatible revision in `.github/workflows/build_orca.yml` rather than cloning the test repository's moving `main`.

For the PP-001 Orca baseline `23c77f15cfa38696d89552344d887ce938ac39d4`, the pinned external-suite revision is `f87619dfc7a4754c88b51e8d53c780b67887b47c` (the latest suite commit predating the 2026-09-18 baseline selection).

When syncing Orca upstream, review and advance this pin deliberately as part of the synchronization PR, then require the pinned suite to pass against the new Orca source. This prevents a later external-test change from retroactively breaking an otherwise unchanged historical baseline while preserving the external gate.

## Routine upstream refresh

Use a dedicated branch and merge upstream rather than rewriting PlanePath history:

```bash
git switch main
git pull --ff-only origin main
git fetch upstream main
git switch -c maintenance/sync-orca-YYYYMMDD
git merge --no-ff upstream/main
```

Resolve conflicts deliberately, run the required PlanePath + Orca regression gates, and merge the synchronization through a PR.

For reproducible work, implementation PRs and release notes should record the upstream commit reachable from the branch at the start of work.

## Conflict policy

Prefer keeping upstream organization/naming where PlanePath has no intentional divergence. Keep PlanePath-specific material under dedicated names/paths such as:

- `docs/rag/`
- `docs/planepath/`
- `resources/planepath/` when introduced
- PlanePath-specific source namespaces/files introduced by implementation issues

Do not resolve an upstream conflict by silently dropping a PlanePath invariant or test.

## Root README

The root `README.md` follows upstream OrcaSlicer to minimize recurring merge conflicts. The original PlanePath project introduction is preserved at `docs/PROJECT-PLANNING-README.md`; `docs/rag/INDEX.md` is the canonical implementation-context entry point.

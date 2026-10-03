# Upstream synchronization

PlanePath is maintained in the GitHub fork `techrote/OrcaSlicer-PlanePath-fork` of `OrcaSlicer/OrcaSlicer`.

## Historical migration base

- Upstream repository: `https://github.com/OrcaSlicer/OrcaSlicer.git`
- historical PP-001 pinned base: `23c77f15cfa38696d89552344d887ce938ac39d4`
- original base selection date: 2026-09-18
- exact historical migration branch: `migration/planepath-linearized`

The historical PP-001 and PP-002 result trees were recreated byte-for-byte in the true fork before the current upstream catch-up. See `BOOTSTRAP.md`.

## Current synchronized upstream

The true-fork migration was initially ported onto OrcaSlicer `00bb4202fe50ca625e59c6fcbec573a2887d22ed`. A post-migration synchronization then advances the fork to OrcaSlicer `main` commit:

`8a6377f087e3f422275cd788339e1fa64a280f50`

The three additional upstream commits are:

- `84657ff11ea231ef717b76eb0a941ba64aa3ec4e` — Add Missing Includes Across the Remaining Sources and Tests (#16071)
- `c86e33db6dd96f20ef1fd92e1018269b95ee0c31` — Make the orca-wxwidgets skill find the wx source on Windows and in worktrees
- `8a6377f087e3f422275cd788339e1fa64a280f50` — Add Missing Includes Across src/libslic3r (#16068)

None of the 41 PlanePath-modified paths changed across these three commits.

## Local remote setup

A normal clone of the fork should use the fork as `origin` and OrcaSlicer as `upstream`:

```bash
git remote add upstream https://github.com/OrcaSlicer/OrcaSlicer.git
git fetch upstream
```

If `upstream` already exists, verify it points to the URL above.

## External regression-suite pin

The Linux build runs `OrcaSlicer/orca-test-repo` as an additional regression gate. PlanePath pins an exact suite revision so a later test-repository change cannot retroactively break an unchanged source baseline.

For the 2026-10-03 Orca catch-up, the pin remains:

`fcc70a676c708660511f0720557773738fa74ecd`

That was still `orca-test-repo/main` when the post-migration sync was prepared, so no pin change was required for the three-commit follow-up. Future Orca synchronization must review and deliberately advance this pin together with the source update, then require the pinned suite to pass.

## Routine upstream refresh

Use a dedicated synchronization branch rather than rewriting PlanePath history:

```bash
git switch main
git pull --ff-only origin main
git fetch upstream main
git switch -c maintenance/sync-orca-YYYYMMDD
git merge --no-ff upstream/main
```

Resolve conflicts deliberately, run PlanePath-focused and Orca regression gates, then merge the synchronization through a PR.

## Conflict policy

Prefer current upstream organization and implementation wherever PlanePath has no intentional divergence. Preserve PlanePath-specific material under dedicated paths such as:

- `docs/rag/`
- `docs/planepath/`
- `resources/planepath/` when introduced
- dedicated PlanePath source namespaces/files introduced by implementation work

Do not resolve an upstream conflict by silently dropping a PlanePath invariant or conformance test.

## Root README

The root `README.md` follows upstream OrcaSlicer to minimize recurring merge conflicts. The PlanePath project introduction is preserved at `docs/PROJECT-PLANNING-README.md`; `docs/rag/INDEX.md` remains the implementation-context entry point.

# PP-001 bootstrap and migration provenance

## Active repository identity

The active PlanePath repository is `techrote/OrcaSlicer-PlanePath-fork`, created on 2026-10-03 as a real GitHub fork of `OrcaSlicer/OrcaSlicer`.

GitHub records both the fork parent and source as `OrcaSlicer/OrcaSlicer`. Future upstream comparison and synchronization therefore use the native fork network rather than an unrelated repository graph.

## Historical PlanePath migration

The original standalone repository was `techrote/OrcaSlicer-PlanePath`. Its accepted milestones were preserved before catch-up:

- original PP-001 accepted tree: `4fa0c875fcc837ea0fa97dd8deb8aea4fe6d53d9`;
- original PP-002 accepted tree: `a0a1e0ef23b725d0240e94d59c82a29a8018ae12`;
- original pinned Orca ancestor: `23c77f15cfa38696d89552344d887ce938ac39d4`;
- migration branch: `migration/planepath-linearized`;
- linearized PP-001 commit in the true fork: `c745c865ee6f07e6f433bfb5738cb19d780a6dcf`;
- linearized PP-002 commit in the true fork: `5d7ac1d5ea06192ebf3f4a662bd5be68e1ac4f90`.

The two migrated trees were reconstructed in the fork's Git object store and verified to match the old accepted tree SHAs exactly. This preserves the historical result without carrying the standalone repository's unrelated-root bootstrap topology.

## October 2026 upstream catch-up

The current migration port is based on OrcaSlicer `main` commit `00bb4202fe50ca625e59c6fcbec573a2887d22ed`.

PlanePath-specific tests, planning/RAG material, and fork-safe CI policy are reapplied on top of that current upstream tree. Upstream changes win by default except where PlanePath has an intentional invariant or fork-safety divergence.

## Workflow boundary

Active fork-oriented CI includes the normal Orca build primitives plus `planepath_ci.yml`.

`build_all.yml` remains available as an explicit manual full multi-platform validation workflow; it is not an automatic PR/nightly workflow in this fork.

Repository-governance and publishing automation that belongs to the upstream project is intentionally not activated here, including issue assignment/deduplication bots, PR merge/label bots, release/winget publishing, translation automation, and upstream scheduled/nightly publishing jobs.

## Planning material

The root `README.md` follows upstream OrcaSlicer. PlanePath-specific context is kept under:

- `docs/PROJECT-PLANNING-README.md`
- `docs/rag/`
- `docs/planepath/`

## Baseline test entry point

Focused already-built test command:

```bash
bash scripts/planepath/run-tests.sh build/tests Release
```

The focused script runs the PlanePath generator and integration tags before the normal complete upstream unit suite used by PlanePath CI.

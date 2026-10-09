# PlanePath repository migration ledger

Prepared: 2026-10-03

## Repository cutover

- Legacy standalone repository: `techrote/OrcaSlicer-PlanePath`
- Active true fork: `techrote/OrcaSlicer-PlanePath-fork`
- Upstream parent/source: `OrcaSlicer/OrcaSlicer`
- Exact historical migration branch: `migration/planepath-linearized`
- Current-upstream port branch: `migration/current-main-port-2026-10-03`
- Migration PR: [fork PR #2](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/2)
- Migration merge commit: `e71fec82f43285567debc4d2b05d51fb47c0820b`

The historical PP-001 and PP-002 trees were recreated byte-for-byte in the true fork before the current-upstream port was prepared. Fork PR #2 passed PlanePath CI, Shellcheck and profile validation before merge.

## Pull-request mapping

| Legacy | Purpose | Migration state |
| --- | --- | --- |
| [PR #18](https://github.com/techrote/OrcaSlicer-PlanePath/pull/18) | PP-001 bootstrap | accepted result preserved in `migration/planepath-linearized` |
| [PR #19](https://github.com/techrote/OrcaSlicer-PlanePath/pull/19) | PP-002 conformance harness | accepted result preserved in `migration/planepath-linearized` |
| [fork PR #2](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/2) | current-upstream PlanePath cutover | merged green into true-fork `main` |

## Issue backlog

The true fork currently has GitHub Issues disabled. Until Issues are enabled there, these legacy issues remain the authoritative discussion/history records. When issue migration is performed, preserve the PP identifier in each new title/body and include the legacy URL.

| Legacy issue | State at cutover | Title |
| --- | --- | --- |
| [#1](https://github.com/techrote/OrcaSlicer-PlanePath/issues/1) | closed | PP-001 — Bootstrap OrcaSlicer upstream, provenance and CI baseline |
| [#2](https://github.com/techrote/OrcaSlicer-PlanePath/issues/2) | closed | PP-002 — Build native PlanePath and Hilbert conformance harness |
| [#3](https://github.com/techrote/OrcaSlicer-PlanePath/issues/3) | open | PP-003 — Centralize fill-pattern traits without changing behaviour |
| [#4](https://github.com/techrote/OrcaSlicer-PlanePath/issues/4) | open | PP-004 — Add scripted-plane-path sentinel and per-context config identity |
| [#5](https://github.com/techrote/OrcaSlicer-PlanePath/issues/5) | open | PP-005 — Embed deterministic sandboxed Lua runtime |
| [#6](https://github.com/techrote/OrcaSlicer-PlanePath/issues/6) | open | PP-006 — Define Pattern Package ABI v1 and dynamic registry |
| [#7](https://github.com/techrote/OrcaSlicer-PlanePath/issues/7) | open | PP-007 — Implement FillScriptedPlanePath through the native geometry pipeline |
| [#8](https://github.com/techrote/OrcaSlicer-PlanePath/issues/8) | open | PP-008 — Implement scripted Hilbert and prove native parity |
| [#9](https://github.com/techrote/OrcaSlicer-PlanePath/issues/9) | open | PP-009 — Harden scripted internal/top/bottom solid-surface integration |
| [#10](https://github.com/techrote/OrcaSlicer-PlanePath/issues/10) | open | PP-010 — Add dynamic scripted-pattern UI, parameters, reload and diagnostics |
| [#11](https://github.com/techrote/OrcaSlicer-PlanePath/issues/11) | open | PP-011 — Ship first curve pack: Peano and Moore |
| [#12](https://github.com/techrote/OrcaSlicer-PlanePath/issues/12) | open | PP-012 — Research and implement printable Wunderlich curve family |
| [#13](https://github.com/techrote/OrcaSlicer-PlanePath/issues/13) | open | PP-013 — Prove non-square PlanePath support and add Sierpiński/Gosper candidates |
| [#14](https://github.com/techrote/OrcaSlicer-PlanePath/issues/14) | open | PP-014 — Add project portability for installed pattern packages |
| [#15](https://github.com/techrote/OrcaSlicer-PlanePath/issues/15) | open | PP-015 — Add optional embedded package transport with trust and quarantine |
| [#16](https://github.com/techrote/OrcaSlicer-PlanePath/issues/16) | open | PP-016 — Security, concurrency, cancellation and performance hardening campaign |
| [#17](https://github.com/techrote/OrcaSlicer-PlanePath/issues/17) | open | PP-017 — Produce reproducible custom build, release docs and final regression gate |

## Cutover status

Completed:

1. Created a genuine GitHub fork of `OrcaSlicer/OrcaSlicer`.
2. Reconstructed and verified exact PP-001 and PP-002 historical trees.
3. Ported PlanePath onto current Orca source without replacing unrelated upstream work.
4. Passed the required PlanePath, shell and profile checks.
5. Merged fork PR #2 into the true fork.
6. Prepared a post-migration upstream sync for the three commits that landed during cutover.

Still required:

1. Enable Issues on the true fork.
2. Recreate the 17 legacy issues with legacy backlinks; close migrated #1/#2 and retain #3–#17 open.
3. Update any documentation links that should resolve to the recreated issue numbers.
4. Keep the standalone repository unchanged until issue/PR provenance is no longer needed for active work; archive it only as a separate deliberate action.

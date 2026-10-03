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
- Post-migration upstream sync: [fork PR #3](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/3)

The historical PP-001 and PP-002 trees were recreated byte-for-byte in the true fork before the current-upstream port was prepared. Fork PR #2 passed PlanePath CI, Shellcheck and profile validation before merge.

## Pull-request mapping

| Legacy | Purpose | Migration state |
| --- | --- | --- |
| [PR #18](https://github.com/techrote/OrcaSlicer-PlanePath/pull/18) | PP-001 bootstrap | accepted result preserved in `migration/planepath-linearized` |
| [PR #19](https://github.com/techrote/OrcaSlicer-PlanePath/pull/19) | PP-002 conformance harness | accepted result preserved in `migration/planepath-linearized` |
| [fork PR #2](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/2) | current-upstream PlanePath cutover | merged green into true-fork `main` |
| [fork PR #3](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/3) | three-commit upstream follow-up | validation/cutover follow-up |

## Issue migration

GitHub Issues is enabled on the true fork. All 17 legacy issues were recreated in order with their original titles and bodies plus a migration-provenance backlink.

The body migration was verified byte-for-byte after stripping the intentional migration header. The legacy tracker had no labels, assignees, milestones, locks or pre-existing comments to reproduce.

| PP ID | Legacy | Active | Active state |
| --- | ---: | ---: | --- |
| PP-001 | [#1](https://github.com/techrote/OrcaSlicer-PlanePath/issues/1) | [#4](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/4) | closed / completed |
| PP-002 | [#2](https://github.com/techrote/OrcaSlicer-PlanePath/issues/2) | [#5](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/5) | closed / completed |
| PP-003 | [#3](https://github.com/techrote/OrcaSlicer-PlanePath/issues/3) | [#6](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/6) | open |
| PP-004 | [#4](https://github.com/techrote/OrcaSlicer-PlanePath/issues/4) | [#7](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/7) | open |
| PP-005 | [#5](https://github.com/techrote/OrcaSlicer-PlanePath/issues/5) | [#8](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8) | open |
| PP-006 | [#6](https://github.com/techrote/OrcaSlicer-PlanePath/issues/6) | [#9](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/9) | open |
| PP-007 | [#7](https://github.com/techrote/OrcaSlicer-PlanePath/issues/7) | [#10](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/10) | open |
| PP-008 | [#8](https://github.com/techrote/OrcaSlicer-PlanePath/issues/8) | [#11](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/11) | open |
| PP-009 | [#9](https://github.com/techrote/OrcaSlicer-PlanePath/issues/9) | [#12](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/12) | open |
| PP-010 | [#10](https://github.com/techrote/OrcaSlicer-PlanePath/issues/10) | [#13](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/13) | open |
| PP-011 | [#11](https://github.com/techrote/OrcaSlicer-PlanePath/issues/11) | [#14](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/14) | open |
| PP-012 | [#12](https://github.com/techrote/OrcaSlicer-PlanePath/issues/12) | [#15](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/15) | open |
| PP-013 | [#13](https://github.com/techrote/OrcaSlicer-PlanePath/issues/13) | [#16](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/16) | open |
| PP-014 | [#14](https://github.com/techrote/OrcaSlicer-PlanePath/issues/14) | [#17](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/17) | open |
| PP-015 | [#15](https://github.com/techrote/OrcaSlicer-PlanePath/issues/15) | [#18](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/18) | open |
| PP-016 | [#16](https://github.com/techrote/OrcaSlicer-PlanePath/issues/16) | [#19](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/19) | open |
| PP-017 | [#17](https://github.com/techrote/OrcaSlicer-PlanePath/issues/17) | [#20](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/20) | open |

Each legacy issue now contains a redirect to the active issue. Legacy PP-003 through PP-017 were closed with state reason `not_planned` because the work moved; PP-001 and PP-002 remain completed.

## Cutover status

Completed:

1. Created a genuine GitHub fork of `OrcaSlicer/OrcaSlicer`.
2. Reconstructed and verified exact PP-001 and PP-002 historical trees.
3. Ported PlanePath onto current Orca source without replacing unrelated upstream work.
4. Passed the required PlanePath, shell and profile checks for the migration.
5. Merged fork PR #2 into the true fork.
6. Migrated and verified all 17 issues; retired the duplicate legacy backlog.
7. Prepared a post-migration upstream sync for the three commits that landed during cutover.

The legacy standalone repository should now be treated as read-only migration provenance. Archiving or deleting it is a separate repository-management decision and should not be mixed into code/history migration.

# PlanePath RAG index

This directory is the canonical context pack for autonomous work on **OrcaSlicer-PlanePath**.

## Retrieval order

For every implementation issue, read:

1. `00-PROJECT-BRIEF.md` — scope, definitions, success criteria, and non-goals.
2. `01-UPSTREAM-SEAM-MAP.md` — where the current OrcaSlicer implementation actually lives and the behaviours that must remain authoritative.
3. **`11-CODE-AUDIT-2026-09-18.md` — mandatory second-pass code audit and implementation blockers.**
4. The architecture document(s) named by the issue.
5. `06-TEST-CI-STRATEGY.md` — test evidence and merge gates.
6. `08-REVISED-ROADMAP.md` — issue dependencies and intended delivery order.
7. `09-AUTONOMOUS-EXECUTION-CONTRACT.md` — branch/PR/CI/merge/verification rules.

Read `07-PLAN-REVIEW.md` whenever changing architecture or scope; it records rejected shortcuts and why they were rejected.

## Topic routing

| Need | Read |
|---|---|
| Project purpose / v1 scope | `00-PROJECT-BRIEF.md` |
| Orca Hilbert / FillPlanePath implementation map | `01-UPSTREAM-SEAM-MAP.md` |
| Target components and data flow | `02-TARGET-ARCHITECTURE.md` |
| Lua API, manifests, quotas and sandbox | `03-SCRIPT-ABI-SANDBOX.md` |
| Trait model, config, serialization and project identity | `04-TRAITS-CONFIG-PORTABILITY.md` |
| Candidate curves and research leads | `05-CURVE-RESEARCH-CATALOG.md` |
| Tests, fixtures, CI and acceptance evidence | `06-TEST-CI-STRATEGY.md` |
| Critique of the first plan | `07-PLAN-REVIEW.md` |
| Dependency graph / issue campaign | `08-REVISED-ROADMAP.md` |
| Autonomous agent operating rules | `09-AUTONOMOUS-EXECUTION-CONTRACT.md` |
| **Current code-level blockers and corrected seams** | **`11-CODE-AUDIT-2026-09-18.md`** |

## Authority order

When documents conflict:

1. the current GitHub issue acceptance criteria;
2. `11-CODE-AUDIT-2026-09-18.md` for code facts discovered in the second-pass audit;
3. `09-AUTONOMOUS-EXECUTION-CONTRACT.md`;
4. `08-REVISED-ROADMAP.md`;
5. architecture/sandbox/config documents;
6. upstream seam map;
7. project brief.

If current OrcaSlicer upstream has moved, refresh the seam map and code-audit facts in the implementing PR rather than forcing stale paths onto new code.

## Preserved PP-005 prepass

[Complete archive][pp005-archive] · [Preservation note][pp005-preservation] · [Preservation manifest][pp005-manifest] · [Active issue #8](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8)

The preserved embedding prepass assessed commit `e71fec82f43285567debc4d2b05d51fb47c0820b` (tree `efe88c014f4129cd1ca847f84687f2a1ce162fad`). Its canonical archive is `checkpoints/prepass/PP-005/b1b0a100f2f62803/`; the final component derives from the original ZIP's SHA-256, not a Git commit. This preservation records the original work without implementing PP-005 or accepting its proposed runtime.

- [`ORIGINAL-PACKET.zip`][pp005-zip] preserves the exact `PP-005-embedding-prepass-e71fec82.zip`: **2,543,274 bytes**, SHA-256 `b1b0a100f2f6280311b89b411a10ec12b369392684eaa6ee31a8bdb25b5fe49c`.
- All **63 original regular-file members** are retained byte-for-byte under `unpacked/`. The unchanged original `MANIFEST.sha256` covers 62 payload files and excludes itself. Reports, source locators, integration/acceptance analysis, probe source, fixtures, six compiled probe paths, six raw JSONL streams, logs and inert proposals remain together.
- The canonical directory contains **67 files**: the ZIP, 63 unpacked originals, the [original external verification receipt][pp005-receipt], and two new preservation files. `PRESERVATION-MANIFEST.json` covers 66 files and excludes itself. The receipt retains its 4 October 2026 verification timestamp; it is not a new execution result.

The earlier partial checkpoints remain unchanged:

| Historical checkpoint | Recorded contents | Relationship to the complete original packet |
|---|---|---|
| [`57b5e58d…` / `bf6427bed5f7b3ad`][pp005-older] | 63 files: 57 original member contents and six wrapper files | Retains all six original raw JSONLs; lacks the original ZIP and six compiled probe paths. |
| [`8af2f698…` / `b46a20725b34c9a5`][pp005-newer] | 59 files: 52 original member contents and seven wrapper files | Retains one raw JSONL; the other five were already available in the older checkpoint. Also lacks the original ZIP and six compiled probe paths. |

The canonical archive preserves original modes: the six probe paths and `probe/run_pinned.sh` are `100755`; the other 56 original members, including `verify_zip.py`, are `100644`. Both earlier checkpoints recorded `run_pinned.sh` as `100644`; the newer one recorded `verify_zip.py` as `100755`. These historical differences remain visible. Executable modes describe original artifacts, not an instruction to run them.

Historical records report **33 probe cases per GCC 14.2, Clang 17 and host-side ASan/UBSan build** against installed Debian **Lua 5.4.7-1+b2**, a surrogate. The prebuilt Lua core was not sanitizer-instrumented. The proposal for Lua **5.5.1 plus GC fix `0b29f408433e92953cc72b1d3e06c7ac8139e439`** remains unaccepted: this packet does not establish exact-pin execution, native Windows packaging, full-runtime sanitizer coverage, Orca integration or complete sandbox acceptance. Preservation adds no new probe execution. The existing implementation requirements remain authoritative and issue #8 remains open.

[pp005-archive]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/3b41017eef8df5afd3cdd7f8e8cfacd94d867be2/checkpoints/prepass/PP-005/b1b0a100f2f62803
[pp005-preservation]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/blob/3b41017eef8df5afd3cdd7f8e8cfacd94d867be2/checkpoints/prepass/PP-005/b1b0a100f2f62803/PRESERVATION.md
[pp005-manifest]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/blob/3b41017eef8df5afd3cdd7f8e8cfacd94d867be2/checkpoints/prepass/PP-005/b1b0a100f2f62803/PRESERVATION-MANIFEST.json
[pp005-zip]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/blob/3b41017eef8df5afd3cdd7f8e8cfacd94d867be2/checkpoints/prepass/PP-005/b1b0a100f2f62803/ORIGINAL-PACKET.zip
[pp005-receipt]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/blob/3b41017eef8df5afd3cdd7f8e8cfacd94d867be2/checkpoints/prepass/PP-005/b1b0a100f2f62803/artifacts/PP-005-embedding-prepass-verification.json
[pp005-older]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/57b5e58df9127a480f63f7cad7be6b2d29408342/checkpoints/prepass/PP-005/bf6427bed5f7b3ad
[pp005-newer]: https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/8af2f69856b5953485fa5006c5a91138a9c45911/checkpoints/prepass/PP-005/b46a20725b34c9a5

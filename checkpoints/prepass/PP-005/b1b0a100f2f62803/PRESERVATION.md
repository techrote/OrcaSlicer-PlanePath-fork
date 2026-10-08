# PP-005 complete packet preservation

This checkpoint preserves the exact original `PP-005-embedding-prepass-e71fec82.zip`
and all 63 regular members as useful unpacked files. The canonical identifier
`b1b0a100f2f62803` is the first 16 hexadecimal characters of the original ZIP's
SHA-256. It is a new content-derived archive identifier, not an assessed Git commit.

## Identity and layout

- Repository/issue: [techrote/OrcaSlicer-PlanePath-fork #8](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8).
- Assessed historical commit: `e71fec82f43285567debc4d2b05d51fb47c0820b`.
- Assessed historical tree: `efe88c014f4129cd1ca847f84687f2a1ce162fad`.
- Intended additive archive branch: `checkpoint/prepass-PP-005-b1b0a100f2f62803`,
  based on that assessed historical commit. Actual durable ancestry and remote
  byte verification are established by the publication record, not by this note.
- Original file locator: `libfile_a6d3c36ce21881918fff24bd406e9f08`.
- Exact ZIP: [ORIGINAL-PACKET.zip](ORIGINAL-PACKET.zip), 2,543,274 bytes,
  SHA-256 `b1b0a100f2f6280311b89b411a10ec12b369392684eaa6ee31a8bdb25b5fe49c`.
- Original path mapping: every ZIP member `<path>` is preserved byte-for-byte at
  `unpacked/<path>`, with its original Unix regular-file mode. The original ZIP
  also preserves the original archive metadata.

[unpacked/MANIFEST.sha256](unpacked/MANIFEST.sha256) remains unchanged: it covers
62 payload files and excludes only itself, giving 63 original regular files and
no directory entries. Seven original paths have mode `100755`: the six compiled
probe paths and `probe/run_pinned.sh`; the other 56 have mode `100644`. Executable
permission is retained as historical metadata and does not authorize execution.

The separate [PRESERVATION-MANIFEST.json](PRESERVATION-MANIFEST.json) records the
original ZIP, all original unpacked members, the existing external receipt,
and this new note. Its 66 entries exclude the manifest itself, for 67 checkpoint
files in total. It explicitly
excludes itself to avoid a circular digest. No original manifest, report, result,
patch or verification script has been edited. The original README refers to a
separate historical verification JSON, now preserved unchanged at
[artifacts/PP-005-embedding-prepass-verification.json](artifacts/PP-005-embedding-prepass-verification.json).
That 2,877-byte receipt is outside the ZIP's 63-member inventory and was recovered
from the older checkpoint, Git blob `b3b8c126f02acf3eea506fd156ad3ba7812b727b`.
Its timestamp is `2026-10-04T21:03:24.012513+00:00`; its extracted-rerun results
are historical, not a new run in this preservation task. Its separately rebuilt
sanitizer executable has its own recorded hash and is not asserted to be one of
the six binaries inside the original ZIP. The new preservation manifest does
not replace or extend that historical run receipt.

## Relationship to earlier checkpoints

The prior partial archives remain historical records:

- [`57b5e58df9127a480f63f7cad7be6b2d29408342`](https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/57b5e58df9127a480f63f7cad7be6b2d29408342/checkpoints/prepass/PP-005/bf6427bed5f7b3ad/),
  branch `checkpoint/prepass-PP-005-bf6427bed5f7b3ad` (63 checkpoint files).
- [`8af2f69856b5953485fa5006c5a91138a9c45911`](https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/8af2f69856b5953485fa5006c5a91138a9c45911/checkpoints/prepass/PP-005/b46a20725b34c9a5/),
  branch `checkpoint/prepass-PP-005-b46a20725b34c9a5` (59 checkpoint files).

The 8 October 2026 audit identified the exact original ZIP and the six compiled
probes as missing across those two checkpoints. Some raw JSONL files absent from
the newer partial were already present in the older one. Live comparison found
57 of the 63 original member paths in the older checkpoint and 52 in the newer,
with no content mismatches among those present. The older checkpoint retained all
six raw JSONLs; the newer retained only `results/probe-clang.jsonl`. Both partials
recorded `probe/run_pinned.sh` as `100644`; the newer also recorded `verify_zip.py`
as `100755`, whereas the original ZIP has `100644` for that verifier. This archive
preserves the original ZIP's modes for both paths. This complete original
packet co-locates all original raw streams and binaries; it does not describe
those already published raw results as newly performed or previously absent.
Checkpoint wrapper counts above differ from the original packet's member count.
Neither partial is rewritten or removed by this additive preservation layout.

PP-003 was already preserved separately at
[`271ce3ad7b99c48f2c0c713a6f3c9c562d0b0491`](https://github.com/techrote/OrcaSlicer-PlanePath-fork/tree/271ce3ad7b99c48f2c0c713a6f3c9c562d0b0491/checkpoints/prepass/PP-003/2921c3fc3227/)
and is outside this task.

## Verification and evidence boundary

This preservation task checked the original ZIP's expected size and SHA-256,
all member CRCs, safe unique paths, regular-file types, all 62 historical hashes,
all 63 unpacked file bytes and modes, and Git blob identities. Static reads of
the six saved JSONL streams found one identity record plus 33 passing case
records in each; the saved reproduction summary's three executable hashes match
the corresponding retained binary bytes. These are checks of retained evidence.
No archived verification script, runner or binary was executed; no compilation,
patch application, new experiment or research continuation was performed.

The prepass recommended Lua **5.5.1 plus upstream GC fix
`0b29f408433e92953cc72b1d3e06c7ac8139e439`**, profile `5.5.1-pp1`.
Its actual historical evidence is 33 cases per GCC 14.2, Clang 17 and host-only
ASan/UBSan run against the installed Debian **Lua 5.4.7-1+b2 surrogate**. The
prebuilt Lua core was not sanitizer-instrumented. The packet's own
[HANDOFF](unpacked/HANDOFF.md), [assessment](unpacked/evidence/assessment.json),
[README](unpacked/README.md), and [acceptance limits](unpacked/ACCEPTANCE.md)
retain these qualifications.

Exact proposed-pin execution, native Windows packaging, full-runtime sanitizer
coverage, Orca integration and complete sandbox acceptance remain unestablished
by this packet. The proposed API/dependency files and patches remain inert
archival material. Complete preservation does not implement PP-005, close issue
#8, or accept the proposed runtime. Current-main navigation/status harmonization
is a separate documentation change; this historical branch is not a production
source overlay.

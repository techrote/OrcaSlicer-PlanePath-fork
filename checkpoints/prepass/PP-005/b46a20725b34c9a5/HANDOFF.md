# HANDOFF — PP-005 embedding prepass Git checkpoint

**Repository/issue:** `techrote/OrcaSlicer-PlanePath-fork` #8 / PP-005.
**Prepass subject:** deterministic sandboxed Lua embedding/runtime dependency.
**Assessed commit:** `e71fec82f43285567debc4d2b05d51fb47c0820b`.
**Assessed tree:** `efe88c014f4129cd1ca847f84687f2a1ce162fad`.
**Checkpoint purpose:** archival preservation only; this does not implement
PP-005 and does not satisfy its acceptance criteria.

## Findings preserved

The completed prepass recommends a C-built static Lua 5.5.1 runtime plus the
identified upstream GC fix `0b29f408433e92953cc72b1d3e06c7ac8139e439`, with
one independent Lua state per execution lifetime, protected C/Lua boundaries,
and independent instruction, allocator, native-work/output and cancellation
controls. See `unpacked/REPORT.md`, `unpacked/INTEGRATION.md` and
`unpacked/ACCEPTANCE.md` for the actual detailed work.

## What was actually executed

The retained records show 33 probe cases passing in each GCC 14.2, Clang 17,
and host-side ASan/UBSan builds against installed Debian Lua **5.4.7-1+b2**.
The exact Lua 5.5.1 pin was not executed. The already-produced ZIP was checked
with `unzip -t`, its original SHA-256 was rechecked, and its extracted
rebuild/rerun verification JSON is retained under `artifacts/`.

## Results and evidence boundaries

Executed evidence is the local Lua 5.4.7 surrogate probe and its recorded
compiler/runtime outputs. Analysis/proposal is the Lua 5.5.1+GC-fix runtime
choice, wrapper/API design and dependency integration plan. Unexecuted
follow-up includes the exact patched runtime build, Windows packaging, full
runtime sanitizer coverage, Orca integration and the full adversarial
acceptance matrix. No proposed patch was applied to active source.

## Artifact/publication limitation

The exact original ZIP is still the authoritative handoff artifact, SHA-256
`b1b0a100f2f6280311b89b411a10ec12b369392684eaa6ee31a8bdb25b5fe49c`.
The GitHub write connector in this turn cannot ingest a local binary by path;
therefore the checkpoint retains its sidecar hash, external verification JSON,
archive-internal manifest and useful UTF-8 unpacked files, but not a duplicate
binary ZIP blob or the six compiled probe binaries. See `ARTIFACTS.md` and
`SOURCE_PAYLOAD.sha256`.

## Recommended next implementation action

First build the exact patched Lua 5.5.1 source and run the retained
`unpacked/probe/run_pinned.sh` / CMake probe. Only after that evidence exists,
port the tested boundaries into `ScriptRuntime`, implement the audited sandbox
allowlist, wire the `deps/` ExternalProject/import target path, and execute the
minimum adversarial/platform matrix. Read live `main` before implementation;
this checkpoint intentionally remains based on the assessed parent above.

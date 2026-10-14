# PP-005 — Embedded Lua sandbox implementation and acceptance log

Issue: [#8](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8).
Implementation: [PR #22](https://github.com/techrote/OrcaSlicer-PlanePath-fork/pull/22).
Complete runtime architecture and maintained ABI: [ScriptRuntime.md](../HLSD/ScriptRuntime.md).

## Dependency

`deps/Lua/` pins PUC Lua **5.5.1** from the official archive
`1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce`,
with upstream GC fix `0b29f408433e92953cc72b1d3e06c7ac8139e439`.
The C-only core, auxlib, bundled patch and MIT notice are installed by Orca's
existing dependency superbuild. Release and Debug static CRT imports are kept
strictly separate. The main Orca build explicitly requires the exact
`PlanePathLua::Lua` target/version/profile under its dependency prefix and
does not search for system/PATH Lua.

## Implementation

- `src/libslic3r/Script/ScriptRuntime.{hpp,cpp}`: one production C++ host,
  one new Lua VM and allocator per invocation, protected text-mode execution,
  explicit selected math/base helpers, sealed `ctx` and `out` proxy objects,
  source/context validation, independent VM/native-work/output quotas,
  cancellation, bounded diagnostics and failure rollback.
- `src/libslic3r/CMakeLists.txt`: static internal runtime linked privately to
  `libslic3r`; no library implementation is duplicated for tests.
- `tests/libslic3r/test_script_runtime.cpp`: Catch2 production source tests.
- `scripts/planepath/lua-runtime-tests/`: focused cross-platform executable
  compiling/linking the *same* production host with the installed pinned Lua.
- `scripts/planepath/lua-dep-smoke/gc.c`: patched generational collector
  stress case, also used in the instrumented C source configuration.
- `resources/licenses/`: complete Lua 5.5.1 MIT notice and build provenance.
- `.github/workflows/planepath_lua_dependency.yml`: Linux, Windows
  Release/Debug and full Lua-core + host ASan/UBSan checks.

The original preserved surrogate packet remains at
`checkpoints/prepass/PP-005/b1b0a100f2f62803/unpacked/`. Its Lua 5.4.7
results are *not* used as current acceptance evidence.

## Evidence and qualification

- [Run 38142836010](https://github.com/techrote/OrcaSlicer-PlanePath-fork/actions/runs/38142836010):
  native targeted Linux and Windows static import/runtime tests, PASS.
- [Run 38143175733](https://github.com/techrote/OrcaSlicer-PlanePath-fork/actions/runs/38143175733):
  Linux/Windows Release, macOS Release, and Linux Lua C/host sanitizer jobs
  PASS; Windows Debug configuration was an independently diagnosed failure.
- [Run 38143705028](https://github.com/techrote/OrcaSlicer-PlanePath-fork/actions/runs/38143705028):
  Linux Release, Windows Release, **Windows Debug**, macOS Release,
  instrumented Lua core/host sanitizer suite: **5/5 PASS**.
  This predates the PP-003 reconciliation and the later new native exception/
  non-tail recursion checks, which require fresh CI evidence.
- PP-003 was merged into main at `3d50e664b74d0426e4ee5117c563ba290025299f`.
  PR #22 reconciles main by preserving its 31-pattern trait work and adding
  only the PP-005 target and Catch2 registrations in the two overlapping
  CMakeLists; it does **not** alter the accepted pattern semantics.

Passing targeted tests alone does not satisfy the gates below.

## Remaining acceptance gates — do not merge until complete

- Full `PlanePath CI` passes on the final head (Linux and Windows full Orca
  build, unit tests, source audit and PP-002 conformance).
- Final security review of the production host: hostile capabilities,
  allocator limits, cancellation, stack/recursion, diagnostics and native
  exceptions. Fix critical/high findings.
- PP-005 must not create slicer infill or G-code.

Release packaging (installers, macOS `.app`, Flatpak/FHS layout, clean-host
smoke tests, extra architectures) is **not** a PP-005 gate. It belongs to
PP-017 ([#20](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/20)).
Packaging checks already run for PP-005 remain useful history but do not
block this merge. Owner decision, 2026-10-13: no test builds or packaging
work until a build contains something the owner can see or use.

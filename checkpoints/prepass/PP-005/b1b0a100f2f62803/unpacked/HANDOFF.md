# HANDOFF — PP-005 embedding prepass

**Repository/issue:** techrote/OrcaSlicer-PlanePath-fork #8.
**Assessed commit:** `e71fec82f43285567debc4d2b05d51fb47c0820b`.
**Assessed tree:** `efe88c014f4129cd1ca847f84687f2a1ce162fad`.
**Scope:** read-only research and isolated CPU probes. No GitHub mutation, no
sentinel, no geometry adapter, no #6 trait work. PP-005 is not complete.

**Recommendation:** C-built static Lua 5.5.1 plus upstream GC fix
`0b29f408433e92953cc72b1d3e06c7ac8139e439`, profile `5.5.1-pp1`.
One fresh independent state per initialization + generate execution; protected
trivial callbacks; quotas for allocator/native work/output separate from hooks.

**Actual evidence:** 33 cases pass in each GCC 14.2, Clang 17, and host-only
ASan/UBSan run against installed Debian Lua **5.4.7-1+b2**. Canonical commands,
hashes, statuses and cleanup results: `results/reproduced/`. Exact 5.5.1 archive
retrieval failed due local DNS; no exact-pin, Windows, full-runtime sanitizer,
Orca build or complete sandbox validation is claimed.

**Read:** REPORT.md; INTEGRATION.md; ACCEPTANCE.md; SOURCES.md.
**Restore/run:** README.md. MANIFEST.sha256 covers packet files; verify_zip.py
checks ZIP CRC, exact membership, path safety, sizes and all manifest hashes.

**Intended paths:** `deps/Lua/*`; common `deps/CMakeLists.txt`; root CMake import;
`src/libslic3r/Script/ScriptRuntime.{hpp,cpp}` and libslic3r linkage;
`tests/libslic3r/test_script_runtime.cpp`, `tests/data/planepath/runtime/`;
`resources/licenses/Lua-5.5.1.txt`; implemented design in
`docs/HLSD/ScriptRuntime.md`, relevant ABI/audit updates. Proposal header is
not implemented; additive recipe patch is not wired or merge-ready.

**Next gate:** build the exact patched archive and run `probe/run_pinned.sh` /
CMake probe, then port tested boundaries and implement the audited allowlist.
Resolve default limits, cross-platform numeric determinism, worker-stack
ceilings, hard-versus-cooperative latency, Flatpak offline source stanza and
macOS notice staging. Read live main before applying: open sync PR #3 was
observed, but its head was not substituted for the assessed commit.

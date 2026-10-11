# PP-005 implementation log — Lua embedding

Active issue: [PP-005 / #8](https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8).

## First production slice: exact dependency

The source-of-truth implementation belongs under `deps/Lua/`. This phase
integrates static Lua 5.5.1 plus the upstream GC bug fix into Orca's
`ExternalProject` dependency superbuild, including installable config targets
and a pinned-version smoke test. The source archive hash and patch provenance
are recorded in `deps/Lua/PATCHES.md`; no separate system runtime is allowed.

The targeted Linux and Windows CI checks exercise `dep_Lua` directly so
developers do not need to rebuild every Orca dependency just to validate the
pinned runtime. The Flatpak offline source list also carries the exact archive.
A full slicer build, runtime sandbox and executable shipping still need further
work before #8 can close.

## Pending PP-005 acceptance

- Implement the production C++ runtime host in `src/libslic3r/Script/`.
- Use independent fresh Lua states per invocation and source-only load.
- Add deterministic selected math/functions with a minimal global allowlist.
- Add bounded instruction count, allocator-backed memory, native work/output
  quotas, cancellation, bounded errors and worker-stack limits.
- Port adversarial and concurrency tests using the preserved PP-005 probes.
- Wire only the accepted runtime into libslic3r and add the application
  license notice to the actual portable/installer/macOS/Flatpak outputs.
- Run required focused, Linux and Windows integration suites, including
  static dependency and exact patched-source provenance checks.

The preserved historical packet in
`checkpoints/prepass/PP-005/b1b0a100f2f62803/unpacked/` is read-only research,
not a production implementation. See its HANDOFF/INTEGRATION/ACCEPTANCE notes.

# Pinned Lua source and security maintenance

- Source: PUC-Rio Lua **5.5.1**, official release 2026-07-24.
- Archive: https://www.lua.org/ftp/lua-5.5.1.tar.gz
- SHA-256: `1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce`.
- Licence: MIT (full notice at `LICENSE.lua.txt`).
- Local profile: `5.5.1-pp1` (not an upstream Lua version).

The source archive is patched with the **two source hunks** from Lua upstream
commit `0b29f408433e92953cc72b1d3e06c7ac8139e439` (2026-09-16).
Original: https://github.com/lua/lua/commit/0b29f408433e92953cc72b1d3e06c7ac8139e439
Bug reference: https://www.lua.org/bugs.html#5.5.1-1

The patch bounds negative allocation growth before a generational-GC decision
and asserts a helper precondition. A failed source patch must fail the build.
Do not silently fall back to unpatched or system Lua.

The static C library deliberately omits Lua's standalone interpreter and the
full `linit`, `loadlib`, `lbaselib`, `lmathlib`, `lstrlib`, `ltablib`,
`liolib`, `loslib`, `ldblib`, `lcorolib`, and related registration objects.
This minimizes linkable script capabilities but is **not a complete sandbox**:
PP-005 must still provide a strict host allowlist, source-only loader,
allocator/VM/output/native-work limits, cancellation and adversarial tests.

The initial dependency smoke test checks the exact pinned API, installation
and imported static target. Passing it does not accept the sandbox, scripts,
GUI or a printable geometry adapter.

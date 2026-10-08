# Proposed Lua dependency profile

Base: Lua 5.5.1 official release archive, SHA-256 in Lua.cmake.
Revision: `5.5.1-pp1` (local packaging/profile identifier, not an upstream version).

`gc-negative-growth.patch` adapts only source-root paths from upstream commit
`0b29f408433e92953cc72b1d3e06c7ac8139e439`, authored 2026-09-16.
Primary: https://github.com/lua/lua/commit/0b29f408433e92953cc72b1d3e06c7ac8139e439
Bug: https://www.lua.org/bugs.html#5.5.1-1
It guards a negative allocation delta before applying a GC parameter and adds
an assertion of the parameter-helper precondition. Upstream's regression is
in `testes/gengc.lua`; retrieve and execute it with an instrumented full test
interpreter during dependency acceptance, not inside the sandbox allowlist.
The two production hunks were inspected against the published 5.5.1 source.
Actual application/build against the tarball was NOT possible here.

The public sandbox does not expose collector controls and stays in default
incremental mode. This is defense in depth, not a claim that denying that API
proves every path to the bug unreachable.

C99, static, int64/double, private LUAI_MAXSTACK=32768/LUAI_MAXCCALLS=100,
no standard-library registration objects, no OS/platform loader options.
The `lauxlib.c` unit still contains file-related auxiliary APIs; their presence
in a binary is NOT script authority. The host must never register/use them for
untrusted source. All source loading is a validated bounded memory buffer.

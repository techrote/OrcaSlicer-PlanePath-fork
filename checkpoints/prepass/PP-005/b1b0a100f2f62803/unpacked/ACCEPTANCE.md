# Minimum adversarial matrix for PP-005 acceptance

`Observed` below means the **Debian 5.4.7 mechanism surrogate**, not the proposed
5.5.1-pp1 build. All production rows must be rerun against the exact static
runtime and the actual capability table. Use finite subprocess/CTest timeouts;
a timeout is a test failure, not evidence that the sandbox cancelled safely.

| Boundary | Minimum cases and required assertion | Evidence / remaining work |
|---|---|---|
| Dependency identity | Correct archive succeeds; wrong hash/version/profile and absent prefix fail; unrelated system Lua cannot satisfy lookup. | Proposed recipe only; exact build/import pending. |
| Allocator | Null/tagged allocation, free/null-free, exact-limit and limit+1 growth, SIZE_MAX arithmetic, failed growth preserves content, shrink and capacity reuse, malloc failure. | Core transitions observed; exact boundary systematic sweep still needed. |
| OOM lifecycle | Failure at every allocatable stage of state creation, library/context setup, parsing, execution, error capture and closing; no panic, leak, double-free or partial successful output. | Startup + six fail-from points and running memory bomb observed. Exhaustive allocation-index sweep and production setup pending. |
| VM fuel | Empty/short success, infinite loop, tail recursion, initializer loop, loop in generate, boundary just before/at cap; fuel is not reset between phases. | Loop/tail cases observed; two-phase generate binding pending. |
| Cancellation | Pre-cancel, atomic cancel during Lua loop, cancel in each host callback, immediately before/after compile and output commit; sticky failure prevents later emission. | Lua async and native-loop polling observed; production phases pending. |
| Native-work quota | Oversized operands, varargs, strings, array copies and any supported sort/pattern helper; no uncharged unbounded loop or native recursion. | Synthetic native loop observed with zero hook progress; every real exposed C helper pending. |
| Protected C++ boundary | Native bad_alloc/other exception is caught in a Lua-free C++ island; destructor counters correct; Lua raises only from trivial frames; outer translation also unwinds normally. | Native vector/exception and outer guard observed. Real output allocation exceptions and application translation pending. |
| Loader and syntax | Text-only binary rejection, malformed source, embedded NUL, oversized input, giant literal, deep syntax nesting; retained original loader status. | Syntax and ESC-Lua binary input observed, including loader status 3 versus outer status 2. Source-cap/parser adversaries pending. |
| Recursion/stack | Non-tail Lua recursion, tail recursion, C/Lua reentry, huge argument/result counts, syntax nesting; run on actual worker stack sizes. | First three observed. Proposed lower build ceilings and Windows stacks pending. |
| Capability graph | Globals AND nested tables/string methods/upvalues/context/userdata cannot reach io/os/package/debug/load/FFI/native modules/environment. No `pcall`, coroutine, `__gc`, or `__close` bypass. | Bare-test global absence observed; actual allowlist graph and locked context pending. |
| Diagnostics | Very large string, arbitrary object error, hostile tostring/traceback capability attempt, OOM during error formatting; fixed-size message, trusted identity, relative path, line optional under exhaustion. | 96-byte truncation and arbitrary table without conversion observed. Production metadata/handler pending. |
| Context immutability | Writes to all context fields and nested params rejected; invalid enums, infinities, out-of-range numbers refused before execution. No shared mutable tables. | ABI requirement; not implemented in probe. |
| Output accounting | Points at limit/limit+1, checked native buffer capacity and arithmetic, invalid coordinates, post-cancel output, output exception rollback. No successful partial result. | Counter seam designed; emitter/coordinate adapter not implemented here. |
| Isolation | Repetition, randomized test order, 8+ simultaneous independent states, one failure alongside successes, package-snapshot reload while executing. | Eight isolated counters + fresh success observed. Mixed failure/snapshot reload pending. |
| Cleanup callbacks | No user finalizer or close method can execute after cancellation; trusted userdata cleanup is finite/noexcept/no allocation/no blocking. | Bare environment has none. Real userdata metatable audit pending. |
| GC fix | Upstream generational regression under full-runtime UBSan, before/after negative control where safe; stress default collector with enforced memory budget. | Patch source reviewed, not applied to actual release here; regression not run. |
| Packaging | Win x64 MSVC/VS Release, clang-cl/Ninja Release/Debug; maintained ARM64 release; clean host with no Lua, no Lua DLL import, relocatable prefix, notice in portable ZIP and installer. Linux/macOS smoke and Flatpak offline build. | No native packaging builds performed. |
| Error integration | Actual runtime failure becomes user-visible SlicingError, not swallowed missing infill; no native fallback. | Runtime error result design here. Later adapter integration gate remains explicit. |

No GPU, printer, USB, sentinel, geometry adapter or #6 fill-trait implementation
is needed to establish the core embedding rows. Do not wait for those unrelated
workstreams to run allocator, exception-boundary and dependency-package tests.

## Remaining design decisions to close, not hide

Final default resource limits need baseline data: the probe's 64 KiB/100-opcode
settings and proposed stack ceilings are stress/configuration choices, not
production performance recommendations. Define whether cross-platform output
is bit-identical or normalized to the ABI coordinate contract; validate the
selected math profile accordingly. Decide whether cooperative wall-time
cancellation meets the product promise; strict hard termination requires a
process boundary. Reconcile the maintained Flatpak manifest's exact source
entry and the macOS app resource-copy path. Validate the exact Lua patch and
compiler configurations before promoting the runtime recommendation to an
accepted dependency.

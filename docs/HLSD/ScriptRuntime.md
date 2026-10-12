# PlanePath Lua runtime: execution and trust boundary

The internal production target is `planepath_script_runtime`, linked privately into
`libslic3r`. The public C++ interface is
`src/libslic3r/Script/ScriptRuntime.hpp`; it does not expose Lua pointers or
allocator internals. PP-005 creates and destroys a **new isolated Lua state for
every call**, including failure paths. This is not a scripted-fill adapter.

## Dependency and identity

The dependency superbuild obtains exact PUC Lua **5.5.1**, SHA-256
`1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce`,
applies the upstream GC fix `0b29f408433e92953cc72b1d3e06c7ac8139e439`,
and exports static `PlanePathLua::Lua` with profile `5.5.1-pp1`. Orca's main
CMake build uses only that package under the explicitly configured dependency
prefix; no `FindLua`, system fallback, runtime download, interpreter executable
or Lua DLL is required. The complete MIT notice and patch provenance are
packaged under `resources/licenses/` as well as the dependency prefix.

## Invocation contract

`execute(source, context, limits, cancel)` borrows immutable inputs for the
duration of its synchronous call. Source is 1–65536 bytes by default,
text-only, with no embedded NUL and no binary signature. The script may define
`function generate(ctx, out)` and use the following allowlist:

- Lua syntax: local variables, closures, deterministic numeric loops, literal
  arrays/maps and ordinary language operators;
- global functions: `type`, `assert`, `error`;
- `math`: `abs`, `floor`, `ceil`, `sqrt`, `sin`, `cos`,
  `min`, `max`, numeric `pi`;
- read-only fields `ctx.min_x/min_y/max_x/max_y/resolution/layer_index`;
- read-only `ctx.params.<name>` for validated finite numbers, booleans and
  bounded ASCII enum strings;
- bounded `out:point(x,y)`, representing a sequence of finite, fill-local
  coordinates.

The `ctx`, `ctx.params` and `out` interfaces are empty Lua proxy tables
with inaccessible host storage and rejecting `__newindex` metamethods.
The VM cannot access the underlying C++ pointers. No `_G`, `rawset`,
`setmetatable`, `getmetatable`, `pcall`, `xpcall`, `load`, `io`,
`os`, `package`, `require`, `debug`, `coroutine`, `string`
library, `table` library, random number generation or dynamic library
registration is present. No full standard libraries are linked or opened.

The host input supplies increasing ASCII parameter names and a validated finite
domain; no ambient clock, locale, process, filesystem, networking or environment
information enters a VM. Each VM uses explicit hash seed `0x50500551`.
Math transcendental results are reproducible on a fixed implementation/build;
**bit-for-bit cross-platform floating point equivalence is not promised**.
Future adapters must normalize to the accepted fill grid/coordinate type.

## Independent resource limits

Default ceilings, adjustable only by trusted host code:

| Budget | Default | Enforcement |
|---|---:|---|
| Source | 64 KiB | Bounded preparse validation, `luaL_loadbufferx(..., "t")` |
| VM retained allocation | 8 MiB | Custom Lua allocator, alignment/header and retained-capacity accounting |
| Lua VM instructions | 4,000,000 | Count hook, terminal error at exact budget |
| Hook/cancellation interval | 128 VM instructions | Adaptive count hook and atomic cancellation |
| Host native work | 300,000 units | Charged on all exposed host operations; vector-copy work charged |
| Emitted points | 100,000 | Reject beyond quota; no partial successful results |
| Output allocated bytes | 2 MiB | Bounded vector reserve, separate from Lua allocator |
| Diagnostic | 255 bytes | Truncation into fixed 256-byte buffer, without user `tostring` |

The VM memory ceiling accounts for alignment-safe host allocation headers, and
shrink-without-reallocation retains its full previously reserved charge.
Allocation failures do not alter the original pointer/accounting. VM state
destruction is checked for zero retained allocations and live blocks.

Lua instruction hooks cannot preempt arbitrary native code. The allowlisted
native operations above are bounded, check cancellation, and charge work.
There are no arbitrary script-defined native C callbacks or long host loops.
**Hard real-time/process-level preemption is not provided**; a subprocess is
needed if future APIs require noncooperative native code.

## Protected Lua/C++ boundaries

Source loading uses text-only `luaL_loadbufferx`. Setup, top-level
initialization and `generate` invocation run inside protected Lua calls.
All callbacks capable of `lua_error` contain only trivially destructible
automatic variables; host container allocation exceptions are caught and
unwound before invoking Lua error propagation. The executable core is compiled
as C, not C++.

The return result contains a structured error category, phase, Lua status,
bounded diagnostic, output points and execution/cleanup statistics. On every
error, points are cleared before return. Script error objects are never
converted using script-defined metamethods.

PP-005 does not handle G-code, extrusion, printer state, clipping, infill
selection, project-carried trust, script packages, registration or script
selection UI. Those belong to PP-006 and later issues.

## Verification

The production host is the same source used by the narrow Windows/Linux
static imported-target security harness and `tests/libslic3r` Catch2
regressions. CI gates include allocator fault injection, instruction and
native-work exhaustion, unsafe-name absence, immutable bindings, output
quotas, finite-coordinate validation, diagnostic bounds, asynchronous
cancellation, repeated determinism and eight concurrent VMs. Platform
qualification, sanitizers and packaged-artifact checks must still be
recorded in the PP-005 PR with exact run evidence before acceptance.

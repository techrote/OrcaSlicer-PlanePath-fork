# PP-005 embedding prepass

Assessed repository: `techrote/OrcaSlicer-PlanePath-fork`, issue #8.
Assessed commit: **e71fec82f43285567debc4d2b05d51fb47c0820b**.
Assessment date: 2026-10-04. Scope: embedding only; no #6 trait work, sentinel,
geometry adapter, remote mutations or workflow actions.

## Decision and evidence boundary

Choose **PUC Lua 5.5.1, compiled as C and statically embedded**, plus the upstream
GC negative-growth correction from `0b29f408433e92953cc72b1d3e06c7ac8139e439`.
Call this dependency profile `5.5.1-pp1`; it is not an upstream release name.
The official archive SHA-256 is
`1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce`.
The official download list dates it 2026-07-24. The official bugs page says
5.4.9 is the last 5.4 release; using the older branch for a new embedding would
therefore start with a maintenance disadvantage. [U1, U2]

This choice follows the existing ABI, rather than replacing Lua with LuaJIT,
a different dialect, an external interpreter, or a Python plugin. Only Lua's
core and auxiliary C API are needed in the initial static library. Selected
language helpers are a separately reviewed allowlist, not `luaL_openlibs()`.
There is no dependency on a system Lua installation or PATH. [R3, R4]

**Release suitability is source-reviewed, not binary-accepted.** Network DNS
failed in the local build workspace even though connector/web browsing worked.
The release archive was not downloaded or compiled. The actual probe used the
already installed **Debian liblua5.4-0:amd64 5.4.7-1+b2**, with its shared-object
hash and ELF build ID recorded. That is a mechanism surrogate, not an allowed
production fallback. The exact 5.5.1+patch build, native Windows execution,
full sandbox, and Orca integration remain acceptance gates.

The 5.5.1 bug is not ignored: an upstream fix prevents a negative memory delta
from being shifted during a generational-GC decision and asserts the helper's
precondition. The two production hunks are included with source paths adapted
to the tarball. Default incremental collection and withholding collector
controls reduce exposure, but are not a proof of universal unreachability.
The upstream regression must run with an instrumented exact runtime. [U2, U9]

## What the ABI actually permits

The repository describes `generate(ctx, out)` and continuous `out:point(x,y)`
output. Context contains local-grid extents, resolution, validated parameters,
and a layer index only where stable. These are **fill-aligned local grid
coordinates**, after Orca's native rotation/origin decisions; not printer XY.
There is no path-break, extrusion, G-code, filesystem or printer-control
capability. Manifest metadata must not require script execution. [R3]

For PP-005, make quota accounting and protected execution independent of the
actual emitter. PP-007 can bind the existing ABI to the type-correct output
seam. This prepass's `host_fail`, `host_work`, `host_reenter`, and `raise` are
explicit **test-only** functions, not additions to the public ABI.

Proposed first allowlist: a bounded `error`/`assert`, `type`, array-only `ipairs`,
and numeric helpers actually needed by the reference curves (initially
`abs`, `min`, `max`, `floor`, `ceil`, with argument-count checks). Lua tables,
indexed reads/writes, arithmetic, comparisons and control flow remain language
features. Add string `len`/bounded `sub` or array helpers only with a demonstrated
package need and a cost contract. The ABI permits a selection of libraries;
it does not require entire `math`, `table`, or `string` libraries.

Do not expose `pcall`, `xpcall`, coroutine creation/resume/yield, `debug`,
`package`, `require`, `load`, `loadfile`, `dofile`, `string.dump`, native module
loaders, `io`, `os`, environment access, `print`, `warn`, collector controls,
randomness, `pairs`/`next`, arbitrary `tostring`, or script metatable mutation.
No accessible host object has script-defined `__gc` or `__close`. Full userdata
may privately implement a locked immutable context/emitter; it must expose no
raw pointer, arbitrary userdata constructor, or user-selected native method.
The terminal lightuserdata error marker is internal and reaches only the host.

Avoid opening full libraries and deleting a few globals. In particular, a
string method can reach a library table through the string metatable; sanitizing
only a second global copy misses that path. Pattern matching, formatting,
concatenation helpers and sorting are not free just because they are C code.
The published string implementation has its own recursive matcher. [U10]

Determinism requires more than removing random functions. Use fixed Lua 5.5
hash seed (the new state API accepts it), bounded numeric configuration,
canonical parameter ordering and no observable address/hash iteration. [U3]
Keep int64/double, no fast-math, and do not change process locale from workers.
Lua's power arithmetic uses platform numeric support; a fixed seed does not
prove cross-platform floating-point equality. [U11] The exact cross-platform
numeric promise needs a tested curve profile, especially before adding
transcendental functions. Same-build repeated output is a first gate, not a
substitute for that question.

## Allocator accounting: three numbers, not one subtraction

The callback must distinguish a fresh allocation from resizing. For a null
pointer, `osize` may be a Lua object type tag and is not old allocated bytes.
For a real pointer it is the prior logical request. Zero new size frees the
block and returns null. A failed resize leaves the old block intact. The
runtime can perform emergency GC and retry an allocation. [U4]

The probe tracks **logical live bytes**, **reserved payload capacity plus its
own aligned allocation header**, and **peak reserved bytes**. Headers preserve
logical size and retained capacity separately. The production recommendation
is the same conservative accounting, with all arithmetic checked before any
allocation:

| Transition | Required accounting |
|---|---|
| Fresh block | Ignore the type tag as an old size; charge payload plus header after success. |
| Grow beyond capacity | Check `new_size + header` for overflow and the net reserved charge against the cap; mutate accounting only after success. |
| Allocation failure | Return null; do not decrement old bytes, destroy the old block, or invent a successful pointer. |
| Shrink | Keep the same allocation if necessary, change logical size, and keep charging retained capacity. |
| Grow within retained capacity | Reuse the block, change logical size, leave reserved charge unchanged. |
| Free | Subtract the stored retained capacity/header and logical size; free exactly once. |

The shrink strategy is intentionally infallible even though the inspected 5.5
source also has a path that can report a failed shrink. It is conservative,
not an assertion that all Lua versions have identical shrink requirements.
The direct allocator test proves failed-grow pointer contents, tagged fresh
allocation, shrink/grow reuse, overflow rejection and final zero accounting.

Allocator denial is not automatically terminal: after collection the retry
may legitimately fit. Classify final `LUA_ERRMEM` (or a preserved loader memory
status) as exhaustion. Do not raise a Lua error from inside the allocator, log
through an allocating logger there, call Lua recursively, or throw C++ from it.
Injected failures must leave frees and shrink/reuse operational during cleanup.

This quota is **not an RSS bound**. It excludes libc allocator metadata,
fragmentation, C stack, source snapshots owned outside Lua, native output,
other executions, and transient old-plus-new storage that `realloc` may need.
A physical-allocation ceiling needs an arena or an allocator that reserves
transient growth too, plus aggregate execution/host budgets. The current probe
makes no such claim. Thread parallelism multiplies per-execution limits; the
slice must also cap admitted concurrent memory and output.

## Cancellation and protected error propagation

Install the count hook before executing any user initializers, and retain one
fuel budget through initialization and `generate`, not a fresh budget per call.
The owner samples an atomic cancellation flag; another thread never calls Lua
or destroys its state. A count interval is a polling granularity, not a clock.
The probe uses interval 100 and terminates its infinite loop on the twentieth
hook. Async cancellation counts vary with scheduling and are not asserted as
a fixed latency.

Terminal budget/cancel state is a sticky host enum. The hook pushes a private
nonallocating error marker and calls `lua_error`; success must be rejected if
the terminal flag is set even if an inner operation reports success. Omit
script-level protected calls and coroutines rather than trying to make a
normal Lua error magically uncatchable. Do not depend on private
`luaD_throwbaselevel` internals. [U5, U6]

Compile the Lua translation units as **C**. Their normal protected error path
uses setjmp/longjmp; compiling Lua as C++ selects materially different internal
exception machinery. `extern "C"` changes linkage, not this cleanup contract.
An unprotected Lua error can reach panic/abort; a panic callback is not a
recovery design. [U5]

Use three layers:

```
C++ owner with RAII and fixed diagnostic storage
  -> lua_pcall(protected bootstrap/load/initialize/generate trampoline)
       -> trivial C/C++ Lua callback frame
            -> C++-only operation: owns temporaries; catches all native exceptions
            <- plain error code, after native destructors run
          lua_error only now, with no nontrivial callback locals alive
  <- Lua status + saved phase/loader status + sticky reason
copy bounded diagnostic; close state; translate to application result/exception
```

The probe intentionally throws in a native operation owning a vector and a
cleanup guard. That exception is caught before returning to the Lua-facing
callback. The callback then raises a Lua error. Both the inner native guard and
the outer owner guard are destroyed and `lua_close` returns tracked bytes to
zero. This proves the chosen boundary pattern on the tested compiler/runtime;
it does not prove that longjmp unwinds arbitrary C++ RAII frames—it does not.

All allocation-capable setup belongs under protection, including creating
library/context tables, pushing source strings and closures with upvalues.
`lua_newstate` has its own null-failure contract. The initial zero-upvalue C
function push fits the fresh state's guaranteed stack space; do not generalize
that exception to arbitrary setup. Check `lua_checkstack` before larger pushes.

**Observed classification trap:** `luaL_loadbufferx` returns syntax status 3,
but re-raising its error with `lua_error` makes the outer `lua_pcall` return
runtime status 2. Preserve `load_status` and phase independently. Both syntax
and binary-input rejection demonstrate this, with zero bytecode hooks.
Likewise preserve a loader OOM status rather than losing it to rethrow.

Diagnostics need no user formatting. Copy only an already-string error object,
with a fixed maximum; otherwise use the host error class and a constant message.
Do not call `tostring`, `luaL_tolstring`, or an unbounded traceback on the error
path. Get optional source/line through a bounded protected host handler or
recorded location; loss of a line on severe OOM must not compromise cleanup.
Identity comes from the trusted descriptor and a relative chunk name, never a
private filesystem path. The probe demonstrates a 96-byte copy of a much
larger message and an arbitrary-table error with no conversion.

At Orca integration, return a bounded `ScriptError` from the runtime and
translate outside Lua to the normal **SlicingError** channel. Never use
`InfillFailedException`: the code audit identifies catches that silently
consume it. The later slicing-visible failure test is not replaced by a unit
probe. [R4]

## Stack, recursion and cleanup lifetime

Lua's value stack and call information are dynamically allocated and therefore
charged to the Lua allocator. This is distinct from native C-stack bytes.
The inspected runtime defaults to one million Lua stack slots and 200 nested
C calls/syntactic recursion levels. The proposal lowers these to 32,768 and
100 respectively; these are candidate build ceilings needing worker-stack
validation, not experimentally selected universal safe values. [U5, U7, U8]

Non-tail Lua recursion can exhaust the heap/stack budget. Tail calls can run
without growing the same stack, so a recursion-only defense misses them.
C-to-Lua-to-C reentry consumes another resource again. Tests cover all three:
heap-bounded non-tail recursion, instruction-bounded tail recursion, and a
protected `C stack overflow` from deliberate C reentry on the installed Lua.
Windows Debug instrumentation and smaller TBB worker stacks must be tested.
The Lua C-call counter does not bound arbitrary recursion inside an exposed
native function; wrappers must not have uncontrolled native recursion.

One independent execution is **one `lua_newstate`, one initialization plus one
`generate`, and unconditional close on success or failure**. `lua_newthread`
shares global state and is not an independent sandbox. No VM, registry reference,
Lua string pointer, or closure may survive the execution. Reusing a poisoned
state is unnecessary; start fresh after every failure. Eight simultaneously
running fresh states each returned their own global counter value of one.
The final fresh success after the failure sequence also passed. [U3, U8]

```
Slice job
  owns immutable registry snapshot/source descriptors
  owns cancellation token and aggregate admission budget
  |
  +-- execute #A (one worker; no shared mutable Lua state)
  |     ExecutionStorage (must outlive lua_close)
  |       allocator + terminal status + counters + bounded diagnostic
  |       borrowed immutable source/context + cancellation token
  |       execution-private binding/output transaction
  |       Lua state owner (destroyed FIRST)
  |           protected setup -> text load -> init -> generate -> validation
  |           close -> verify allocator zero -> discard/publish output
  |
  +-- execute #B ... independent storage/state

Returned result: owning identity + copied diagnostic/statistics, never Lua data
```

Close can invoke finalizers and to-be-closed variables; automatic destruction
is not automatically bounded. No user-created finalizers/close methods are
permitted in this profile. Any host userdata cleanup is nonallocating,
nonthrowing, finite, and may not call Lua or block. Keep allocator, bindings and
source alive through close; release the output transaction afterward. [U8, U12]

## What the bytecode hook does not bound

A VM instruction is not a unit of bounded elapsed work. Native C execution,
parser/lexer work, GC—including emergency GC on denial—allocator internals,
table rehash/large string operations, and host output allocations can run
between hook opportunities. OS scheduling, locks and library calls add further
latency. Lua heap quotas do not account for native vectors or all C stack. [U4,
U6, U10, U12]

The probe's native loop performs 1,000 explicitly charged work units without
one intervening bytecode-hook tick, then fails at its own budget. Another run
sets cancellation at native iteration 31 and stops through its own polling.
These bounded positive controls show the gap without running an unbounded
native hang. Every exposed C function therefore needs its own maximum input
size, checked output-size arithmetic, native-work charge, polling granularity,
and exception/cleanup contract. Either omit operations such as pattern
matching/sort, or wrap and prove them separately.

Preflight source length before allocating a VM, cap parameter/context sizes,
use text-only loading (`mode="t"`), and check cancellation before and after
loading. A bounded reader helps between reader calls, but cannot preempt one
large token's parser work. A deadline checked by hooks and callbacks is a
cooperative safety guard, not a hard deadline. Hard stop against a hung C
function/native runtime defect requires process isolation, which this in-process
ABI/prepass does not implement. Do not terminate a slicing thread or promise
hard real-time cancellation.

The output budget remains independent: charge points/work before storing,
check finite/scalable coordinates, track vector capacity rather than just
size, and never allow partial output to become a successful fill. Orca buffers
the full path before clipping. Clipping/extrusion are outside the VM budget and
need their own slice-level accounting; PP-005 must expose counters, not pretend
the bytecode quota controls the later geometry pipeline. [R4]

## Actual observations and their scope

`results/reproduced/` is the canonical reproducible run. Each of GCC, Clang and
the instrumented-host Clang build ran **33 cases**, exit code 0. The sanitizer
build instruments the host wrapper only: the installed Lua shared object was
not rebuilt with sanitizers. No sanitizer diagnostics were emitted. Environment:
GCC 14.2.0, Clang 17.0.0, C++17, Debian 13.3 x86-64, glibc 2.41, CMake 3.31.6.
Exact commands, binary hashes, library hash/build ID and logs are retained.

Representative canonical GCC observations:

| Case | Observation |
|---|---|
| Success | Sum 1..100 = 5050; independent state closes. |
| Instruction exhaustion | Hook 20, host fuel reason, protected runtime status. |
| Allocation bomb | 65,536-byte cap; peak reserved 65,491 bytes; final LUA_ERRMEM; two denials; zero remaining bytes/blocks. |
| Native exception bridge | Inner guard = 1, outer guard = 1, state close = 1; host-error reason. |
| Native work | 1,000 charged iterations, unchanged bytecode-hook count. |
| Native cancellation | Stops at 31 iterations, unchanged bytecode-hook count. |
| Syntax/text-only loader | Native loader status 3 retained despite outer status 2; zero hooks. |
| C reentry | Protected `C stack overflow`, zero retained allocation. |
| Startup/fault injection | Null startup or protected failure; all tracked allocations released. |
| Parallel isolation | Eight independent counters all equal 1. |

The complete cases include allocator transitions, tail/non-tail recursion,
forbidden globals in the intentionally bare test environment, large/non-string
errors and fresh execution after failure. The globals test does **not** certify
a future richer allowlist. The host-work test does **not** audit every standard
C library function. The assertion suite does **not** implement geometry validation,
manifest ingestion, a registry, an actual `generate(ctx,out)` adapter, wall-time
latency limits, or a production sandbox. `ACCEPTANCE.md` makes the distinction
explicit, and `INTEGRATION.md` gives the file-level implementation plan.

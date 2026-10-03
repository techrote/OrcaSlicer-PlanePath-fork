# Test and CI strategy

## Rule zero

Characterize before refactoring. PP-002 lands behavioural fixtures before PP-003 replaces scattered pattern checks.

## Test layers

### 1. Native baseline / conformance

Preserve and extend upstream tests around:

- `tests/libslic3r/test_fill_plane_path.cpp`;
- `tests/fff_print/test_fill.cpp`;
- `tests/fff_print/test_printobject.cpp`.

Capture native Hilbert:

- raw point sequences for small domains/orders;
- resolution behaviour;
- smoothing on/off;
- start/end and bounds;
- rotated/aligned cases as applicable;
- dense-turn bridge/reconstruction behaviour.

### 2. Trait regression

Before and after PP-003, assert that every existing native pattern receives the same effective decisions for the audited capabilities.

Prefer table-driven tests over only testing Hilbert.

### 3. Runtime/sandbox

Positive:

- valid minimal script;
- math/table/string operations that are intentionally supported;
- parameter types;
- deterministic repetition.

Negative/adversarial:

- filesystem/process/network APIs absent;
- package/require escape unavailable;
- native module loading unavailable;
- bytecode/dynamic loader attacks blocked if applicable;
- infinite loop stopped by instruction budget;
- memory bomb stopped by allocator budget;
- huge point output stopped;
- recursion/stack abuse fails cleanly;
- NaN/Inf/overflow coordinate rejected;
- oversized diagnostics bounded;
- cancellation interrupts execution.

### 4. ABI/registry

Test:

- manifest schema;
- path traversal rejection;
- duplicate ID resolution;
- API incompatibility;
- canonical parameter serialization;
- reload snapshot semantics;
- missing ID/version/hash;
- same ID with changed content;
- malformed source;
- concurrent resolution/execution.

### 5. Hilbert parity

For a matrix of small/medium grid extents and resolution/smoothing conditions:

- native raw plane-path result;
- scripted Hilbert raw result;
- equality where exact equality is expected;
- otherwise explicitly documented normalization with tests proving equivalence.

Parity is the architectural gate before adding other production curves.

### 6. Solid-surface integration

Exercise scripted patterns on:

- rectangle;
- concave outline;
- polygon with hole;
- multiple disconnected islands;
- very small/sliver surfaces;
- rotations/origin modes;
- internal solid;
- top;
- bottom;
- smoothing;
- relevant bridge/reconstruction code;
- multiple scripted contexts in the same project.

Assertions should prefer geometry invariants and stable snapshots over fragile full-G-code byte equality unless exact output is intentionally contractual.

### 7. UI/config/project

Automate what is practical:

- native/scripted selection round-trip;
- independent top/bottom/internal choices;
- generated parameter controls;
- invalid parameter rejection;
- missing package display;
- project save/load identity;
- explicit rebind/upgrade;
- no auto-execution of newly embedded package content in the later trust milestone.

### 8. Performance

Build representative stress fixtures:

- high-order Hilbert/Peano;
- complex clipped surface;
- many small surfaces;
- parallel slices.

Track:

- total generated points;
- script runtime;
- clipping/runtime regression;
- memory peak where tooling permits.

Do not choose a hard performance threshold without baseline evidence. PP-016 records baseline and selects justified regression bounds.

## CI policy

Every implementation issue:

1. run focused tests locally/CI;
2. run the repository's required broader test/build gates;
3. open a PR with evidence;
4. repair failures instead of disabling/skipping checks;
5. merge only after all required automated checks are green;
6. verify the merge commit/result is on `main`;
7. close the issue only when acceptance criteria are actually satisfied.

If a check is flaky, diagnose or quarantine it through a separately justified change; do not rerun until green and call that proof.

## CI practicality

OrcaSlicer is large. Establish a fast focused unit-test job for PlanePath development, plus a broader gate appropriate to touched code. Do not make every small script change rebuild every platform if upstream CI architecture supports reliable caching/matrices, but a release milestone must prove the supported distributable platform(s), including the intended Windows custom build.

## Evidence expected in PR

- commands/check names run;
- relevant test counts/results;
- before/after behaviour where applicable;
- security negative-test evidence for sandbox work;
- screenshots only for UI changes, not as substitutes for automated tests;
- upstream base commit used.

## Additional mandatory regressions from the code audit

The following tests are required because current Orca internals make them easy to get subtly wrong:

- two surfaces on one layer using the same scripted sentinel but **different package IDs** must not batch together;
- same package with different canonical parameters must not batch together;
- native narrow-internal-solid substitution to `ipConcentricInternal` must clear scripted invocation state;
- bridge/helper-generated solids must continue following existing Orca effective-pattern rules rather than accidentally invoking the custom top/bottom/internal selection;
- scripted generation through the aligned/clipping branch must use the concrete `InfillPolylineClipper` semantics, not only the base output implementation;
- a script runtime/budget/coordinate failure must produce a propagated user-visible slicing error, never a successful slice with silently missing infill;
- preserve-source-order patterns must be checked after polygon clipping **and** at final extrusion-entity ordering/reversal boundaries;
- `FillParams` must remain trivially copyable;
- adding the sentinel must not break `Fill::use_bridge_flow()` initialization or any exhaustive `InfillPattern` switch;
- an in-progress multi-layer slice must use one registry content hash even while the UI publishes a reloaded registry snapshot;
- V1 solid scripts must not unexpectedly inherit `sparse_infill_smooth_factor`;
- serialization round trips must cover preset JSON, CLI validation, object overrides, standard 3MF and Orca/BBS 3MF.

Performance tests must account for the fact that `InfillPolylineOutput` buffers the complete generated point vector. Output-point quotas therefore need memory-growth assertions, not just Lua instruction/runtime assertions.

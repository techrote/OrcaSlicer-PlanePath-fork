# Target architecture

## Data flow

```text
preset / project
  -> native fill enum = scripted-plane-path sentinel
  -> companion script pattern ID + version/hash + params
  -> PatternRegistry resolves descriptor
  -> ScriptRuntime validates/loads package
  -> FillScriptedPlanePath invokes generate(ctx, out)
  -> bounded stream of plane-path points
  -> existing FillPlanePath clipping / smoothing / surface machinery
  -> normal Orca extrusion entities
  -> normal Orca G-code generation
```

The bold boundary is conceptual: **scripts stop at geometry generation**.

## Components

### 1. Pattern traits

Introduce one authoritative trait query for native and scripted patterns. A practical shape is:

```cpp
struct FillPatternTraits {
    bool plane_path;
    bool centered;
    bool self_crossing;
    bool separable;
    bool smoothable;
    bool supports_multiline;
    bool dense_bridge_sampling;
};
```

Exact naming may follow upstream conventions, but capabilities must stop being duplicated as ad-hoc `pattern == ipHilbertCurve || ...` tests.

Traits are behavioural contracts, not decorative metadata. Script-supplied traits are validated against what V1 supports.

### 2. Stable scripted sentinel

Add one compiled pattern type such as `ipScriptedPlanePath`. It maps to `FillScriptedPlanePath`.

Never create enum values dynamically and never make preset deserialization depend on installation order.

### 3. Companion script selection

Each solid-fill context needs its own selected custom pattern and parameters; a single global script ID is insufficient.

Planned companion config concepts:

- `top_surface_script_pattern`
- `top_surface_script_params`
- `bottom_surface_script_pattern`
- `bottom_surface_script_params`
- `internal_solid_infill_script_pattern`
- `internal_solid_infill_script_params`

Names can be adjusted once upstream naming/migration conventions are confirmed, but **the one-selection-per-context invariant is fixed**.

### 4. PatternRegistry

Responsibilities:

- discover built-in and user-installed pattern packages;
- validate manifest schema/API version;
- reject duplicate IDs deterministically;
- resolve ID + requested version/hash;
- expose UI metadata and parameter schema;
- make missing/incompatible patterns an explicit error;
- reload user packages on request without destabilizing an active slice.

Suggested locations:

- built-in resources distributed with the custom Orca build;
- user data directory dedicated to PlanePath scripts.

Do not scan arbitrary working directories or project-relative executable code automatically.

### 5. ScriptRuntime

Use an embedded, pinned Lua runtime through its C API.

Properties:

- no external interpreter dependency;
- one isolated state/context per concurrent execution or another design that proves thread isolation; never share a mutable Lua state unsafely across slicing workers;
- deterministic host-provided environment;
- no persisted native Lua bytecode as a portability mechanism;
- instruction, memory, output-point and wall-time/cancellation budgets;
- actionable diagnostics carrying pattern ID and script location/package identity.

### 6. FillScriptedPlanePath

This is a `FillPlanePath` adapter.

Responsibilities:

- resolve the already-selected package;
- translate `FillPlanePath::generate` inputs into ABI context;
- stream validated points into `InfillPolylineOutput`;
- surface script errors as slicing errors with context.

It must **not**:

- clip polygons itself;
- calculate extrusion;
- emit G-code;
- bypass existing smoothing/rotation/surface semantics.

## Concurrency and determinism

Orca slicing is concurrent. The implementation must assume simultaneous fill generation.

- Registry metadata may be shared if immutable/thread-safe.
- Script execution state must be isolated.
- User package reload must use snapshot semantics: an active slice sees one resolved package version for its entire operation.
- Randomness is absent by default. If a future API provides deterministic PRNG, its seed is host-controlled and serialized when relevant.

## Failure semantics

Reject, do not improvise:

- unknown pattern ID;
- API version mismatch;
- hash/version mismatch where strict identity is required;
- invalid manifest;
- invalid parameter type/range;
- NaN/Inf or out-of-contract coordinate;
- point/output budget exceeded;
- instruction/memory budget exceeded;
- sandbox violation;
- runtime error.

There is no automatic Rectilinear/Hilbert fallback. Silent geometry substitution is unacceptable in a slicer.

## Evolution

V1 ABI should be deliberately small and versioned. Additive host features can be negotiated by API version/capabilities. Do not expose all Orca internals "for flexibility"; that would permanently couple third-party scripts to unstable implementation details.

## Mandatory code-audit refinements

Read `11-CODE-AUDIT-2026-09-18.md` before implementing this architecture. The code audit makes the following refinements authoritative:

- the trait model must distinguish plane-path origin, center-of-surface support, inward/outward fill-order support, source-path ordering, reversibility, dense-turn sampling and bridge-flow semantics rather than relying on one broad `centered` concept;
- scripted invocation identity/fingerprint participates in `SurfaceFillParams` batching;
- `FillParams` remains trivially copyable and does not own strings/JSON/Lua objects;
- a slice owns an immutable registry snapshot; layer generation never resolves against a mutable live registry;
- `FillScriptedPlanePath` is configured with immutable resolved state before `centered()`, `no_sort()`, `is_self_crossing()` or `generate()` are called;
- preserve-source-order scripts must retain ordering after polygon clipping and through extrusion-entity planning;
- script failures propagate through a user-visible `SlicingError` path and must not use swallowed `InfillFailedException`;
- scripted output must go through a type-correct emitter that preserves the concrete `InfillPolylineClipper` path despite non-virtual `add_point()`;
- the incremental Lua `out:point()` API is not memory streaming: Orca buffers the complete `Points` path, so point quotas are direct memory-safety controls;
- V1 scripts operate in Orca's fill-aligned local grid coordinates after native rotation/origin selection, not world/printer XY.

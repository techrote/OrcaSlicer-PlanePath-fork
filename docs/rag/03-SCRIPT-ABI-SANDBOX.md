# Script ABI and sandbox

## Runtime choice

Plan: embed a pinned Lua release in the custom build. Lua is small, designed for embedding through a C API, and permits constructing a restricted standard-library environment.

Do not depend on system Lua, Python, Node, shell execution or environment PATH.

The implementation issue must record the exact vendored/dependency version and licence notices.

## Package layout

A package should have a manifest and source, for example:

```text
my-pattern/
  pattern.json
  main.lua
  README.md        # optional
```

Do not execute code merely to discover metadata. Manifest parsing/validation happens before Lua execution.

## Manifest v1

Required concepts:

- `id`: stable reverse-domain- or namespace-like identifier; ASCII-limited and length-bounded;
- `name`: display name;
- `version`: package semantic/version string;
- `api`: integer ABI version, initially 1;
- `entry`: package-local script path, normally `main.lua`;
- `traits`: requested supported pattern traits;
- `parameters`: declarative UI/validation schema.

Optional concepts:

- author/homepage/licence metadata;
- description;
- tags/category;
- built-in documentation path.

All manifest paths must remain inside the package root after canonicalization.

## Parameter schema v1

Keep it small:

- bounded integer;
- bounded finite float;
- boolean;
- enum string.

Each definition can provide default, label/description, and for numeric values min/max/step.

The host validates and canonicalizes parameters before starting Lua. The script receives typed values, not an arbitrary JSON parser surface.

## Geometry API v1

V1 is intentionally a **continuous single plane path**.

Conceptual Lua:

```lua
function generate(ctx, out)
    -- ctx.min_x / min_y / max_x / max_y
    -- ctx.resolution
    -- ctx.layer_index where stable/available
    -- ctx.params.<name>

    out:point(x, y)
    ...
end
```

The host validates every point before passing it onward.

V1 does **not** provide `break_path()`. Discontinuous address orders such as raw Morton/Z-order are deferred until a later design decides how travel moves, extrusion breaks and clipping should interact. Do not encode discontinuities as giant connecting segments.

## Coordinate contract

Scripts operate in the same abstract plane-path domain used by the adapter, not directly in machine/G-code coordinates.

Required guarantees:

- all coordinates finite and representable in Orca's coordinate type;
- a bounded guard margin around requested domain, if any, is documented and enforced;
- segment/point counts bounded;
- no zero-information infinite repetition;
- resolution is a geometric hint/constraint from the host, not permission to emit unbounded detail.

The Hilbert parity issue defines exact fixtures and therefore freezes the practical V1 coordinate interpretation.

## Sandbox

Do not expose:

- `io`;
- `os`;
- `package` / unrestricted `require`;
- `debug`;
- `dofile`, `loadfile` or arbitrary path loaders;
- FFI/native modules;
- sockets/network APIs;
- process/environment APIs;
- host object pointers or unsafe userdata.

Only expose selected safe base/math/table/string functionality needed for geometry, plus the narrow `ctx` and `out` interfaces.

If dynamic `load` is retained for legitimate code structure, it must not enable bytecode/native escape and needs explicit security tests; default preference is to omit it.

## Budgets and cancellation

Every invocation must be interruptible.

Required independent controls:

- instruction/hook budget;
- allocator-backed memory ceiling;
- maximum emitted point count;
- maximum diagnostic/error string size;
- host cancellation check;
- bounded recursion/stack behaviour through Lua/runtime limits where practical.

A wall-clock timeout may be a secondary guard but must not be the only guard because scheduling load makes it nondeterministic.

## Determinism

Scripts do not receive current time, environment data or ambient randomness.

If `math.random` cannot be made suitably deterministic/isolated, omit it in V1.

The same package bytes, parameters and geometry context must generate the same output sequence.

## Errors

Report at least:

- package ID/version/hash;
- source file and Lua line where available;
- error class (manifest, sandbox, budget, runtime, coordinate);
- concise message.

Never print full arbitrary project paths into telemetry/logs without following Orca's existing privacy conventions.

## Code-audit ABI clarifications

The current Orca path establishes the script coordinate frame before `generate()`: it rotates the surface into fill-direction coordinates, selects the relevant surface/model/assembly bounds, shifts by the pattern's origin semantics, and converts into a density/spacing-adjusted grid. ABI v1 coordinates are therefore **fill-aligned local grid coordinates**, not world/printer XY.

Top and bottom surface density may be below 100%, so scripts must not assume unit grid spacing means one nozzle-width or a fully dense surface.

The Lua `out:point()` interface is incremental only at the API boundary. Orca's `InfillPolylineOutput` buffers the complete `Points` vector before polygon intersection. Point quotas are therefore direct memory-safety limits.

Before forwarding a point into Orca:
- reject NaN/Inf;
- verify the scaled coordinate is representable by `coord_t`;
- enforce point quota;
- handle consecutive coordinates that snap to the same Orca point deterministically;
- on completion, reject a non-empty fill invocation with fewer than two usable points.

The output adapter must preserve Orca's concrete `InfillPolylineClipper` dispatch despite non-virtual `add_point()`; see `11-CODE-AUDIT-2026-09-18.md`.

Script failures must propagate through a user-visible slicing error path. Do not use `InfillFailedException`, which existing fill code catches and discards.

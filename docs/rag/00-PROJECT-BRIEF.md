# Project brief

## Problem

OrcaSlicer exposes a finite, compiled set of fill patterns. Its Hilbert Curve implementation already demonstrates a useful abstraction: a plane-path generator emits a path over a regular domain, then OrcaSlicer performs the printable-surface clipping and downstream toolpath work.

The project will turn that seam into a **safe, user-extensible scripted plane-path system** and ship a useful library of printable space-filling curves.

## Product objective

A user can install/select a custom plane-path script for internal solid fill, top surfaces, or bottom surfaces. The script emits bounded geometry through a narrow host API. OrcaSlicer remains authoritative for printable-region clipping, flow, line width, surface semantics, extrusion entities and final G-code.

The first end-to-end proof is strict: a scripted Hilbert implementation must reproduce the native Hilbert raw plane path over a defined fixture matrix and survive Orca's existing clipping pipeline without a scripting-specific downstream toolpath path.

## V1 scope

- Custom **solid-fill** plane paths:
  - internal solid fill;
  - top surface fill;
  - bottom surface fill.
- Embedded, sandboxed scripting runtime.
- Dynamic pattern registry with stable IDs and parameter schemas.
- Pattern traits sufficient to replace scattered Hilbert/Octagram special-case capability checks.
- Native/project serialization of selected script identity and parameters.
- Built-in scripted reference patterns:
  - Hilbert (golden parity implementation);
  - Peano;
  - Moore;
  - Wunderlich family;
  - later non-square/recursive candidates such as Sierpiński and Gosper if the geometry contract proves suitable.
- Deterministic, bounded execution with cancellation.
- UI selection, parameter editing, reload and actionable diagnostics.

## Explicit non-goals for V1

- Do not make scripts G-code postprocessors.
- Do not give scripts raw authority over extrusion amount, firmware dialect, acceleration commands or arbitrary G-code.
- Do not expose filesystem, network, subprocess, native-module or FFI access to pattern scripts.
- Do not initially generalize scripted patterns to sparse structural infill. Sparse density, bridge semantics and structural expectations require a later explicit design.
- Do not silently substitute another pattern when a required custom pattern is missing, incompatible or fails validation.
- Do not automatically execute source embedded in an untrusted project file.
- Do not require external Python or a separately installed runtime.

## Terminology

**Plane path** — an ordered geometric path generated over the fill domain before printable-surface clipping.

**Pattern package** — manifest + script source + optional documentation/assets, identified by stable pattern ID and version/hash.

**Scripted pattern** — a pattern implemented through the embedded runtime and host ABI.

**Native pattern** — a compiled OrcaSlicer pattern such as current Hilbert/Octagram implementations.

**Pattern traits** — declarative properties used by the slicer to select legal downstream behaviour; examples include centered origin, self-crossing, smoothability and dense bridge sampling.

## Core invariants

1. Orca geometry remains authoritative after script output.
2. Equivalent input + script version + parameters produces deterministic output.
3. Script failure is explicit; no silent fallback.
4. A script cannot escape its execution budget or sandbox by design.
5. Dynamic patterns do not require dynamic C++ enum values.
6. Existing native presets/projects continue to deserialize with unchanged meaning.
7. Native Hilbert behaviour is characterized before refactoring its surrounding special cases.
8. Every merged implementation issue is proven by automated checks and PR evidence.

# OrcaSlicer upstream seam map

## Research snapshot

The planning research inspected `OrcaSlicer/OrcaSlicer` code-search results on 2026-09-18. Returned source blobs were at commit `23c77f15cfa38696d89552344d887ce938ac39d4`. Upstream is active: PP-001 must record the actual imported/pinned upstream commit, and every later issue must adapt paths if upstream moves.

Primary upstream: `https://github.com/OrcaSlicer/OrcaSlicer`.

## Plane-path core

### `src/libslic3r/Fill/FillPlanePath.hpp`

Important current concepts:

- `InfillPolylineOutput`
  - accepts generated points;
  - scales/snap-rounds output;
  - returns the generated point path.
- `FillPlanePath : Fill`
  - owns `_fill_surface_single(...)`;
  - declares `centered()`;
  - declares the virtual `generate(min_x, min_y, max_x, max_y, resolution, output)` seam;
  - has a `FillParams` overload for generators that need fill parameters.
- Current subclasses include:
  - `FillHilbertCurve`;
  - `FillArchimedeanChords`;
  - `FillOctagramSpiral`.

The header explicitly records that the original implementation used generators from the Perl **Math::PlanePath** library. This is both architectural evidence and a strong research catalogue, not permission to copy GPL source indiscriminately.

### `src/libslic3r/Fill/FillPlanePath.cpp`

Contains:

- `FillPlanePath::_fill_surface_single`, the critical path between abstract plane-path generation and printable surface output;
- Hilbert coordinate generation helpers;
- `FillHilbertCurve::generate`;
- Archimedean/Octagram generators;
- integration with corner smoothing/clipping.

**Invariant:** scripted patterns should enter at the plane-path generation seam and then reuse this path. Do not create a separate scripted G-code/toolpath pipeline.

## Pattern enum and factory

### `src/libslic3r/PrintConfig.hpp`

Defines `enum InfillPattern`, including `ipHilbertCurve`, `ipArchimedeanChords` and `ipOctagramSpiral`.

Also contains pattern capability helpers such as `is_smoothable_infill_pattern(...)` and configuration fields such as:

- `sparse_infill_pattern`;
- `top_surface_pattern`;
- `bottom_surface_pattern`;
- `internal_solid_infill_pattern`.

### `src/libslic3r/Fill/FillBase.cpp`

The compiled factory maps `ipHilbertCurve -> FillHilbertCurve`, etc.

**Implication:** dynamically assigning one C++ enum value per installed script is the wrong model. The plan adds one stable sentinel/native adapter type and carries dynamic script identity separately.

## Fill selection / surface context

### `src/libslic3r/Fill/Fill.cpp`

Current logic chooses different configured patterns for top, bottom, internal solid and sparse surfaces, then builds `FillParams`.

It also consults helpers such as `is_smoothable_infill_pattern(...)`.

This file is a key integration point for passing selected scripted-pattern identity/parameters into the filler without contaminating unrelated pattern types.

## Scattered behavioural knowledge

### `src/libslic3r/PrintObject.cpp`

Current code contains Hilbert/Octagram-specific logic for "turning" patterns, including denser sampling/reconstruction of bridge anchors so local turns are respected.

### `src/slic3r/GUI/ConfigManipulation.cpp`

Contains pattern-specific enablement/capability logic, including multiline and smoothing decisions.

### Other GUI/config consumers

Search before editing:

- `src/slic3r/GUI/GUI.cpp`;
- `src/slic3r/GUI/Field.cpp`;
- `src/slic3r/GUI/ConfigValueFormatter.cpp`;
- `src/slic3r/GUI/GUI_Factories.cpp`;
- `src/libslic3r/Preset.cpp`;
- any current config migration/3MF serialization code touching infill enums.

**Implication:** PP-003 centralizes traits only after PP-002 has golden behaviour fixtures.

## Existing tests to preserve/extend

Research found relevant coverage in:

- `tests/libslic3r/test_fill_plane_path.cpp`
  - Hilbert generation;
  - smoothing;
  - resolution;
  - corner curvature.
- `tests/fff_print/test_fill.cpp`
  - fill-pattern behaviour and rotation/smoothing coverage.
- `tests/fff_print/test_printobject.cpp`
  - Hilbert/Octagram bridge/turning behaviour.

Do not replace existing native Hilbert tests with scripted tests. The native implementation is the parity oracle until the project intentionally decides otherwise.

## Upstream-refresh checklist

At the start of every implementation issue that touches Orca source:

1. record current upstream/base commit;
2. search for all references to the relevant enum/pattern/helper, not only the paths listed here;
3. compare current behaviour with this map;
4. update this document in the same PR if paths or semantics materially moved;
5. avoid restoring obsolete code merely to match this planning snapshot.


## PP-001 import verification

PP-001 imported OrcaSlicer commit `23c77f15cfa38696d89552344d887ce938ac39d4` as a real second parent of merge commit `316aa44382bb04a0b368cd565435c1912f8b1951`. This exactly matches the source commit used for the planning/code-audit snapshot, so no seam-map semantic drift was found during bootstrap.

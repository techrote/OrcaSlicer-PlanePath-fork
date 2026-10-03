# PP-002 native PlanePath conformance map

Base repository commit: `71679a3ab6a339cca63efc9992767ff4293f0e7c`  
Pinned OrcaSlicer ancestor: `23c77f15cfa38696d89552344d887ce938ac39d4`

PP-002 is deliberately a characterization milestone. It does not change slicer production behaviour. The tests below freeze the native behaviour that PP-003 and PP-004 are expected to refactor without semantic drift.

## Generator contract

`tests/libslic3r/test_fill_plane_path.cpp` records:

- exact raw Hilbert point sequences for order-1 and order-2 domains;
- translated and rectangular requested extents, including the current power-of-two square expansion;
- order-1/order-2/order-3 point count, bounds, uniqueness, endpoints and unit-step continuity;
- the existing raw-path fact that resolution does not alter unsmoothed Hilbert coordinates;
- smoothing endpoint preservation, zero-length suppression, curvature and resolution behaviour.

## PlanePath capability decisions

The focused tests freeze the current pre-trait-refactor decisions:

| Behaviour | Hilbert | Archimedean chords | Octagram spiral |
| --- | --- | --- | --- |
| Fill-bounds origin via `FillPlanePath::centered()` | corner/min | centered | centered |
| `is_separable_infill_pattern` | yes | yes | yes |
| sparse smooth-factor support | yes | no | yes |
| bridge-flow override | no | no | no |
| self-crossing declaration | no | no | no |
| filler-level `no_sort()` | no | no | no |

The same test also keeps `FillParams` trivially copyable.

## Clipping and ordering

A dedicated output-mode probe verifies that sparse `erInternalInfill` PlanePath generation selects the clipping output path while solid generation does not. Existing sparse-anchor parity coverage then compares generated anchoring polylines against the final printed sparse PlanePath geometry through rotations, smoothing, multiline and separated-body origins.

Final extrusion semantics are characterized separately:

- native Hilbert with default ordering remains in a sortable collection and its paths remain reversible;
- the existing top/bottom center-pattern tests retain their checks that explicit inward/outward ordering sets `no_sort` and disables reversal.

This intentionally records that native PlanePath source order is not generally a final-toolpath ordering guarantee.

## Surface grouping and effective pattern

FFF integration coverage records two `group_fills()` invariants before scripted identity is introduced:

- two regions using different compiled PlanePath patterns on the same print retain different generated solid-fill shapes, exercising `SurfaceFillParams.pattern` as part of the batching identity;
- narrow internal-solid detection makes the final narrow-fill result independent of whether Hilbert or Octagram was configured, while the non-substituted results remain distinct. This freezes the current native-substitution behaviour that PP-004 must later clear scripted invocation state for.

## Solid smoothing scope

A full solid Hilbert print is sliced with sparse smoothing set to both 0% and 100%. Top, bottom and internal-solid path counts, point counts and ordered-coordinate digest must remain identical. This records that `sparse_infill_smooth_factor` is a sparse-only control and is not an existing solid PlanePath feature.

## Bridge and reconstruction behaviour

The focused `[PlanePathConformance]` integration set includes the existing turning-pattern bridge regressions for Hilbert and Octagram:

- lower-layer/template/model-rotation bridge angle selection;
- preservation of another region's real anchors;
- rounded/high-turn bridge endpoints remaining on printed support.

An additional check varies top, bottom and internal-solid PlanePath selections while holding bridge inputs fixed and requires identical internal-bridge extrusion. This freezes the current rule that bridge effective fill is not replaced by the configured solid PlanePath.

These tests guard the externally visible contract around the high-turn sampling/reconstruction code in `PrintObject.cpp`: selected bridge direction and reconstructed/cleaned bridge ends must remain anchored to actually printed support. They deliberately avoid exposing private reconstruction internals as test API.

## Focused CI entry point

`scripts/planepath/run-tests.sh` now runs both:

1. `libslic3r_tests '[FillPlanePath]'`; and
2. `fff_print_tests '[PlanePathConformance]'`.

The complete upstream unit suite still runs immediately afterward in the normal PlanePath CI job. No correctness gate is removed or weakened.

# PP-003 native fill-pattern trait reconciliation

Implementation baseline: `3d43296f35ca99a040d74415f24647acd778ad8b`

PP-003 centralizes compiled native fill-pattern capabilities without adding a scripted pattern, Lua runtime state, registry, new configuration field, or serialization value. The two archived preparation branches were reviewed before implementation:

- `checkpoint/prepass-PP-003-19f12bbc8cfc`
- `checkpoint/prepass-PP-003-2921c3fc3227`

Their retained research describes 31 compiled native patterns, 18 caller decisions, 13 explicit exceptions and 267,840 context cases. Those packet results remain research evidence rather than production acceptance; this implementation rechecks the relevant claims against the live source and native test factory.

## Production trait model

`src/libslic3r/Fill/FillPatternTraits.hpp/.cpp` provides a checked `native_fill_pattern_traits(InfillPattern)` lookup. Invalid values, including `ipCount`, are rejected rather than indexing a cache.

The descriptor keeps independent semantics for:

- PlanePath origin: not applicable, bounding-box minimum, or centered;
- PlanePath conditional source-order handling;
- per-body/separated-infill eligibility;
- surface-centering controls;
- top/bottom `SurfaceFillOrder` capability;
- declared self-crossing;
- filler collection `no_sort`;
- layer-level reversal prohibition;
- sparse multiline capability;
- smoothing policy: never, always, or multiline-only;
- native pattern bridge-flow override;
- standard versus turning native bridge-anchor/reconstruction policy.

Context remains outside the descriptor where it belongs: surface role, density, rotation-template overrides, physical bridge state, effective-pattern substitutions and requested inward/outward direction.

## Reconciled callers

PP-003 routes the legacy separability and smoothing helpers through the descriptor and migrates the duplicated policy decisions in:

- `FillPlanePath.cpp`: Hilbert minimum-origin and Archimedean/Octagram centered-origin decisions;
- `Fill.cpp`: surface-order capability, model/body origin eligibility, Grid reversal policy and checked sparse-anchor native domain;
- `PrintObject.cpp`: turning bridge-anchor policy;
- `ConfigManipulation.cpp`: multiline, surface-centering and surface-order GUI capability lists;
- `FillBase.cpp`: static pattern bridge-flow lookup.

The filler virtuals remain as compatibility/intrinsic hooks. The all-native trait test compares their current `no_sort`, self-crossing and bridge-flow results with the descriptor for every factory target.

## Behavior intentionally preserved

The neutral refactor does not reset reused `SurfaceFillParams` fields. Integration tests characterize the existing fill-order and smoothing carry-over so a future bug fix cannot hide inside PP-003.

The following existing behavior is also preserved:

- Hilbert uses the bounding-box minimum origin while retaining the hidden backend center-mode path;
- Archimedean Chords and Octagram Spiral are centered and expose surface-centering controls;
- QuarterCubic remains separable;
- non-empty sparse/solid rotation templates remain independent exceptions to normal separability;
- multiline support remains a GUI capability rather than a new backend clamp;
- Concentric and SpiralInset keep their distinct default ordering behavior;
- low-density PlanePath `connect_infill()` does not gain source-path restoration;
- default Hilbert remains sortable and reversible;
- Grid alone retains the layer-level `can_reverse = false` policy;
- turning bridge reconstruction remains Hilbert/Octagram only, excluding Archimedean Chords;
- all audited native pattern-specific bridge-flow overrides remain false, while physical bridge flow remains contextual;
- narrow-internal-solid and bridge/helper effective-pattern substitutions remain unchanged;
- CrossHatch and QuarterCubic continue through sparse-anchor generation despite their historical omission from the old switch cases.

## Whole-checkout audit

`scripts/planepath/scan-native-pattern-uses.py` scans every tracked C/C++/Objective-C source file in an actual checkout. It derives the native enum names from `PrintConfig.hpp` and records every reference, every switch containing native cases and every source line containing multiple native pattern names.

PlanePath CI runs the scanner and uploads `pp003-native-pattern-audit.json`. The inventory is review evidence, not an automatic instruction to replace every switch: factory dispatch, geometry heuristics, effective-pattern substitution and other contextual decisions remain justified native special cases.

## Acceptance gates

Production acceptance requires the complete 31-pattern trait/factory equivalence test, invalid-enum rejection, focused PP-002 PlanePath conformance tests, the mixed-context carry-over regressions, turning bridge regressions, full repository unit tests, the whole-checkout audit, and platform build validation. CI results and final merge evidence are recorded on the PP-003 pull request.

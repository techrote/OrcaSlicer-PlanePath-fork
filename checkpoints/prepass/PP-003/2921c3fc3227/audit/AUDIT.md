# PP-003 source-analysis prepass

Assessed fork: `techrote/OrcaSlicer-PlanePath-fork`, migrated issue **#6**. Assessed `main`: `e71fec82f43285567debc4d2b05d51fb47c0820b`. Current synchronized Orca base is `00bb4202fe50ca625e59c6fcbec573a2887d22ed`, not the September planning base `23c77f15…`. The migrated source is the authority. This is a local research/prototype packet, not PP-003 implementation acceptance.

## Findings that determine the design

**Use a small native trait record plus explicitly contextual caller policy. Do not introduce one general “plane-path-like” flag.** Origin, body re-centering, GUI surface centering, surface order, kernel traversal, final collection ordering and reversal have different native memberships and gates. The complete 31-pattern table is `native_patterns.csv`/`.json`; every column maps to symbols through `column_provenance.json` and the 18-row `caller_decisions.json`. Source paths, assessed ref, read ranges and reported blob identities are in `../evidence/sources.json`. Line ranges are inspected windows, not claims of exact function start/end in every entry.

Most importantly, two apparent cleanups would change native behavior. First, `group_fills()` reuses one `SurfaceFillParams` outside both loops and only conditionally assigns `fill_order` and `smooth_factor`. A top Archimedean surface set Outward can leave Outward on the next Hilbert surface; sparse Hilbert smoothing can remain on a later solid Hilbert. The normal policy is sparse-only smoothing, but the actual implementation is stateful. The homogeneous all-solid PP-002 fixture never sets the sparse value, so it does not refute this. Second, the sparse-anchor switch does not list CrossHatch or QuarterCubic and has no default: both continue after the switch and generate. A whitelist generated from just its cases would omit two native patterns.

### Native memberships and nonuniform behavior

PlanePath origin: Hilbert uses the chosen bounds' minimum; Archimedean and Octagram use their center. Every other pattern is **not applicable**, not “uncentered.” All three consume center mode in the PlanePath backend. Only Archimedean/Octagram expose that mode in the GUI and participate in external per-model body selection. Hilbert can therefore be affected by a retained/configured center option that its own UI does not expose.

Separable membership is the explicit 15-entry native switch, not inheritance or self-crossing. Line and the Monotonic fillers are not included; QuarterCubic is. Nonexternal separated-body selection additionally accepts either rotation template, bypassing the native membership test. This is not confined to sparse extrusion: internal solids and bridges also satisfy “not external.” Each body is selected by greatest positive overlap, ties retaining the earlier body, and a miss leaves the original bounds. Anchors and final infill call the same helper.

The 22-member multiline list is a GUI capability. Unsupported configurations reset to one only in the GUI's `have_infill` branch. `group_fills()` itself passes configured multiline to every sparse-role fill, and sets one for nonsparse roles. PlanePath both multiplies grid spacing and applies multiline offsets before polygon clipping; the snug bounds are expanded by spacing times multiline. Do not add backend clamping as part of moving the GUI predicate.

Smoothing has three states, not a bool: Always for Hilbert, Octagram, Lightning, Honeycomb, 3DHoneycomb, Concentric and CrossHatch; MultilineOnly for Grid, Triangles, Stars and Cubic; Never otherwise. In particular Archimedean, SpiralInset and adaptive cubic are not smoothable via this helper. The GUI smoothing visibility does not additionally test sparse density. Caller assignment normally uses sparse role and `0.01 * sparse_infill_smooth_factor`; preserve the state exception until separately adjudicated. Keep generator sanitization, resolution handling and concrete emitter dispatch unchanged.

### Ordering is not one trait

Top/bottom order controls and assignments apply to Concentric, SpiralInset, Archimedean and Octagram. Role comes after effective-pattern substitution, and only actual top/bottom roles assign those options. Nonexternal helper/bridge fill must retain its existing effective-pattern rule. Narrow internal solids still substitute ConcentricInternal; future script invocation state must be cleared on both all-narrow and split branches, but PP-003 need not introduce that state.

PlanePath restores clipped fragments to original path order only in its `dont_connect() || density > 0.5` branch, and only for nondefault order. Inward reverses both fragment order and each fragment. At density exactly 0.5 with connecting enabled, it takes `connect_infill()` instead; final explicit-order guards still suppress later sorting/reversal. Do not assert unconditional source preservation from the presence of `fill_order`.

Default Hilbert is intentionally sortable and reversible. All three native PlanePath fillers return `no_sort=false`. Concentric and SpiralInset return true, as do Monotonic, the factory's plural `FillMonotonicLines`, and ConcentricInternal. This is collection-level behavior, not proof that raw point order survives clipping. Grid alone sets `params.can_reverse=false` in `Layer::make_fills`; its collection is not intrinsically unsortable. Self-crossing declarations are true for Grid, Triangles, Stars, Cubic and the shared Adaptive/SupportCubic filler, false for the others; do not “correct” them by geometric intuition during this refactor.

Concentric classic generates outside-in and reverses loop list only for Outward. Its Arachne branch stable-sorts by inset depth for explicit order and otherwise calls shortest-traverse reordering. SpiralInset reverses both each spiral and the output list whenever order is **not Inward**, so Default is outward too. A shared capability does not imply a shared default direction or ordering algorithm.

Calibration has two distinct gates. PlanePath's special geometry is top + Archimedean + Default: print chained chords then the longest central spiral oriented outward. FillBase's final protection is any top pattern with the flag, without the Archimedean/Default restrictions. FillBase combines virtual `no_sort`, calibration and nondefault order, then protects the initially appended paths from reversal. It appends gap fill afterward. ConcentricInternal and LockedZag override extrusion emission; the general FillBase formula does not describe their complete results. The ConcentricInternal implementation independently shortest-traverse reorders variable-width lines and sets its collection no_sort. LockedZag's full custom emission body remains an implementation verification target, not silently included in a base-class equivalence claim.

### Bridge flow and bridge reconstruction are separate

All 31 native factory targets resolve `use_bridge_flow()` to false. The explicit overrides in 3DHoneycomb, Gyroid, TpmsD and TpmsFK also return false, despite misleading “require bridge flow” comments in some headers. The physical flow decision is still `(layer.id() > 0 && surface.is_bridge()) || pattern_override`. Thickness policy separately uses the raw surface bridge/internal classification and object settings.

The static overload currently eagerly initializes a vector by constructing every enum value in `[0, ipCount)`, calling its virtual method and deleting the filler. A global initializer calls this before ordinary use. A new enum sentinel without a factory case can therefore throw during static initialization, not only when sliced. The raw-pointer loop also lacks exception-safe ownership if future methods throw. The vector is mutated after a `cached.empty()` check rather than initialized as a complete immutable value; do not treat the local-static declaration alone as proof that the fill loop is protected against every reentrant/concurrent initialization path. Existing eager initialization attempts to avoid the normal first-call race. Invalid indices are unchecked. A per-enum cache cannot express per-package/per-invocation capabilities.

Replace the static query with a checked native descriptor lookup and remove eager construction. Preserve every native false result; retaining the virtual query temporarily for direct callers is safe only with tests checking it against the descriptor for every factory target. Reject invalid enum values rather than reproduce undefined indexing. Keep factory dispatch explicit: it is implementation selection, not a capability switch. Do not add or construct a scripted sentinel in PP-003.

`PrintObject::bridge_over_infill()` has an independent native Turning policy for **Hilbert/Octagram sparse support configuration**, not Archimedean and not the final bridge fill. It couples three behaviors: prefer lower-layer configured direction when actual anchors and same-region sparse overlap exist; use `max(1, physical_spacing/4)` boundary scan spacing; restore contacts after opening/closing. Standard patterns use physical spacing for scans and no turning-specific repair. Model rotation and angle normalization precede user absolute/relative bridge overrides. Collision reconstruction must reuse the same scan spacing. Shared anchors remain real lower-layer geometry, never a fabricated grid. Sampling does not change extrusion flow or density.

### Sentinel failure points and switch coverage

The reviewed five-switch inventory records actual behavior, including defaults and absent cases, in `switch_inventory.json`. Factory dispatch covers all native values; the anchor switch is not actually exhaustive. Layer's max-void-area switch assigns factors 1, 4 or 4.5, with default 1 and density-zero early return -1. A sentinel would get an unproved geometry estimate from that default. Separability and smoothing switches would quietly return false; GUI lists would omit controls; no_sort/reversal defaults would not preserve arbitrary script ancestry; origin requires an actual filler implementation. Enum name maps and allowed config lists also need later sentinel migration, not just these switches.

**Global-search limit:** fork code search returned `incomplete_results=true` with zero results, and bulk checkout was unavailable. Therefore this packet does not certify that no other exhaustive switch or indirect caller exists. Candidate paths found in indexed upstream search were read from the assessed fork before being used as evidence. The included read-only full-checkout scanner emits every direct enum-case switch and references; its self-test passed, but its real full-tree mode was not run here. This is an explicit remaining verification gate, not an inferred clean search.

## Proposed minimal interface

The compiled standalone proposal is `../prototype/FillPatternTraits.hpp`. Its 11 fields occupy 11 bytes on this compiler (see measured result; do not make ABI size contractual): origin enum; separability; surface-center-control capability; surface-order capability; declared self-crossing; declared collection no_sort; make_fills reversal prohibition; tri-state smoothing; sparse multiline UI capability; pattern bridge-flow override; and native bridge-anchor policy enum. No strings, ownership, Lua state or package identity enter FillParams. The record separates policy-family declarations from context. The last word on context remains the audited caller, including previous grouping state where relevant.

`native_fill_pattern_traits(InfillPattern)` returns an immutable checked descriptor. Keep `is_separable_infill_pattern` and `is_smoothable_infill_pattern` as thin compatibility wrappers initially so callers do not all change at once. Production storage belongs in `src/libslic3r/Fill/FillPatternTraits.hpp/.cpp`; forward-declare `enum InfillPattern : int` in the header to avoid a PrintConfig include cycle, and include PrintConfig in the implementation. Add an explicit count/coverage assertion and tests for each named enum, not just array length. Do not rely on an enum reorder preserving an anonymous row's identity.

Existing instance virtuals can remain through the first migration with a native factory-vs-table regression over all 31 values. For no_sort and self-crossing, replace existing overrides with descriptor-based expressions in their **existing declaring classes**; inherited defaults remain supported by the all-native regression. PlanePath concrete centered overrides can forward to each concrete pattern's origin entry. No extra pattern member is required in Fill, avoiding uninitialized identity in direct constructors and test subclasses. Adaptive/SupportCubic share the same relevant class traits today; tests must flag a future split requiring separate identity. Do not make an abstract/test PlanePath subclass depend on an assumed factory enum.

Do not add a redundant all-false `preserves_source_order` flag to native traits. For a future adapter, define a separate resolved path-order requirement (e.g. `Optimize` versus `PreserveSourceDirection`) and carry it through clipping, final entity flags and planner boundaries. This is a future extension contract, not a reason to change Hilbert defaults or start #8. Likewise TurningNative is intentionally one current policy family; a future arbitrary curve may need independent direction/sampling/repair declarations. That split requires evidence then, not a Lua-runtime decision now.

## Validation level

`../scripts/run_all.py` compiles a dependency-free C++17 descriptor and independently extracted source predicates, compares the machine table, runs 267,840 procedural context combinations, records state carry-over, rejects six mutated trait decisions and checks invalid enum rejection. The integer Hilbert transcription is checked against five literal PP-002 goldens plus three Hamiltonian domains. Additional boundary checks cover first-layer bridge exclusion and integer scan-spacing floor/clamp. Compiler/runtime IDs and all outputs are retained.

These are **decision/probe executions**, not linked native factory execution, polygon clipping, full Orca unit tests or actual slicing. PP-002 files and tags were inspected, not rerun. The main-SHA check snapshot showed only Shellcheck success and a skipped daily job, so this audit does not certify the required native conformance gate green on main. Full implementation acceptance still needs the native checks in `GEOMETRY_AND_REFACTOR.md`.

## Complete native matrix

S = separable; C = surface-center controls; O = top/bottom order; X = declared self-crossing; N = collection no_sort; R = make_fills prohibits reversal; M = sparse multiline UI. Smoothing A=Always, >1=MultilineOnly, -=Never. All native pattern bridge-flow overrides are false. Turning applies only to Hilbert/Octagram. Read the context exceptions above before interpreting a bit as effective behavior.

| Pattern | Origin | S | C | O | X | N | R | Smooth | M |
|---|---|---|---|---|---|---|---|---|---|
| ipMonotonic | NotPlanePath | - | - | - | - | Y | - | - | - |
| ipMonotonicLine | NotPlanePath | - | - | - | - | Y | - | - | - |
| ipRectilinear | NotPlanePath | Y | - | - | - | - | - | - | Y |
| ipAlignedRectilinear | NotPlanePath | Y | - | - | - | - | - | - | Y |
| ipZigZag | NotPlanePath | Y | - | - | - | - | - | - | - |
| ipCrossZag | NotPlanePath | Y | - | - | - | - | - | - | - |
| ipLockedZag | NotPlanePath | Y | - | - | - | - | - | - | - |
| ipLine | NotPlanePath | - | - | - | - | - | - | - | - |
| ipGrid | NotPlanePath | Y | - | - | Y | - | Y | >1 | Y |
| ipTriangles | NotPlanePath | Y | - | - | Y | - | - | >1 | Y |
| ipStars | NotPlanePath | Y | - | - | Y | - | - | >1 | Y |
| ipCubic | NotPlanePath | Y | - | - | Y | - | - | >1 | Y |
| ipAdaptiveCubic | NotPlanePath | - | - | - | Y | - | - | - | Y |
| ipQuarterCubic | NotPlanePath | Y | - | - | - | - | - | - | Y |
| ipSupportCubic | NotPlanePath | - | - | - | Y | - | - | - | Y |
| ipLightning | NotPlanePath | - | - | - | - | - | - | A | Y |
| ipHoneycomb | NotPlanePath | - | - | - | - | - | - | A | Y |
| ip3DHoneycomb | NotPlanePath | - | - | - | - | - | - | A | Y |
| ipLateralHoneycomb | NotPlanePath | Y | - | - | - | - | - | - | Y |
| ipLateralLattice | NotPlanePath | Y | - | - | - | - | - | - | Y |
| ipCrossHatch | NotPlanePath | - | - | - | - | - | - | A | Y |
| ipTpmsD | NotPlanePath | - | - | - | - | - | - | - | Y |
| ipTpmsFK | NotPlanePath | - | - | - | - | - | - | - | Y |
| ipGyroid | NotPlanePath | - | - | - | - | - | - | - | Y |
| ipConcentric | NotPlanePath | - | - | Y | - | Y | - | A | Y |
| ipSpiralInset | NotPlanePath | - | - | Y | - | Y | - | - | - |
| ipHilbertCurve | Minimum | Y | - | - | - | - | - | A | Y |
| ipArchimedeanChords | Center | Y | Y | Y | - | - | - | - | Y |
| ipOctagramSpiral | Center | Y | Y | Y | - | - | - | A | Y |
| ipSupportBase | NotPlanePath | - | - | - | - | - | - | - | - |
| ipConcentricInternal | NotPlanePath | - | - | - | - | Y | - | - | - |
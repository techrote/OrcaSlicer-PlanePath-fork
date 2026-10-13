# Native surface-fill batching and contextual parameters

`group_fills()` in `src/libslic3r/Fill/Fill.cpp` constructs a `SurfaceFillParams`
key for each non-void surface. The `std::set<SurfaceFillParams>` comparator and
equality operator group surfaces that have the same effective fill invocation,
including geometry, flow, pattern, role and ordering. The scratch instance is
reused during traversal for allocation efficiency, but **no value determined by
a previous surface may decide the next surface's effective behaviour**.

## Per-surface contextual defaults

Before computing the effective pattern or role for each non-void surface:

- `fill_order` starts at `SurfaceFillOrder::Default`. Only a pattern with
  `native_fill_pattern_traits(pattern).surface_fill_order` and a top- or
  bottom-solid extrusion role may select its region's corresponding explicit
  top/bottom order. Hilbert and unsupported native patterns must never inherit
  inward/outward ordering from an earlier region or surface.
- `smooth_factor` starts at zero. Only a sparse `erInternalInfill` role whose
  effective native pattern supports smoothing at the effective multiline count
  may read `sparse_infill_smooth_factor`. Solid top, bottom and internal fills
  remain unsmoothed by this sparse-only option.

These values are part of the batching key and are forwarded to `FillParams`.
A stale key can both change emitted geometry/order and merge or split otherwise
identical fill groups incorrectly. Reinitializing at the surface boundary is
necessary even when no suitable pattern/role later assigns an override. The
collapsed-void fallback can synthesize a new internal-solid fill after normal
surface traversal; because that path reuses the same scratch object, it invokes
the same contextual reset before constructing the fallback fill.

Other existing effective-pattern substitutions, rotation and centering rules,
flow selection, per-pattern traits, serialization and factory dispatch are
unchanged. In particular, native Hilbert remains ordinarily sortable/reversible
and an explicitly ordered Archimedean top/bottom fill retains its requested
source-path order.

## Regression invariants

The native conformance tests exercise both pattern/region orders for
Archimedean versus unsupported Hilbert on top and bottom surfaces, with default
and outward ordering. They also compare identical Archimedean patterns in
different regions with distinct effective order settings, ensuring the batching
key does not conflate their output policies. Mixed sparse/solid native Hilbert
tests reverse the region roles, verify that sparse smoothing really changes the
sparse path and require byte-exact geometric/order identity for every solid path
when sparse smoothing changes.

The historical PP-003 trait-refactor characterization intentionally recorded
the previous carry-over behaviour without changing it; it is not the desired
behavioural contract for subsequent development.

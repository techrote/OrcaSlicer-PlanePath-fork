# Revised implementation roadmap

## Dependency graph

```text
PP-001 Bootstrap upstream fork / CI baseline
  |\
  | +--> PP-005 Sandboxed Lua runtime
  v
PP-002 Native plane-path conformance harness
  v
PP-003 Centralize pattern traits
  v
PP-004 Scripted sentinel + per-context config/serialization
  \                         /
   +----> PP-006 ABI + registry <---- PP-005
                     v
             PP-007 FillScriptedPlanePath
                     v
             PP-008 Scripted Hilbert parity
                     v
             PP-009 Solid-surface integration hardening
                /       |       \
               v        v        v
          PP-010 UI  PP-011     PP-014 project identity
                       curve pack
                       |   \
                       v    v
                    PP-012 PP-013
                           non-square curves

PP-010 + PP-014 -> PP-015 optional embedded-package trust
PP-009 + PP-011 + PP-013 + PP-015 -> PP-016 hardening/performance
PP-016 -> PP-017 release packaging/docs
```

## Issue catalogue

### PP-001 — Bootstrap the OrcaSlicer fork and provenance/CI baseline

Import/fork current upstream into this repository without losing the RAG corpus, record exact upstream commit/remote strategy, prove a clean baseline build/test path, and create focused PlanePath CI scaffolding.

### PP-002 — Characterize native PlanePath/Hilbert behaviour

Add golden/unit/integration fixtures before refactoring. Cover raw Hilbert points, smoothing/resolution and existing turning/bridge decisions.

### PP-003 — Centralize fill-pattern traits without behaviour change

Replace audited scattered concrete-pattern capability checks with one trait source. Golden fixtures must prove equivalence.

### PP-004 — Add scripted sentinel and per-context config identity

Add stable adapter enum/type and companion pattern ID/params for top, bottom and internal solid. Add preset/project serialization round trips without executing scripts.

### PP-005 — Embed the deterministic sandboxed Lua runtime

Vend/pin runtime, implement restricted environment, budgets, cancellation, deterministic execution and adversarial tests. This may proceed after PP-001 in parallel with PP-002–004 if branch coordination allows.

### PP-006 — Define Pattern Package ABI v1 and registry

Manifest parser/schema, parameter validation, built-in/user discovery, duplicate handling, immutable resolution snapshots, version/hash identity, reload, diagnostics.

### PP-007 — Implement `FillScriptedPlanePath`

Bridge resolved scripts into existing `FillPlanePath` generation/output only. No separate clipping, extrusion or G-code path.

### PP-008 — Ship scripted Hilbert as the parity gate

Implement Hilbert package and prove raw-path parity across fixture matrix. No production curve expansion until this gate passes.

### PP-009 — Harden scripted solid-surface integration

Prove internal solid/top/bottom, holes, concavity, islands, rotations, smoothing, bridge/turn sampling and simultaneous different patterns.

### PP-010 — Dynamic pattern UI and parameter editor

Show registry patterns in relevant selectors, generate controls from schema, preserve independent context selections, add reload and precise diagnostics.

### PP-011 — First curve pack: Peano and Moore

Implement, document and test two additional continuous square-domain curves with licence/reference provenance.

### PP-012 — Wunderlich curve family

Implement selected Meander/Serpentine variants after researching exact mathematical definitions and print suitability. Do not cargo-cult GPL implementation source.

### PP-013 — Non-square path foundation + Sierpiński/Gosper candidates

Prove whether V1 plane-path coordinates can cleanly support triangular/hexagonal recursive paths. Add only candidates that satisfy continuity, bounds, clipping and performance criteria.

### PP-014 — Project portability for installed packages

Save/load stable ID/version/hash/params, missing/mismatch UI and explicit user-controlled rebind/upgrade. No embedded execution yet.

### PP-015 — Optional embedded package transport with trust/quarantine

Allow projects to carry package bytes only under an explicit trust/import model. Opening a project must not auto-execute newly embedded source.

### PP-016 — Security, concurrency and performance campaign

Stress/fuzz malformed packages and scripts, prove thread isolation/reload snapshots/cancellation, baseline runtime/memory and establish justified regression gates.

### PP-017 — Release integration and custom-build documentation

Produce reproducible supported build(s), final install/update/user-script docs, packaged built-in curve set, provenance/licences, and final regression/release evidence.

## Parallelism policy

Parallel work is allowed only where dependencies above are satisfied and overlapping files are coordinated.

In particular:

- PP-005 can proceed after PP-001 while PP-002/003/004 advance.
- Curve issues should not begin before PP-008.
- PP-015 must not be folded into PP-006 "for convenience".
- PP-017 is not a substitute for unresolved hardening failures.

## Definition of campaign completion

The custom build is not complete merely because scripts run. Completion requires:

- strict Hilbert parity proof;
- multiple useful curves;
- independent top/bottom/internal selection;
- sandbox negative tests;
- deterministic project identity;
- explicit trust boundary for transported executable source;
- concurrency/cancellation/performance evidence;
- reproducible supported build and documentation.

## Second-pass code-audit corrections

`11-CODE-AUDIT-2026-09-18.md` was produced after rereading the roadmap against Orca's current implementation. The dependency graph remains valid, but the issue scopes are tightened:

- **PP-002:** characterize clipped-fragment/final ordering, batching identity inputs, effective-pattern substitutions, and current solid smoothing semantics.
- **PP-003:** centralize bridge-flow, ordering/reversibility, origin/center/fill-order distinctions; audit exhaustive enum switches.
- **PP-004:** add script invocation identity to `SurfaceFillParams` batching, preserve the `FillParams` POD invariant, make the sentinel safely constructible/valid immediately, and clear script identity on native substitutions.
- **PP-005:** integrate pinned Lua through Orca's dependency superbuild, not a system runtime.
- **PP-006:** expose a slice-wide immutable registry snapshot as a first-class object.
- **PP-007:** preserve concrete output dispatch for `InfillPolylineClipper`; script failures use propagated `SlicingError`; validate scaled-coordinate range and minimum usable output.
- **PP-008:** first parity target is native unsmoothed Hilbert, including its exact power-of-two covering square/orientation.
- **PP-009:** test final ordering, different-script batching, narrow-concentric substitution and bridge/helper exclusions; do not require sparse smoothing on solid paths.
- **PP-010:** keep the native pattern enum static: sentinel row plus a separate dynamic package/parameter control.
- **PP-014:** cover all config/project serialization surfaces rather than one project format.
- **PP-016:** stress slice-wide snapshot consistency and complete-path buffering/memory growth.

These are amendments to the existing PP issues, not a new parallel implementation track.

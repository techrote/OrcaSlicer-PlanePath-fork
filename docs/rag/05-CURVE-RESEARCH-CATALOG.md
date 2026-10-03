# Space-filling-curve research catalogue

## Primary research lead: Math::PlanePath

Orca's own `FillPlanePath.hpp` records ancestry from Perl's Math::PlanePath. Treat Math::PlanePath as a taxonomy and mathematical research lead.

Important licensing rule: its implementation source is GPL-family material. Do not copy implementation text into this project merely because Orca mentions it. Reimplement from mathematical definitions/permissive references unless a deliberate licence review establishes compatibility and attribution requirements.

## Selection criteria for printable plane paths

Prefer curves that:

- are continuous at finite approximation order;
- visit/fill the domain reasonably uniformly;
- have bounded local step length;
- can be generated incrementally without storing the whole order;
- expose stable orientation/origin semantics;
- do not create unintended long diagonals at recursion joins;
- clip sensibly against arbitrary polygons;
- have understandable turn/acceleration behaviour;
- can obey output-point budgets.

## Tier 0: parity oracle

### Hilbert

Status: native Orca reference + scripted golden implementation.

Purpose:

- freeze ABI coordinate semantics;
- validate registry/runtime/adapter;
- compare raw point sequence against the native generator;
- exercise smoothing and high-turn bridge sampling.

Do not remove native Hilbert during initial development.

## Tier 1: first useful curve pack

### Peano

- base-3 recursive square-filling traversal;
- continuous;
- good contrast to Hilbert's base-2 structure;
- valuable for proving order/grid calculations are not Hilbert-specific.

### Moore

- closed-loop Hilbert-relative construction;
- useful surface aesthetic and alternate start/end topology;
- investigate how clipping destroys/retains the closed-loop advantage.

### Wunderlich Meander / Serpentine family

- strong Math::PlanePath research lead;
- square-grid recursive traversals;
- useful for evaluating turn density and alternate local motion.

Each shipped implementation needs independent mathematical references recorded in its source/tests and licence review of any code used.

## Tier 2: non-square / recursive geometry

### Sierpiński-family continuous traversals

Triangular/diagonal geometry can stress assumptions hidden by square orthogonal paths.

Do not ship until tests prove scaling, clipping, smoothing and origin semantics.

### Gosper / flowsnake

Hexagonal recursive geometry is aesthetically valuable and an excellent test of non-orthogonal path support.

Investigate:

- coordinate basis conversion;
- endpoint continuity;
- segment length uniformity;
- path growth/order limits;
- clipping cost.

## Research/experimental candidates

- Dekking families;
- Kochel-style plane paths;
- Beta/Omega variants;
- other Math::PlanePath continuous traversals with suitable licences/reimplementable definitions.

Add candidates only with a short evidence note covering continuity and printable relevance.

## Explicitly deferred: discontinuous address orders

### Morton / Z-order

Morton ordering is useful computationally but has discontinuous jumps between some successive cells. It must not be faked as a continuous extrusion by drawing a giant connecting segment.

A future ABI may add explicit path breaks/travel semantics after the continuous system is mature. Until then Morton belongs in research, not the shipping solid-fill selector.

## Curves that fill only after thickening

Fractals such as dragon-curve variants can be mathematically interesting but are not automatically equivalent to a uniform solid-fill plane path at practical finite order. Evaluate them separately rather than labelling every fractal a solid space-filling curve.

## Per-pattern acceptance dataset

For every built-in scripted curve, tests should capture:

- known low-order point sequence;
- expected point count/order growth;
- endpoint(s);
- continuity / maximum step;
- bounds;
- deterministic repeatability;
- transformed orientation if supported;
- clipping smoke tests on rectangle, concave polygon, hole and disconnected islands;
- point-budget failure at intentionally excessive order.

## Licensing clarification after code audit

OrcaSlicer itself is AGPL-3.0, and the native Hilbert implementation in this codebase explicitly records adaptation from Math::PlanePath.

The conservative rule above means "do not copy external implementation source without checking it", not "GPL-family material is categorically unusable". For every copied/adapted implementation:

1. establish licence compatibility with this AGPL-3.0 fork;
2. preserve required copyright/attribution/notices;
3. record source/version/commit where practical;
4. prefer the existing Orca-native Hilbert implementation for exact parity if that is the clearest compatible source.

Do not deliberately implement a differently oriented Hilbert curve merely to avoid reusing already-compatible code, because PP-008 is a parity gate.

## Output growth note

Although pattern algorithms should generate incrementally where convenient, Orca's current `InfillPolylineOutput` stores the full point path. Candidate evaluation must therefore consider total finite-order point count and memory, not only whether the mathematical generator can be written as an iterator.

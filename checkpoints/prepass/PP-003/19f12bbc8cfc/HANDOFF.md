# PP-003 prepass handoff

Repository: `techrote/OrcaSlicer-PlanePath-fork` — migrated **#6 / PP-003**.
Assessed main: **`e71fec82f43285567debc4d2b05d51fb47c0820b`**.
Current synchronized Orca base: `00bb4202fe50ca625e59c6fcbec573a2887d22ed`.

Read `audit/AUDIT.md`, then `audit/GEOMETRY_AND_REFACTOR.md`. The 31-pattern CSV/JSON, 18 caller decisions, 13 explicit exceptions and switch inventory are the audit deliverables. `prototype/` is a compiled standalone interface proposal, not installed production code; no production patch is claimed.

## Restore and run

Python 3 and a C++17 compiler named `c++` are sufficient:

```sh
python3 scripts/verify_packet.py
python3 scripts/run_all.py --out /tmp/pp003-rerun
```

Observed results: 500 compiled predicate checks; 403 C++/JSON field comparisons; 267,840 context cases; six deliberate trait mutations detected; four invalid enum cases rejected; five PP002 literal Hilbert goldens; three Hamiltonian domains; 310 bridge boundary checks; scanner self-test. Compiler/runtime identities are in `results/decision_results.json`. These are source-extraction/prototype checks, NOT full Orca/native geometry tests.

## Intended repository paths

Research record: `docs/planepath/PP-003-PREPASS.md` (or archive this packet under `docs/planepath/pp003-prepass/`). Implementation proposal: `src/libslic3r/Fill/FillPatternTraits.hpp/.cpp`; wrappers/callers in `PrintConfig.hpp`, `FillBase.cpp`, `FillPlanePath.hpp`, `Fill.cpp`, `PrintObject.cpp`, `Layer.cpp`, existing native filler headers and `src/slic3r/GUI/ConfigManipulation.cpp`. Tests: `tests/libslic3r/test_fill_pattern_traits.cpp`, existing `tests/fff_print/test_fill.cpp` / `test_printobject.cpp`, and corresponding CMake lists. The exact sequence is in the refactor document.

## Do not lose these exceptions

`group_fills` carries order/smoothing between surfaces; resetting them changes behavior. Hilbert consumes hidden backend center mode. Rotation templates bypass separability. Forced PlanePath order does not restore source order in the low-density connect branch. CrossHatch/QuarterCubic fall through the nominally exhaustive anchor switch. Concentric and SpiralInset have different default order directions. All native pattern bridge-flow overrides are false, but physical bridge flow is contextual.

## Remaining gates / questions

No GitHub mutation was attempted. No sentinel, Lua dependency, registry, adapter or config change was made. PP003 is independent of #8.

A full checkout could not be obtained and fork search reported incomplete results; whole-tree switch completeness is NOT certified. Run `scripts/scan_repository.py --repo CHECKOUT --out inventory.json` on the assessed tree. Native factory, clipping, bridge contacts, final G-code and PP002 suite were not executed here; current main's retrieved checks did not establish native conformance green. Complete LockedZag custom emission and planner/source-order internals during implementation. Keep carry-over fixes separate from the neutral traits refactor unless explicitly adjudicated with new geometry evidence.

The manifest verifies the packet's retained bytes. Source blob IDs in `evidence/sources.json` were reported by the connector; complete original source blobs are not bundled/rehashed. Do not mistake normalized excerpts or transcribed fixtures for a full source checkout.

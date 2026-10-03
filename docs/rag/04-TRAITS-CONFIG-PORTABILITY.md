# Traits, configuration and project portability

## Why traits come first

Current Orca code contains behaviour keyed directly to concrete enum values. Scripted patterns cannot safely participate if every new capability requires another hard-coded `ipX` comparison.

PP-003 therefore converts behaviour into one trait source **after** PP-002 captures golden behaviour.

Initial traits to audit:

- plane-path implementation;
- centered vs corner/global alignment;
- self-crossing;
- separable per connected body;
- smoothable;
- multiline support where relevant;
- dense bridge/reconstruction sampling for high-turn paths;
- any upstream property discovered during the full reference search.

Do not invent a trait merely to preserve an unexplained branch. First characterize the behaviour with a test.

## Configuration identity

One compiled sentinel (`ipScriptedPlanePath` or upstream-appropriate equivalent) selects the adapter.

Dynamic identity is stored separately per solid-fill context.

Conceptual fields:

```text
top_surface_pattern = scriptedplanepath
top_surface_script_pattern = "org.example.moore"
top_surface_script_params = { ...canonical... }

bottom_surface_pattern = scriptedplanepath
bottom_surface_script_pattern = "org.example.peano"
bottom_surface_script_params = { ...canonical... }

internal_solid_infill_pattern = scriptedplanepath
internal_solid_infill_script_pattern = "org.example.hilbert"
internal_solid_infill_script_params = { ...canonical... }
```

If Orca's config types make a canonical map awkward, a canonical JSON/string representation is acceptable, but serialization order must be deterministic and the schema must be validated before slicing.

## Backward compatibility

Existing enum values/presets retain their meaning.

Rules:

- never renumber existing enum values casually; follow upstream enum serialization conventions;
- strict deserialization of old projects/presets must continue to work;
- projects not using scripted patterns should not acquire script companion options semantically;
- unknown new script companion fields should follow Orca's existing forward/backward compatibility policy;
- add migration tests for any key rename.

## Package identity

A project selection should record enough identity to detect accidental substitution:

- stable pattern ID;
- package version;
- cryptographic content hash or equivalent immutable content identity;
- canonical parameter payload.

For a locally installed package, reopening a project with same ID but different strict hash must not silently proceed as if identical. The UX may allow an explicit user-approved upgrade/rebind, but that action is visible.

## Missing-pattern behaviour

Missing package = unresolved configuration.

Allowed:

- show the missing ID/version/hash;
- preserve the unresolved selection in the project;
- allow the user to install/rebind explicitly.

Forbidden:

- silently change to Rectilinear/Hilbert/another package;
- discard parameters;
- execute a same-named package with mismatching strict identity without acknowledgement.

## Project-embedded scripts

Separate local script support from executable-content portability.

Stage A:

- serialize ID/version/hash/params;
- resolve against installed registry;
- robust missing/mismatch UX.

Stage B, later issue:

- optional embedding/import of pattern package bytes;
- embedded source is **data/quarantined** until explicitly trusted/imported;
- project opening must not automatically execute newly embedded source;
- display origin/hash and trust decision;
- sandbox still applies after trust.

This separation is mandatory; convenience is not a reason to make a 3MF an auto-running code container.

## UI projection

Dynamic scripts may appear alongside fill patterns, but the underlying config stays stable:

- selecting native pattern writes native enum;
- selecting dynamic package writes sentinel + companion identity;
- parameter controls are generated from validated manifest schema;
- changing context (top/bottom/internal) preserves independent selections.

The UI must make "scripted/custom" provenance visible enough that a user can tell a project depends on non-stock geometry.

## Code-audit amendments

The second-pass audit in `11-CODE-AUDIT-2026-09-18.md` adds mandatory implementation constraints.

### Trait granularity

Do not model all center/order behaviour as one `centered` boolean. Audit and separate:

- plane-path origin centering;
- separability/per-body origin;
- support for `center_of_surface_pattern`;
- support for top/bottom `SurfaceFillOrder`;
- source-path ordering preservation;
- path reversibility;
- dense bridge/reconstruction sampling;
- bridge-flow behaviour;
- sparse-only smooth/multiline capabilities.

### Batching identity

The sentinel alone is not a complete effective pattern identity. A compact scripted invocation identity/fingerprint must be part of `SurfaceFillParams::operator<` and `operator==`; otherwise different scripts/params may be grouped together.

When Orca deliberately substitutes a native effective pattern—especially narrow internal solid -> `ipConcentricInternal`—clear scripted invocation identity from that effective group.

### FillParams POD invariant

Do not add owning dynamic state to `FillParams`; it is statically required to remain trivially copyable. Resolve the scripted invocation against a slice-owned immutable registry snapshot and configure the filler object with immutable state before generation.

### Static enum/UI boundary

The compiled sentinel belongs in the static enum map and the explicit allowed lists for top, bottom and internal-solid patterns only. Dynamic package names remain companion state and are shown by a separate dynamic picker/editor, not by allocating enum values per package.

### Serialization test surface

Companion keys must be tested through preset JSON, CLI/config validation, object/volume `ModelConfig` overrides, standard 3MF, Orca/BBS 3MF, and relevant copy/undo/reset flows.

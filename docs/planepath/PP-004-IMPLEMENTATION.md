# PP-004 scripted invocation foundation

PP-004 retains native enum indices **0–30** and adds **31 = ipScriptedPlanePath**, serialized
as `scriptedplanepath`. The compiled `ipCount` becomes 32, but the PP-003 native trait
authority remains exactly 31 patterns; unresolved scripted traits are not inferred.

Companion options (flat `ConfigOptionString` members of `PrintRegionConfig`):

- `top_surface_script_pattern` / `top_surface_script_params`
- `bottom_surface_script_pattern` / `bottom_surface_script_params`
- `internal_solid_infill_script_pattern` / `internal_solid_infill_script_params`

IDs are stable strings limited to 1,024 bytes, and parameter defaults are `{}`. PP-004 validates active
parameter payloads as bounded JSON objects and canonicalizes parsed key order using
the existing nlohmann JSON dependency for **effective invocation equality**. The
configuration fields are standard string options; syntactically valid parameter objects
are normalized at strict deserialization and at the generic INI/JSON/3MF
serialization boundaries. Equivalent parameter objects thus serialize
identically despite source key order or whitespace. Invalid **dormant** raw
strings remain reversible without affecting native slicing; once selected
the sentinel rejects invalid parameters explicitly. The future PP-006
schema and PP-014 immutable package/version/hash identity remain deferred.

The `SurfaceFillParams` batching key is now test-visible via
`Fill/SurfaceFillParams.hpp`; its strict ordering and equality both include the
complete script ID + canonical JSON object string. `FillParams` remains trivially
copyable and carries no owning script state. Native effective substitutions and
helper-generated solids clear scripted identity, including both narrow-concentric
replacement branches. PP-003's separate `fill_order`/`smooth_factor` carry-over
is explicitly left unchanged for issue #24.

`Fill::new_from_type` constructs a lightweight placeholder. All geometry/emission
entrypoints throw `SlicingError` (not the swallowed `InfillFailedException`);
missing IDs and invalid JSON are rejected earlier. No PP-005 runtime is queried,
and no scripts or registry packages are executed or loaded.

V1 adds the sentinel only to top/bottom/internal-solid enum whitelists. Native,
sparse, ironing and support behavior is unchanged; a sparse sentinel is rejected
by strict config validation.

### Persistence scope

The options use Orca's existing native config/print-preset/3MF key pathways.
BBS/Orca project persistence is covered end-to-end for global process config plus
generic object and volume `ModelConfig` overrides. Both model-config exporters
XML-escape C-style serialized object option values before placing them in
attributes, matching the existing volume path so JSON script parameters cannot
make project XML malformed.

The legacy Prusa-format importer is intentionally narrower and PP-004 does not
change that policy. In `Format/3mf.cpp::_3MF_Importer::_load_model_from_file()`,
the block that would read `Metadata/Slic3r_PE.config` is commented out and the
code explicitly says it will "only read the model config for Prusa 3mf".
`_handle_start_config_metadata()` then filters object/volume metadata through a
fixed legacy `valid_keys` list (for example name, transforms, extruder and
modifier), so the PP-004 generic keys cannot currently be restored by that
backend. Regression coverage proves the Prusa writer does serialize the sentinel
and companion fields into the archive, and separately proves the importer leaves
their process defaults / drops their non-whitelisted overrides. This is recorded
as an existing backend limitation rather than reactivating broad Prusa project
settings import inside PP-004.

### Qualification

Do not merge before successful exact Linux and Windows x64 tests, a
whole-checkout native-pattern audit and reviewed CI reports. Until PP-006/007,
an active scripted pattern must fail explicitly during slicing.

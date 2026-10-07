# Artifact preservation note

The completed PP-005 prepass produced the exact handoff archive
`PP-005-embedding-prepass-e71fec82.zip`.

- byte size: `2,543,274`
- SHA-256: `b1b0a100f2f6280311b89b411a10ec12b369392684eaa6ee31a8bdb25b5fe49c`
- `unzip -t`: passed immediately before checkpoint publication
- archive members: 63
- archive-internal SHA-256 records: 62
- extracted rebuild/rerun return code: 0

The exact ZIP bytes remain the authoritative conversation artifact and were
re-verified before this checkpoint save. The GitHub connector available for
this preservation step accepts UTF-8/base64 content but cannot ingest an
existing local binary file by path or file reference. Re-encoding 2.54 MB of
already-verified binary through the chat transport would risk changing or
truncating the artifact, so it was not recreated.

The Git checkpoint therefore retains:

- the exact ZIP SHA-256 sidecar and verification JSON;
- the complete archive-internal manifest and a full original-payload manifest;
- reports, source locators, acceptance matrix and handoff;
- probe source, fixtures and rerun scripts;
- proposed dependency/runtime API material and inert patch;
- environment/network evidence, compiler logs, one complete raw JSONL run,
  and a three-build reproduced summary covering GCC, Clang and sanitizer runs.

The ZIP's six compiled probe binaries and the other repetitive raw JSONL copies
remain hash-identifiable in `SOURCE_PAYLOAD.sha256` / `unpacked/MANIFEST.sha256`
and inside the authoritative ZIP, but are not duplicated into Git.

This is a publication-transport limitation only. It does not change the
prepass findings or turn the Lua 5.4.7 surrogate execution into exact-pin
acceptance evidence.

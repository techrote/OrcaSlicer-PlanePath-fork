# Artifact preservation note

The completed PP-005 prepass produced the exact handoff archive
`PP-005-embedding-prepass-e71fec82.zip`.

- byte size: `2543274`
- SHA-256: `b1b0a100f2f6280311b89b411a10ec12b369392684eaa6ee31a8bdb25b5fe49c`
- `unzip -t`: passed before checkpoint publication
- archive members: 63
- archive-internal SHA-256 records: 62
- extracted rebuild/rerun return code: 0

The exact ZIP bytes are retained in the conversation artifact named above and
were re-verified before this checkpoint save. The GitHub connector available
for this preservation turn accepts UTF-8/base64 content but cannot ingest an
existing local binary file by path or file reference. To avoid recreating or
transcoding the already-verified ZIP, this checkpoint stores its original
sidecar hash, external verification JSON, internal manifest and all useful
UTF-8 unpacked material. Six compiled probe binaries are likewise represented
by their original hashes/results and remain inside the exact ZIP, rather than
being re-encoded into Git.

This is a checkpoint-publication limitation only. It does not change the
prepass findings or turn the Lua 5.4.7 surrogate execution into exact-pin
acceptance evidence.

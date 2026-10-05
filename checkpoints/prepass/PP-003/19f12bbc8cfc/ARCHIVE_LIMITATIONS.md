# Checkpoint publication limitation

This checkpoint preserves an exact, hash-verified subset of the completed PP-003 prepass packet as ordinary Git blobs, including its handoff, full-packet SHA-256 manifest, principal geometry/refactor report, native-pattern decision table, compiled trait prototype, comparison program, decision results, runner, and archive verification receipt.

The already-produced binary handoff archive `OrcaSlicer_PP003_source_prepass_e71fec82.zip` could not be transferred byte-for-byte through the available GitHub connector because its Git Data write operation accepts supplied text/base64 content but exposes no upload primitive for the existing local binary attachment. Manual retranscription was explicitly rejected after blob-ID verification detected corruption. The exact local archive remains:

- SHA-256: `d7273947298bbf937f51d0c9c45a83bfad196cc8453d4e89482442671b5ebe00`
- size: 46,993 bytes
- Git blob identity of the exact bytes: `6ecd51858492318349c7d2a9a8b2997aee9864b1`
- internal file count: 32
- internal manifest records: 31
- CRC/readability verification: PASS

`PACKET_MANIFEST.sha256` is the exact manifest from that archive and records hashes for the complete unpacked payload. `artifacts/OrcaSlicer_PP003_verification.json` is the original verification receipt. Files from the complete packet that are not individually present here are therefore *identified and hash-addressed but not themselves reachable as Git blobs from this checkpoint*.

No production source patch is claimed or applied. The prototype files are inert prepass material. This checkpoint must not be interpreted as PP-003 implementation acceptance.

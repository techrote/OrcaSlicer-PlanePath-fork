# Plan review: initial proposal, defects, corrections

This document records the deliberate critique pass requested before emitting the implementation campaign.

## Initial proposal

The first concept was:

1. centralize pattern traits;
2. add one scripted enum/filler;
3. embed Lua;
4. create registry/ABI;
5. reimplement Hilbert;
6. add curve library;
7. add UI/project portability.

That direction was sound but the order and some identity/security details were insufficient.

## Review finding 1: refactoring before behavioural fixtures is risky

Hilbert/Octagram knowledge is scattered across fill, config, GUI and bridge/reconstruction paths. Moving it into traits first could preserve compilation while changing slicing.

**Correction:** PP-002 builds a golden/conformance harness before PP-003 trait refactor.

## Review finding 2: one global script ID is incorrect

Top, bottom and internal solid surfaces can use different fill patterns today. One `scripted_pattern_id` would couple independent selections.

**Correction:** companion pattern identity/params exist per fill context.

## Review finding 3: dynamic enum entries are a serialization trap

Installed scripts vary per machine and over time. Dynamic enum values would make presets/projects dependent on registry order.

**Correction:** one stable compiled sentinel + stable string package identity.

## Review finding 4: project embedding is executable-content delivery

Embedding Lua in a 3MF and executing it on open/slice would turn ordinary project exchange into code execution even with a sandbox.

**Correction:** local registry + ID/version/hash portability lands first. Embedded packages are a later opt-in trust/quarantine milestone and never auto-run merely because a project was opened.

## Review finding 5: discontinuous curves cannot be squeezed through a continuous polyline

Morton/Z-order can jump. Connecting jumps with extrusion changes the algorithm and can cross voids.

**Correction:** ABI v1 is continuous. Explicit path-break/travel semantics are deferred to a separately designed future ABI.

## Review finding 6: external Python is operationally wrong

It introduces runtime discovery, IPC/process failure modes, packaging differences and excessive authority.

**Correction:** embed a pinned Lua runtime and expose only a narrow C API host surface.

## Review finding 7: scripts must not inherit printer/G-code authority

Providing nozzle/G-code control "for flexibility" would make the extension impossible to reason about and harder to sandbox.

**Correction:** scripts emit topology/geometry only. Existing Orca fill/extrusion/G-code stages stay authoritative.

## Review finding 8: a single mutable Lua state is unsafe under slicing concurrency

Lua states are not a magic concurrent shared runtime.

**Correction:** execution isolation/snapshot semantics are architectural requirements and get concurrency tests.

## Review finding 9: copying Math::PlanePath code is not the research strategy

Orca points to Math::PlanePath, but the source's licence and a desire for auditable implementations make wholesale code transplantation undesirable.

**Correction:** use it as taxonomy/mathematical lead; independently implement from mathematical/permissive references unless explicit licence review says otherwise.

## Review finding 10: security quotas need more than a timeout

A wall timeout alone is nondeterministic and does not prevent memory/output abuse.

**Correction:** instruction, allocator memory, output point and cancellation budgets are separate mandatory controls.

## Resulting plan

The revised campaign is intentionally staged:

1. upstream bootstrap/provenance;
2. native conformance;
3. traits;
4. config identity;
5. sandbox runtime (parallel after bootstrap where dependency permits);
6. manifest/registry ABI;
7. adapter;
8. Hilbert parity gate;
9. solid-surface hardening;
10. UI;
11–13. curve packs and non-square research;
14–15. project identity then embedded-package trust;
16. concurrency/performance/security hardening;
17. release integration.

Do not collapse these gates simply to reduce issue count.

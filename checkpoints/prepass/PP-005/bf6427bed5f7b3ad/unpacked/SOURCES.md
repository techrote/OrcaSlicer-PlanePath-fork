# Sources and retained evidence

Read on 2026-10-04. Repository reads use immutable assessed commit
`e71fec82f43285567debc4d2b05d51fb47c0820b`, unless an issue/PR collection is named.
These are primary-source locators with read-scope notes, not a claim to have a
complete local source checkout. The packet SHA-256 manifest verifies local
files only. The Lua archive hash is publisher-listed, not a downloaded-byte
verification in this environment.

## Repository

For R2–R21, base URL is:
`https://github.com/techrote/OrcaSlicer-PlanePath-fork/blob/e71fec82f43285567debc4d2b05d51fb47c0820b/`

| ID | Path / scope | Reported Git blob SHA-1 where retained |
|---|---|---|
| R1 | https://github.com/techrote/OrcaSlicer-PlanePath-fork/issues/8 — full issue; open, reported zero comments | dynamic issue |
| R2 | AGENTS.md — full | 3d0a61b303dc375f316beef72427d48192ab889b |
| R3 | docs/rag/03-SCRIPT-ABI-SANDBOX.md — full ABI | see assessed tree |
| R4 | docs/rag/11-CODE-AUDIT-2026-09-18.md — runtime/output/error/deps refinements; long tool result not archived verbatim | see assessed tree |
| R5 | deps/CMakeLists.txt — helper and common dependency list | aa64ad4dc9fc7767195c99dca6f3811d41a109a8 |
| R6 | deps/libnoise/libnoise.cmake; deps/JPEG/JPEG.cmake — full recipes | 0e19f269260ac0b183db944df31f80fdfabb1e59; 82b0ca5565f7fe1609411054ee0116da19bb7273 |
| R7 | cmake/modules/FindNLopt.cmake — full | 42b47f218b74facffa20c4d4c3b212724fea91ab |
| R8 | deps/deps-windows.cmake — full | 4305489758e997905774ff8e095f06dd3b6b1d2c |
| R9 | CMakeLists.txt — selected dependency/build sections, lines 650–1070 | c431bff558b80647c1e286431f11c9184e495976 |
| R10 | src/libslic3r/CMakeLists.txt — link block, lines 690 onward | ef15304c8b899cd0f8fece026727bb91576d7d72 |
| R11 | build_win.bat — options/CRT config and actual deps build/pack commands, lines 1–180 and 570–730 | 098d6fe1ffbd3179fa58f7df5dbdb72687c17a75 |
| R12 | CMakeLists.txt — resource install/system runtime/CPack, lines 1180 onward | c431bff558b80647c1e286431f11c9184e495976 |
| R13 | .github/workflows/build_check_cache.yml — full | 2da05e1d696915de4309dc28aa78168602bdd101 |
| R14 | .github/workflows/build_deps.yml — full | f1cc9a9723261c72431e819fd75f474624c4933f |
| R15 | scripts/flatpak/make_deps_tar.sh — full | e561da2e7b4aeea55a7ee56555e1a5f46dc00dff |
| R16 | tests/AGENTS.md — full | e6f3bf864cd6a026f6384a50224555b22ade394d |
| R17 | docs/rag/02-TARGET-ARCHITECTURE.md — full | 04bbe394c42c56e4223d9a648a140950316f2c55 |
| R18 | docs/rag/06-TEST-CI-STRATEGY.md — full | addabbd916ac49696c1f4c723773a6363aaf6a5a |
| R19 | docs/rag/00-PROJECT-BRIEF.md — full | c15571e496fab68e0789a8f75c36ea3259e81f99 |
| R20 | docs/rag/07-PLAN-REVIEW.md — full | 464e28c485f3c1d2c1d06bcffe9d54270ef2f4c0 |
| R21 | docs/rag/09-AUTONOMOUS-EXECUTION-CONTRACT.md — full; user read-only scope overrides publication instructions | defe3f679558bea853b24645c2fd28f9d21404ca |

Also read the main branch identity, full recursive tree, relevant directory
listings, and open PR collection. Open sync PR #3 head was
`08bd2e4c8e39758ef87542fc527738ab99403618`; its base matched the assessed commit.
No PP-005 implementation PR appeared in that returned collection. This is not
a claim to have audited every branch or all historical CI runs.
`RAG.md` and guessed `cmake/modules/FindLIBNOISE.cmake` returned 404; the actual
project docs live under docs/rag and NLopt provided the imported-target example.

## Lua primary sources

| ID | URL | Inspected subject |
|---|---|---|
| U1 | https://www.lua.org/ftp/ | 5.5.1 release date, archive and SHA-256 |
| U2 | https://www.lua.org/bugs.html | 5.4 maintenance endpoint; 5.5.1 known bug |
| U3 | https://www.lua.org/manual/5.5/manual.html | lua_Alloc, lua_newstate, lua_newthread, load modes, errors, hooks, tail calls |
| U4 | https://www.lua.org/source/5.5/lmem.c.html | allocation/free contract, shrink handling, emergency GC retry |
| U5 | https://www.lua.org/source/5.5/ldo.c.html | C/C++ protected transfer, panic, Lua stack ceiling, protected close/parser |
| U6 | https://www.lua.org/source/5.5/ldebug.c.html | instruction-hook dispatch |
| U7 | https://www.lua.org/source/5.5/ldo.h.html | LUAI_MAXCCALLS and syntactic/native recursion |
| U8 | https://www.lua.org/source/5.5/lstate.c.html and lstate.h.html | newstate seed, ownership/global state, startup and close, native stack counter |
| U9 | https://github.com/lua/lua/commit/0b29f408433e92953cc72b1d3e06c7ac8139e439 | full commit metadata and production patch hunks via GitHub read connector |
| U10 | https://www.lua.org/source/5.5/lstrlib.c.html | matcher recursion, C routines, string metatable |
| U11 | https://www.lua.org/source/5.5/luaconf.h.html ; https://www.lua.org/source/5.5/llimits.h.html ; https://www.lua.org/source/5.5/lobject.c.html | numeric config, platform macros and numeric/GC implementation |
| U12 | https://www.lua.org/source/5.5/lgc.c.html | finalizer cleanup and negative-growth fix context |
| U13 | https://www.lua.org/source/5.5/lua.h.html | public signatures and complete required Lua notice |

The web source pages label their content **5.5.1**; they are browsed source, not
a cryptographically verified replacement for the official tarball. Some are
long; the functions relevant to this assessment were inspected, not every line
of every source file. The release patch matches the displayed production
contexts; actual application to the archive remains a gate.

## Local evidence

`results/environment.txt` identifies compiler/OS/libc/library and ELF build ID.
`results/reproduced/summary.json` lists canonical commands, return codes and
executable hashes. JSONL files give individual test observations; empty compiler
or stderr logs mean no diagnostics were emitted, not a missing test result.
`results/network.txt` records the workspace retrieval blocker.
The installed runtime is NOT the recommended release and is NOT distributed
in the ZIP. The included probe executables dynamically use that recorded
library and are Linux-only; portable reproduction uses the C++ source.

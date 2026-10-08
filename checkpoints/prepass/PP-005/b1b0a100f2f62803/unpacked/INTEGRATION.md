# Proposed dependency and implementation integration

Base: e71fec82f43285567debc4d2b05d51fb47c0820b. These are proposals, not applied
repository changes. `proposed-dependency.patch` adds recipe files only; it does
NOT wire the root targets or implement ScriptRuntime.

## Actual superbuild pattern

The existing `deps/CMakeLists.txt` defines `orcaslicer_add_cmake_project` around
`ExternalProject_Add(dep_<name>)`, marks projects `EXCLUDE_FROM_ALL`, forwards
compiler/toolchain/launcher/architecture settings and installs to `DESTDIR`.
Normal DESTDIR is `<deps-build>/OrcaSlicer_dep/usr/local/`; legacy Linux trees
can use `destdir`. Flatpak omits the appended `/usr/local`. Sources are cached
under `DEP_DOWNLOAD_DIR/<name>`. [R5]

Dependencies are included in the COMMON portion after platform setup and must
also occur in `_dep_list`, which feeds `add_custom_target(deps ALL ...)`.
`deps/libnoise/libnoise.cmake` and JPEG show the URL/HASH/helper pattern. The
NLopt finder consumes an exported config target and inspects its imported
configuration-specific archive locations. [R5, R6, R7]

### 1. Add the self-contained recipe directory

Add `proposed/deps/Lua/*` as `deps/Lua/*`. The recipe pins the official tarball,
copies its own CMake build/export/license files into the extracted source, and
applies the provenance-labelled GC fix. All files live inside `deps/` so the
Flatpak dependency tar does not need sibling `cmake/` sources. No download is
performed by Orca's main CMake build.

Build only the explicitly enumerated core and lauxlib C sources into static
`planepath_lua55` (`planepath_lua55d` for Debug). Do not compile `lua.c`, `luac.c`,
`linit.c`, `loadlib.c`, or any standard-library registration units. The host
will supply its narrow helper allowlist; this is deliberate, not an incomplete
accidental standard-library link. No `LUA_BUILD_AS_DLL`, `LUA_USE_LINUX`,
`LUA_USE_MACOSX`, `LUA_USE_DLOPEN`, readline or dynamic-loader link dependency.
Private C build limits and numeric flags are in the draft CMakeLists.

On Unix, the static target publishes its `m` link requirement. Install core
public headers in `include/planepath-lua-5.5`, archive under `lib`, exported
`PlanePathLua::Lua` and exact-version config under `lib/cmake/PlanePathLua`, and
the complete Lua notice under `share/licenses/PlanePathLua`. Use GNUInstallDirs;
pass `CMAKE_INSTALL_LIBDIR=lib` when preparing the documented Orca prefixes.
If a maintained platform uses lib64, the main lookup must honor that actual
export location rather than guessing.

### 2. Wire the superbuild

In `deps/CMakeLists.txt`, adjacent to `include(libnoise/libnoise.cmake)`, insert:

```cmake
include(Lua/Lua.cmake)
```

In `set(_dep_list ...)`, adjacent to `dep_libnoise`, insert `dep_Lua`.
Do not add a Lua system `find_package` escape to the Flatpak branch. Common
inclusion covers MSVC x64/ARM64, clang-cl, macOS, Linux and the existing MinGW
branch without duplicating platform recipes. Actual platform builds remain
required, not implied by this structural reuse.

The draft uses the helper's **FORWARD_CONFIG** so its `cmake --build/--install`
configuration follows the enclosing build. Do not copy `add_debug_dep`: the
current Windows macro invokes `msbuild ... INSTALL.vcxproj` and is not portable
to the supported clang-cl/Ninja path. [R8]

### 3. Main-build import and linkage

Place the lookup with the other dependencies, near the root
`find_package(NLopt 1.4 REQUIRED)` around assessed lines 993+. Require an explicit
trusted dependency prefix; do not use generic `FindLua`:

```cmake
# Default from the build's existing dependency prefix; support a prefix list.
if(NOT PLANEPATH_LUA_PREFIX)
    if(NOT CMAKE_PREFIX_PATH)
        message(FATAL_ERROR "PlanePathLua requires Orca's dependency prefix")
    endif()
    list(GET CMAKE_PREFIX_PATH 0 _pp_prefix_default)
    set(PLANEPATH_LUA_PREFIX "${_pp_prefix_default}" CACHE PATH "Built Lua dependency prefix")
endif()
# Force the selected config path so an old cached system/package config is not used.
set(PlanePathLua_DIR "${PLANEPATH_LUA_PREFIX}/lib/cmake/PlanePathLua"
    CACHE PATH "Pinned PlanePathLua config" FORCE)
find_package(PlanePathLua 5.5.1 EXACT CONFIG REQUIRED
    PATHS "${PlanePathLua_DIR}" NO_DEFAULT_PATH)
if(NOT PlanePathLua_REVISION STREQUAL "5.5.1-pp1")
    message(FATAL_ERROR "Unexpected PlanePathLua source/build profile")
endif()
get_target_property(_pp_type PlanePathLua::Lua TYPE)
if(NOT _pp_type STREQUAL "STATIC_LIBRARY")
    message(FATAL_ERROR "PlanePathLua must be static")
endif()
```

Check imported archive/include real paths are under the selected relocated
prefix in the dependency smoke test. Enforce version/profile identity even when
a machine happens to have another Lua. Scope the Lua headers privately to the
runtime target and do not put Lua types in public fill structures.

In `src/libslic3r/CMakeLists.txt`, add `PlanePathLua::Lua` to the existing
`target_link_libraries(libslic3r PRIVATE ...)` block (assessed lines 703+), or to
a new internal `planepath_script_runtime` static target linked privately by
libslic3r. The latter allows the runtime probe to build without geometry deps;
keep one implementation, not a second test-specific runtime. CMake carries
static-library private link requirements to final links as link-only usage.
Never hand-write a `.lib` name in the executable target. [R9, R10]

### 4. Windows configurations and packaging

`build_win.bat` explicitly shares /MD dependency trees for Release,
RelWithDebInfo and MinSizeRel; Debug has a separate /MDd tree. It supports
MSVC/Visual Studio and clang-cl/Ninja Multi-Config. Its selected configuration
is passed to the deps build. [R11]

The draft target sets `MSVC_RUNTIME_LIBRARY` to
`MultiThreaded$<$<CONFIG:Debug>:Debug>DLL`, static **Lua**, not static CRT. Its
config maps the three non-Debug configurations only to other /MD variants,
and Debug only to Debug. Fail early for a selected Debug build without the
Debug archive. Never map Debug to Release merely to make linking work.

Commands for later acceptance (not run here), using the existing script:

```bat
build_win.bat -d --arch x64 --msvc --msbuild --config release -t dep_Lua
build_win.bat -d --arch x64 -l -x --config release -t dep_Lua
build_win.bat -d --arch x64 -l -x --config debug -t dep_Lua
build_win.bat -p --arch x64 -l -x --config release
```

A dep_Lua-only prefix is enough for the standalone imported-target probe, not
a complete Orca application build. Use a complete existing/rebuilt deps prefix
for the main application. Add ARM64 release coverage to match the maintained
Windows architecture. The current packaging script archives the entire
`OrcaSlicer_dep` directory into an architecture/compiler/config-labelled ZIP;
there is no separate Lua DLL list to extend. Its archive path already includes
the headers/config/archive/license. [R11]

For the distributable application, add the Lua notice as
`resources/licenses/Lua-5.5.1.txt` (copy the reviewed complete notice from
`deps/Lua/LICENSE.lua.txt`, and test equality), with a concise source/version/
patch attribution nearby. Windows root CMake installs the resource directory;
its CPack/NSIS package consumes installation output. Existing system-runtime
packaging remains needed because /MD still uses the Microsoft runtime; static
Lua does not eliminate that requirement. Verify the installed/portable ZIP and
NSIS installation both carry the notice and execute a script on a machine with
no Lua installed. `dumpbin /DEPENDENTS` or `llvm-readobj --coff-imports` must
show **no Lua DLL dependency**. [R12]

For macOS verify the notice is in the actual app bundle; root CMake's macOS
resource installation differs, so Windows installation alone does not establish
that result. Linux/FHS installs the resource tree. No UI screenshot proves
runtime packaging.

### 5. Cache and Flatpak consequences

`build_check_cache.yml` hashes **`deps/**`** and keys by OS/architecture/compiler.
Changing the new recipe/patch/profile files therefore changes the dependency
cache key without workflow mutation. Current `build_deps.yml` delivers via
cache; its old dependency artifact upload steps are commented out. A failed
or stale prefix must fail exact dependency discovery, not silently resolve a
system library. [R13, R14]

`make_deps_tar.sh` reproducibly packs `deps/`, excluding build and DL_CACHE.
Regenerate that tar and its manifest hash where the maintained Flatpak manifest
references it. The Lua source archive must also be declared as an offline
Flatpak source with the exact hash, placed at the recipe's expected
`DEP_DOWNLOAD_DIR/Lua/` filename, or provided by an explicit local URL override
that retains the hash. A network download during flatpak-builder's isolated
build phase is not an acceptable implicit dependency. The exact Flatpak
manifest filename/source stanza has NOT been read in this prepass and must be
reconciled before claiming that packaging path supported. [R15]

## Implementation order and issue acceptance

1. **Dependency package first:** add recipe + audited fix, build/install/relocate
   and import smoke on the supported platforms; prove static/no external Lua.
2. **Execution core:** move the tested accounting/boundary pattern into
   `src/libslic3r/Script/ScriptRuntime.{hpp,cpp}` and a small trivial callback
   implementation. The proposed API header is a design declaration, not a
   drop-in implementation. Preserve source/error phase and fixed diagnostics.
3. **Allowlist and quotas:** explicit versioned capability table, compiler/load
   source bounds, instruction/cancel/native-work/output counters, immutable
   input exposure, forbidden API/method/metatable tests. No geometry adapter.
4. **Tests:** port to `tests/libslic3r/test_script_runtime.cpp`, list it in that
   suite's CMakeLists; put fixtures under `tests/data/planepath/runtime/`.
   Use `[ScriptRuntime]`, flat behavioral Catch2 test cases, collect worker
   results before main-thread assertions, and set CTest timeouts. [R16]
5. **Maintainable documentation:** update ABI allowlist/limits in doc03 and
   the audit's dependency section as needed; describe the implemented design
   in `docs/HLSD/ScriptRuntime.md`. Keep this investigative packet outside
   production/instruction paths; AGENTS says unimplemented investigation belongs
   in ignored planning material, not a permanent "implemented design" claim.
6. **Integration boundary gate:** prove an error result is usable by the
   SlicingError channel. The actual no-silent-missing-infill geometry test belongs
   with the later adapter; it must not be falsely counted by PP-005's host probe.

PP-005 is not complete until its actual runtime, capability and packaging
acceptance criteria pass. No PR/comment/branch/status/workflow was created here.
Open upstream-sync PR #3 was observed at head
`08bd2e4c8e39758ef87542fc527738ab99403618`; it is not the assessed base. Reconcile
live main before applying any future patch.

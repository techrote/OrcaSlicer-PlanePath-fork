# PP-005 packet

Start with HANDOFF.md. REPORT.md explains the embedding decisions;
INTEGRATION.md contains the concrete dependency/import/Windows plan;
ACCEPTANCE.md distinguishes observed mechanism tests from implementation gates.

## Recorded local reproduction

On the exact recorded Debian amd64 environment:

```sh
python3 probe/run_local.py --out /tmp/pp005-reproduction
```

The runner verifies the installed Lua shared-object SHA-256 before using the
small 5.4 declaration shim, builds with GCC/Clang and host ASan+UBSan, gives each
run a 15-second external timeout, and checks 33 passing case records. Use a new
output directory. `results/reproduced/summary.json` records actual commands,
return codes and executable hashes. Raw JSONL/stderr and compiler output remain
available. The external timeout protects the test process; it is not the
runtime cancellation mechanism.

## Exact proposed runtime (NOT executed in this prepass)

Supply the official archive locally; the script verifies its published hash:

```sh
bash probe/run_pinned.sh /path/to/lua-5.5.1.tar.gz /tmp/pp005-exact-pin
```

This uses a new directory, applies the proposed upstream patch, builds/installs
the static runtime, imports its CMake target using upstream headers, and runs
the same mechanism probe. Native Windows uses the same CMake projects with
its actual compiler/generator and configuration; see INTEGRATION.md. This
script is a prepared rerun path, not evidence that its 5.5 build already passes.

## Files and trust

`proposed-dependency.patch` adds only deps/Lua proposal files. It has a local
additive patch application check, not a full-repository build check.
`proposed/ScriptRuntimeAPI.hpp` is a compilable declaration sketch, not runtime
implementation. Test-only host functions are deliberately absent from the
proposed ABI. The packet does not contain the Lua release tarball or a full
Orca checkout; source locators and reported Git blob identities are in
SOURCES.md / evidence/assessment.json. Repository source bytes were read through
tools, not independently downloaded and hash-verified locally.

To verify the delivered archive with its internal manifest:

```sh
python3 verify_zip.py /path/to/PP-005-embedding-prepass.zip
```

The outer ZIP SHA-256 is supplied separately. The manifest excludes only itself,
to avoid a circular digest. The external verification JSON describes an actual
extract/rebuild/run check without putting a self-referential hash inside the ZIP.

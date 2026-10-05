#!/usr/bin/env bash
# NOT EXECUTED HERE: requires the verified official Lua tarball supplied locally.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "usage: $0 lua-5.5.1.tar.gz NEW-WORK-DIRECTORY" >&2; exit 2; fi
archive=$(realpath "$1")
expected=1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce
printf '%s  %s\n' "$expected" "$archive" | sha256sum -c -
root=$(cd "$(dirname "$0")/.." && pwd)
mkdir "$2" # refuse to overwrite an existing workspace
work=$(realpath "$2")
mkdir "$work/source"
tar -xzf "$archive" -C "$work/source" --strip-components=1
cp "$root/proposed/deps/Lua/CMakeLists.txt" "$root/proposed/deps/Lua/PlanePathLuaConfig.cmake.in" "$root/proposed/deps/Lua/LICENSE.lua.txt" "$work/source/"
git -C "$work/source" apply --check "$root/proposed/deps/Lua/gc-negative-growth.patch"
git -C "$work/source" apply "$root/proposed/deps/Lua/gc-negative-growth.patch"
cmake -S "$work/source" -B "$work/runtime-build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$work/prefix" -DCMAKE_INSTALL_LIBDIR=lib
cmake --build "$work/runtime-build" --config Release --parallel 2
cmake --install "$work/runtime-build" --config Release
cmake -S "$root/probe" -B "$work/probe-build" -DCMAKE_BUILD_TYPE=Release -DPLANEPATH_LUA_PREFIX="$work/prefix"
cmake --build "$work/probe-build" --config Release --parallel 2
ctest --test-dir "$work/probe-build" -C Release --output-on-failure

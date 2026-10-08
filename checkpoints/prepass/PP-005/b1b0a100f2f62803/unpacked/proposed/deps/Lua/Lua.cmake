# PROPOSED: source-audited, not built against the release in this prepass.
# Pin: upstream Lua 5.5.1 plus the two-line GC fix identified in PATCHES.md.
orcaslicer_add_cmake_project(Lua
    FORWARD_CONFIG
    URL https://www.lua.org/ftp/lua-5.5.1.tar.gz
    URL_HASH SHA256=1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce
    PATCH_COMMAND
        ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" "<SOURCE_DIR>/CMakeLists.txt"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/PlanePathLuaConfig.cmake.in" "<SOURCE_DIR>/PlanePathLuaConfig.cmake.in"
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_LIST_DIR}/LICENSE.lua.txt" "<SOURCE_DIR>/LICENSE.lua.txt"
        COMMAND ${GIT_EXECUTABLE} -C <SOURCE_DIR> apply
            "${CMAKE_CURRENT_LIST_DIR}/gc-negative-growth.patch"
    CMAKE_ARGS
        -DPLANEPATH_LUA_REVISION=5.5.1-pp1
)

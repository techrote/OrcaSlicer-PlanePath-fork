# PP-005: Lua is built in Orca's dependency superbuild, never obtained from
# PATH, FindLua, a host-shared library or a main-application configure download.
# Source: PUC Lua 5.5.1, 2026-07-24. See PATCHES.md.
orcaslicer_add_cmake_project(Lua
    FORWARD_CONFIG
    URL https://www.lua.org/ftp/lua-5.5.1.tar.gz
    URL_HASH SHA256=1c4b4068d67061f2a2231ad2b5422e77acea1487ea9890f6320af614f4373dce
    PATCH_COMMAND
        ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt" "<SOURCE_DIR>/CMakeLists.txt"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_LIST_DIR}/PlanePathLuaConfig.cmake.in"
            "<SOURCE_DIR>/PlanePathLuaConfig.cmake.in"
        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_CURRENT_LIST_DIR}/LICENSE.lua.txt" "<SOURCE_DIR>/LICENSE.lua.txt"
        COMMAND ${GIT_EXECUTABLE} -C "<SOURCE_DIR>" apply
            "${CMAKE_CURRENT_LIST_DIR}/gc-negative-growth.patch"
    CMAKE_ARGS
        -DCMAKE_INSTALL_LIBDIR=lib
        -DPLANEPATH_LUA_SANITIZE:BOOL=${PLANEPATH_LUA_SANITIZE}
)

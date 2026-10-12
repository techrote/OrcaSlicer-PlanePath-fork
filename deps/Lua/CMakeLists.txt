cmake_minimum_required(VERSION 3.15)
project(PlanePathLua VERSION 5.5.1 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# This is the exact source profile reviewed for PP-005, not a generic system Lua.
set(PLANEPATH_LUA_REVISION "5.5.1-pp1")
# Compile the core and auxiliary API, deliberately no standard libraries or CLI.
# The host must register only its reviewed sandbox helpers.
set(_lua_core
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c
    lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c
    ltm.c lundump.c lvm.c lzio.c lauxlib.c)
list(TRANSFORM _lua_core PREPEND "src/")
add_library(PlanePathLua STATIC ${_lua_core})
add_library(PlanePathLua::Lua ALIAS PlanePathLua)
set_target_properties(PlanePathLua PROPERTIES
    EXPORT_NAME Lua
    OUTPUT_NAME planepath_lua55
    DEBUG_POSTFIX d
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# Conservative worker-stack limits; instruction/allocator/output budgets belong
# to the sandbox host and are not replaced by these private compile definitions.
target_compile_definitions(PlanePathLua PRIVATE LUAI_MAXSTACK=32768 LUAI_MAXCCALLS=100)
if(MSVC)
    target_compile_definitions(PlanePathLua PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(PlanePathLua PRIVATE /fp:precise)
else()
    target_compile_options(PlanePathLua PRIVATE -fno-fast-math -ffp-contract=off)
    if(UNIX)
        target_link_libraries(PlanePathLua PUBLIC m)
    endif()
endif()

# Instrument the *actual pinned Lua C sources*, not only the C++ host.
option(PLANEPATH_LUA_SANITIZE "Instrument PUC Lua with ASan and UBSan" OFF)
if(PLANEPATH_LUA_SANITIZE)
    if(MSVC)
        message(FATAL_ERROR "The PP-005 Lua sanitizer profile requires GCC/Clang")
    endif()
    target_compile_options(PlanePathLua PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(PlanePathLua INTERFACE -fsanitize=address,undefined)
endif()

target_include_directories(PlanePathLua PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/planepath-lua-5.5>)

install(TARGETS PlanePathLua EXPORT PlanePathLuaTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/lua.h src/luaconf.h src/lauxlib.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/planepath-lua-5.5)
install(FILES LICENSE.lua.txt
    DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/PlanePathLua)
install(EXPORT PlanePathLuaTargets NAMESPACE PlanePathLua::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlanePathLua)
configure_package_config_file(PlanePathLuaConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/PlanePathLuaConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlanePathLua)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/PlanePathLuaConfigVersion.cmake"
    VERSION ${PROJECT_VERSION} COMPATIBILITY ExactVersion)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/PlanePathLuaConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PlanePathLuaConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/PlanePathLua)

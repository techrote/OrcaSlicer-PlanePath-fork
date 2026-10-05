cmake_minimum_required(VERSION 3.15)
project(PlanePathLua VERSION 5.5.1 LANGUAGES C)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
set(PLANEPATH_LUA_REVISION "5.5.1-pp1" CACHE STRING "Audited source/build profile")
# Keep all runtime units in C: do not turn Lua's internal throws into C++ throws.
# This intentionally omits lua.c, luac.c and ALL standard-library registration units.
set(lua_core
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c
    lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c
    ltm.c lundump.c lvm.c lzio.c lauxlib.c)
list(TRANSFORM lua_core PREPEND "src/")
add_library(PlanePathLua STATIC ${lua_core})
add_library(PlanePathLua::Lua ALIAS PlanePathLua)
set_target_properties(PlanePathLua PROPERTIES
    EXPORT_NAME Lua OUTPUT_NAME planepath_lua55 DEBUG_POSTFIX d
    C_STANDARD 99 C_STANDARD_REQUIRED YES C_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
# Candidate conservative build profile; Windows worker-stack testing remains mandatory.
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
target_include_directories(PlanePathLua PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/planepath-lua-5.5>)
install(TARGETS PlanePathLua EXPORT PlanePathLuaTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES src/lua.h src/luaconf.h src/lauxlib.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/planepath-lua-5.5)
install(FILES LICENSE.lua.txt DESTINATION ${CMAKE_INSTALL_DATADIR}/licenses/PlanePathLua)
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

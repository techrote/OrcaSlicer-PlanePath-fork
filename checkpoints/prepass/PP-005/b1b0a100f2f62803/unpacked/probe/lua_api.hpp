#pragma once
// LOCAL PROBE FALLBACK ONLY: declarations for the installed Debian amd64 Lua 5.4 ABI.
// Not vendored Lua headers; production builds must use the pinned source's headers.
#ifdef PP_USE_UPSTREAM_HEADERS
extern "C" {
#include <lua.h>
#include <lauxlib.h>
}
#else
#include <cstddef>
#include <cstdint>
extern "C" {
struct lua_State;
struct lua_Debug;
using lua_Integer = long long;
using lua_Number = double;
using lua_KContext = std::intptr_t;
using lua_Alloc = void *(*)(void *, void *, std::size_t, std::size_t);
using lua_CFunction = int (*)(lua_State *);
using lua_KFunction = int (*)(lua_State *, int, lua_KContext);
using lua_Hook = void (*)(lua_State *, lua_Debug *);
lua_State *lua_newstate(lua_Alloc, void *);
void lua_close(lua_State *);
lua_Number lua_version(lua_State *);
lua_Alloc lua_getallocf(lua_State *, void **);
int lua_gettop(lua_State *);
void lua_settop(lua_State *, int);
int lua_checkstack(lua_State *, int);
void lua_pushcclosure(lua_State *, lua_CFunction, int);
void lua_pushlightuserdata(lua_State *, void *);
void lua_pushinteger(lua_State *, lua_Integer);
const char *lua_pushlstring(lua_State *, const char *, std::size_t);
void lua_pushnil(lua_State *);
void lua_pushvalue(lua_State *, int);
void lua_setglobal(lua_State *, const char *);
int lua_getglobal(lua_State *, const char *);
void lua_createtable(lua_State *, int, int);
void lua_setfield(lua_State *, int, const char *);
int lua_type(lua_State *, int);
const char *lua_tolstring(lua_State *, int, std::size_t *);
lua_Integer lua_tointegerx(lua_State *, int, int *);
int lua_error(lua_State *);
int lua_pcallk(lua_State *, int, int, int, lua_KContext, lua_KFunction);
void lua_callk(lua_State *, int, int, lua_KContext, lua_KFunction);
void lua_sethook(lua_State *, lua_Hook, int, int);
int luaL_loadbufferx(lua_State *, const char *, std::size_t, const char *, const char *);
}
#define LUA_VERSION_NUM 504
#define LUA_OK 0
#define LUA_ERRRUN 2
#define LUA_ERRSYNTAX 3
#define LUA_ERRMEM 4
#define LUA_TSTRING 4
#define LUA_TFUNCTION 6
#define LUA_MASKCOUNT 8
#define lua_pcall(L,n,r,e) lua_pcallk((L),(n),(r),(e),0,nullptr)
#define lua_call(L,n,r) lua_callk((L),(n),(r),0,nullptr)
#define lua_pushcfunction(L,f) lua_pushcclosure((L),(f),0)
#endif

#include <lua.h>
#include <lauxlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#if LUA_VERSION_NUM != 505
#error "The PlanePath embedded source must be Lua 5.5"
#endif

static void *smoke_allocator(void *ud, void *ptr, size_t oldsize, size_t newsize)
{
    (void)ud;
    (void)oldsize;
    if (newsize == 0) { free(ptr); return NULL; }
    return realloc(ptr, newsize);
}

int main(void)
{
    const char source[] = "return (8 * 5) + 2";
    lua_State *L = lua_newstate(smoke_allocator, NULL, 0x55a11u);
    if (!L) { fputs("Lua state allocation failed\n", stderr); return 1; }
    /* No luaL_openlibs: scripts start with an empty global environment. */
    if (luaL_loadbufferx(L, source, sizeof(source) - 1, "@pp005-smoke", "t") != LUA_OK ||
        lua_pcall(L, 0, 1, 0) != LUA_OK) {
        fputs("Text-mode Lua execution failed\n", stderr);
        lua_close(L);
        return 2;
    }
    if (!lua_isinteger(L, -1) || lua_tointeger(L, -1) != 42) {
        fputs("Unexpected Lua arithmetic result\n", stderr);
        lua_close(L);
        return 3;
    }
    lua_close(L);
    return 0;
}

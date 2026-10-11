/* GC stress using the exact patched Lua 5.5.1 C core, not surrogate Lua.
 * Exercises transitions after large retained-object shrink and repeated
 * generational collection; instrumented together with Lua under ASan/UBSan.
 */
#include <lua.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *alloc(void *ud, void *p, size_t old, size_t n)
{
    (void)ud; (void)old;
    if (n == 0) { free(p); return NULL; }
    return realloc(p, n);
}

int main(void)
{
    lua_State *L = lua_newstate(alloc, NULL, 0x50500551u);
    if (!L) return 1;
    lua_gc(L, LUA_GCGEN, 0, 0);
    char value[1024];
    memset(value, 'P', sizeof(value));
    for (int round = 0; round < 64; ++round) {
        lua_createtable(L, 0, 0);
        for (int i = 1; i <= 256; ++i) {
            lua_pushinteger(L, i);
            lua_pushlstring(L, value, sizeof(value));
            lua_settable(L, -3);
        }
        /* Release the large recent allocation set before the next
         * major/minor decision. Repeatedly stress negative-growth cases. */
        lua_settop(L, 0);
        if ((round % 4) == 0)
            lua_gc(L, LUA_GCCOLLECT);
        else
            lua_gc(L, LUA_GCSTEP, 1);
    }
    lua_gc(L, LUA_GCCOLLECT);
    lua_close(L);
    puts("patched Lua 5.5.1 generational collector stress passed");
    return 0;
}

#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdio.h>
#include <stdbool.h>

bool ch_lua(lua_State *L, int result){
    if (result != LUA_OK){
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    ch_lua(L, luaL_dofile(L, "./assets/scripts/hello.lua"));

    lua_close(L);
    return 0;
}

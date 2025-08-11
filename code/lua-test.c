#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdio.h>

int main() {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    
    if (luaL_dofile(L, "./assets/scripts/hello.lua") != LUA_OK){
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    }

    lua_close(L);
    return 0;
}

#include <ghpl/scripting/lua/lua.h>

bool ch_lua(lua_State *L, int result){
    if (result != LUA_OK){
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void init_lua(lua_State **L){
    *L = luaL_newstate();
    luaL_openlibs(*L);
}

void end_lua(lua_State *L){
    lua_close(L);
}

int init_script(lua_State *L, const char *path_to){
    if (!ch_lua(L, luaL_dofile(L, path_to))){
        return -1;
    }
    
    return 0;
}

// Functions

int func_on_init(lua_State *L){
    lua_getglobal(L, "OnInit");
    if (!lua_isfunction(L, -1))
        return -1;

    if (!ch_lua(L, lua_pcall(L, 0, 0, 0)))
        return 2;

    return 0;
}

int func_on_tick(lua_State *L, float dt_s){
    lua_getglobal(L, "OnTick");
    if (!lua_isfunction(L, -1))
        return -1;

    lua_pushnumber(L, dt_s);
    if (!ch_lua(L, lua_pcall(L, 1, 0, 0)))
        return 2;

    return 0;
}

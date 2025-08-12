#include <ghpl/scripting/lua/lua.h>

bool ch_lua(lua_State *L, int result){
    if (result != LUA_OK){
        printf("Error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return false;
    }
    return true;
}

void init_lua(struct ScriptingContext *L){
    L->state = luaL_newstate();
    luaL_openlibs(L->state);
}

void end_lua(struct ScriptingContext *L){
    lua_close(L->state);
    L->event_ref = LUA_NOREF;
}

int init_script(struct ScriptingContext *L, const char *path_to){
    if (!ch_lua(L->state, luaL_dofile(L->state, path_to))){
        return -1;
    }

    lua_getglobal(L->state, "OnEvent");
    if (lua_isfunction(L->state, -1)){
        L->event_ref = luaL_ref(L->state, LUA_REGISTRYINDEX);
    } else {
        L->event_ref = LUA_NOREF;
        return 1;
    }
    lua_pop(L->state, -1);
    
    return 0;
}

// Functions

int func_on_init(struct ScriptingContext *L){
    lua_getglobal(L->state, "OnInit");
    if (!lua_isfunction(L->state, -1))
        return -1;

    if (!ch_lua(L->state, lua_pcall(L->state, 0, 0, 0)))
        return 2;

    return 0;
}

int func_on_tick(struct ScriptingContext *L, float dt_s){
    lua_getglobal(L->state, "OnTick");
    if (!lua_isfunction(L->state, -1))
        return -1;

    lua_pushnumber(L->state, dt_s);
    if (!ch_lua(L->state, lua_pcall(L->state, 1, 0, 0)))
        return 2;

    return 0;
}

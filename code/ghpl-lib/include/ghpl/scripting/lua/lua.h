#pragma once
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdio.h>
#include <stdbool.h>

struct ScriptingContext {
    lua_State *state;
    int event_ref;
};

int init_script(struct ScriptingContext *L, const char *path_to);

bool ch_lua(lua_State *L, int result);
void init_lua(struct ScriptingContext *L);
void end_lua(struct ScriptingContext *L);

// Functions
int func_on_init(struct ScriptingContext *L);
int func_on_tick(struct ScriptingContext *L, float dt_s);
// int func_on_event(lua_State *L);

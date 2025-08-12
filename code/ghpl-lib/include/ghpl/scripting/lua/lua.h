#pragma once
#include <lua5.4/lua.h>
#include <lua5.4/lualib.h>
#include <lua5.4/lauxlib.h>
#include <stdio.h>
#include <stdbool.h>

int init_script(lua_State *L, const char *path_to);

bool ch_lua(lua_State *L, int result);
void init_lua(lua_State **L);
void end_lua(lua_State *L);

// Functions
int func_on_init(lua_State *L);
int func_on_tick(lua_State *L, float dt_s);
// int func_on_event(lua_State *L);

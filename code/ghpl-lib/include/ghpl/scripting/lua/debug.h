#pragma once
#include <ghpl/scripting/lua/lua.h>
#include <ghpl/console/tgr.h>
#define MAX_DEBUG_LINES_NUM 2048

extern char *SCRIPTS_DEBUG_LINES[MAX_DEBUG_LINES_NUM];
extern int repeated_logs;

int script_intlog(lua_State *L);
void register_intlog(struct ScriptingContext *scontext, struct tgr_app *app);
void debug_cleanup();

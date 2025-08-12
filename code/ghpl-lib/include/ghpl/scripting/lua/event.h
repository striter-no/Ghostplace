#pragma once
#include <ghpl/utils/int.h>
#include <ghpl/scripting/lua/lua.h>

enum EVENT_TYPE {
    KEY_EVENT,
    MOUSE_EVENT,
    ELEMENT_EVENT
};

struct LuaSEvent {
    enum EVENT_TYPE type;

    i32 mouse_x;
    i32 mouse_y;
    i16 btn;
    i32 unich;

    u64 elem_uid;

    bool alted;
    bool shifted;
    bool ctrled;
};

struct LuaSEvent constr_event(enum EVENT_TYPE type);
int lua_event_trigger(struct ScriptingContext *L, struct LuaSEvent *event);

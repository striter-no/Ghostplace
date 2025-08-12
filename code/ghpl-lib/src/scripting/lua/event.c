#include <ghpl/scripting/lua/event.h>

struct LuaSEvent constr_event(enum EVENT_TYPE type){
    return (struct LuaSEvent){
        .type = type,
        .elem_uid = 0,
        .mouse_x = -1,
        .mouse_y = -1,
        .unich = 0,
        .alted = false,
        .ctrled = false,
        .shifted = false,
        .btn = -1
    };
}

int lua_event_trigger(struct ScriptingContext *LC, struct LuaSEvent *event){
    if (LC->event_ref == LUA_NOREF){
        return -1;
    }

    lua_State *L = LC->state;
    lua_rawgeti(L, LUA_REGISTRYINDEX, LC->event_ref);
    lua_newtable(L);

    // Adding type
    
    const char *type_str = "";
    switch (event->type) {
        case KEY_EVENT: type_str = "key"; break;
        case MOUSE_EVENT: type_str = "mouse"; break;
        case ELEMENT_EVENT: type_str = "elem"; break;

        default: type_str = "undef"; break;
    }
    lua_pushstring(L, type_str);
    lua_setfield(L, -2, "type");

    // Adding unich
    lua_pushinteger(L, event->unich);
    lua_setfield(L, -2, "unich");
    
    // Adding elemid
    lua_pushinteger(L, event->elem_uid);
    lua_setfield(L, -2, "elem");

    // Adding mouse_x
    lua_pushinteger(L, event->mouse_x);
    lua_setfield(L, -2, "mouse_x");

    // Adding mouse_y
    lua_pushinteger(L, event->mouse_y);
    lua_setfield(L, -2, "mouse_y");

    // Adding shifted
    lua_pushinteger(L, event->shifted);
    lua_setfield(L, -2, "shifted");

    // Adding ctrled
    lua_pushinteger(L, event->ctrled);
    lua_setfield(L, -2, "ctrled");

    // Adding alted
    lua_pushinteger(L, event->alted);
    lua_setfield(L, -2, "alted");

    // Adding btn
    lua_pushinteger(L, event->btn);
    lua_setfield(L, -2, "btn");

    if (!ch_lua(L, lua_pcall(L, 1, 0, 0))){
        return -1;
    }

    return 0;
}

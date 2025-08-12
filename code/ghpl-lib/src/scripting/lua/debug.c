#include <ghpl/scripting/lua/debug.h>

char *SCRIPTS_DEBUG_LINES[MAX_DEBUG_LINES_NUM] = {0};
int repeated_logs = 0;

static struct tgr_app *LOG_APP_PTR = NULL;
static int logs_line = 0;
static char *last_log_message = NULL;

int script_intlog(lua_State *L){
    int n = lua_gettop(L);

    luaL_Buffer buffer;
    luaL_buffinit(L, &buffer);

    for (int i = 1; i <= n; i++) {
        if (i > 1) {
            luaL_addchar(&buffer, ' ');  // Добавляем пробел между аргументами
        }
        
        size_t len;
        const char *s = luaL_tolstring(L, i, &len);
        luaL_addlstring(&buffer, s, len);
        lua_pop(L, 1);  // Убираем результат toluastring
    }
    
    // 4. Получаем итоговую строку
    luaL_pushresult(&buffer);
    const char *message = lua_tostring(L, -1);
    // printf("\n%s\n", message);
    // abort();
    // 5. Отображаем на экране
    if (LOG_APP_PTR) {
        if (last_log_message && strcmp(message, last_log_message) == 0){
            repeated_logs++;
        } else {
            size_t ins_inx = logs_line;
            if (ins_inx >= MAX_DEBUG_LINES_NUM){
                ins_inx = MAX_DEBUG_LINES_NUM - 1;
                
                char *log_buff[MAX_DEBUG_LINES_NUM] = {0};
                memcpy(log_buff, SCRIPTS_DEBUG_LINES + 1, sizeof(char*) * (MAX_DEBUG_LINES_NUM - 1));
                free(SCRIPTS_DEBUG_LINES[MAX_DEBUG_LINES_NUM - 1]);
                
                memcpy(SCRIPTS_DEBUG_LINES, log_buff, sizeof(char*) * (MAX_DEBUG_LINES_NUM - 1));
            }

            last_log_message = (char*)realloc(last_log_message, strlen(message) + 1);
            strcpy(last_log_message, message);
            SCRIPTS_DEBUG_LINES[ins_inx] = strdup(last_log_message);

            repeated_logs = 0;
            logs_line++;
        }
    }
    
    // 6. Убираем строку из стека
    lua_pop(L, 1);
    return 0;
}

void register_intlog(struct ScriptingContext *scontext, struct tgr_app *app){
    // Сохраняем указатель на приложение
    LOG_APP_PTR = app;
    
    // Регистрируем функцию в Lua
    lua_pushcfunction(scontext->state, script_intlog);
    lua_setglobal(scontext->state, "intlog");
}

void debug_cleanup(){
    free(last_log_message);
    for (size_t i = 0; i < MAX_DEBUG_LINES_NUM; i++){
        if (SCRIPTS_DEBUG_LINES[i]){
            free(SCRIPTS_DEBUG_LINES[i]);
            SCRIPTS_DEBUG_LINES[i] = NULL;
        }
    }
}

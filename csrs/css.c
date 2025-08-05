#include <css.h>


// Утилиты для работы со строками (те же, что и в XML-парсере)
static void trim_whitespace(char *str) {
    char *end;
    
    // Удаляем начальные пробелы
    while (isspace((unsigned char)*str)) str++;
    
    if (*str == 0) return;
    
    // Удаляем конечные пробелы
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
}

static size_t safe_strncpy(char *dest, const char *src, size_t size) {
    size_t len = strnlen(src, size - 1);
    memcpy(dest, src, len);
    dest[len] = '\0';
    return len;
}

// Парсинг значения атрибута с учетом кавычек
static size_t parse_attribute_value(const char **p_ptr, char *value_buf, size_t buf_size) {
    const char *p = *p_ptr;
    char *dest = value_buf;
    size_t copied = 0;
    
    // Пропускаем пробелы
    while (isspace((unsigned char)*p)) p++;
    
    // Проверяем кавычки
    char quote = '\0';
    if (*p == '"' || *p == '\'') {
        quote = *p;
        p++;
    }
    
    // Копируем значение
    while (*p && copied < buf_size - 1) {
        if (quote) {
            if (*p == quote) break;
        } else {
            if (isspace((unsigned char)*p) || *p == ';' || *p == '}') break;
        }
        
        *dest++ = *p++;
        copied++;
    }
    
    // Пропускаем закрывающую кавычку
    if (quote && *p == quote) p++;
    
    *dest = '\0';
    *p_ptr = p;
    return copied;
}

struct css_block* parse_css(const char *css) {
    if (!css || !*css) return NULL;
    
    struct css_block *blocks = NULL;
    u64 block_count = 0;
    const char *p = css;
    
    while (*p) {
        // Пропускаем пробелы и переносы
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        
        // Начало блока - имя
        const char *name_start = p;
        while (*p && !isspace((unsigned char)*p) && *p != '{') p++;
        size_t name_len = p - name_start;
        
        // Проверяем, что после имени есть '{'
        while (isspace((unsigned char)*p)) p++;
        if (*p != '{') {
            // Пропускаем до конца строки или блока
            while (*p && *p != '\n' && *p != '}') p++;
            continue;
        }
        
        // Создаем новый блок
        struct css_block *new_blocks = (struct css_block *)realloc(blocks, (block_count + 1) * sizeof(struct css_block));
        if (!new_blocks) {
            // Освобождаем уже выделенные блоки при ошибке
            for (u64 i = 0; i < block_count; i++) {
                free(blocks[i].name);
                free(blocks[i].attrs);
            }
            free(blocks);
            return NULL;
        }
        blocks = new_blocks;
        struct css_block *current = &blocks[block_count];
        block_count++;
        
        // Копируем имя блока
        current->name = (char *)malloc(name_len + 1);
        if (!current->name) {
            // Освобождаем текущий блок при ошибке
            for (u64 i = 0; i < block_count - 1; i++) {
                free(blocks[i].name);
                free(blocks[i].attrs);
            }
            free(blocks);
            return NULL;
        }
        memcpy(current->name, name_start, name_len);
        current->name[name_len] = '\0';
        
        // Инициализируем атрибуты
        current->attrs = NULL;
        current->attrs_used = 0;
        
        p++; // Пропускаем '{'
        
        // Парсим атрибуты внутри блока
        while (*p) {
            // Пропускаем пробелы и переносы
            while (isspace((unsigned char)*p)) p++;
            if (!*p || *p == '}') break;
            
            // Начало атрибута - имя
            const char *attr_name_start = p;
            while (*p && !isspace((unsigned char)*p) && *p != '=') p++;
            size_t attr_name_len = p - attr_name_start;
            
            // Проверяем наличие '='
            while (isspace((unsigned char)*p)) p++;
            if (*p != '=') {
                // Пропускаем до конца строки
                while (*p && *p != '\n' && *p != '}') p++;
                continue;
            }
            p++; // Пропускаем '='
            
            // Добавляем атрибут в блок
            struct attribute *new_attrs = (struct attribute *)realloc(
                current->attrs, (current->attrs_used + 1) * sizeof(struct attribute));
            if (!new_attrs) {
                // Освобождаем текущий блок при ошибке
                free(current->name);
                for (u64 i = 0; i < current->attrs_used; i++) {
                    // Имена атрибутов уже скопированы в структуру
                }
                free(current->attrs);
                // Освобождаем предыдущие блоки
                for (u64 i = 0; i < block_count - 1; i++) {
                    free(blocks[i].name);
                    free(blocks[i].attrs);
                }
                free(blocks);
                return NULL;
            }
            current->attrs = new_attrs;
            
            // Копируем имя атрибута
            safe_strncpy(current->attrs[current->attrs_used].name, 
                        attr_name_start, 
                        attr_name_len < MAX_ATTR_NAME ? attr_name_len + 1 : MAX_ATTR_NAME);
            
            // Парсим значение
            char value_buf[MAX_ATTR_VALUE];
            parse_attribute_value(&p, value_buf, sizeof(value_buf));
            
            // Копируем значение атрибута
            safe_strncpy(current->attrs[current->attrs_used].value, 
                        value_buf, 
                        sizeof(current->attrs[current->attrs_used].value));
            
            current->attrs_used++;
            
            // Пропускаем возможные разделители
            while (isspace((unsigned char)*p) || *p == ';') p++;
        }
        
        // Пропускаем '}' если есть
        if (*p == '}') p++;
    }
    
    // Добавляем завершающий NULL блок для обозначения конца массива
    struct css_block *new_blocks = (struct css_block *)realloc(blocks, (block_count + 1) * sizeof(struct css_block));
    if (new_blocks) {
        blocks = new_blocks;
        blocks[block_count].name = NULL;
        blocks[block_count].attrs = NULL;
        blocks[block_count].attrs_used = 0;
    }
    
    return blocks;
}

// Функция для освобождения памяти
void free_css(struct css_block *blocks) {
    if (!blocks) return;
    
    for (u64 i = 0; blocks[i].name != NULL; i++) {
        free(blocks[i].name);
        free(blocks[i].attrs);
    }
    
    free(blocks);
}
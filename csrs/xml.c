#include <xml.h>

// Утилиты для работы со строками
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

static int safe_strncpy(char *dest, const char *src, size_t size) {
    size_t len = strnlen(src, size - 1);
    memcpy(dest, src, len);
    dest[len] = '\0';
    return len;
}

// Парсинг атрибутов
static u16 parse_attributes(const char *str, struct attribute *attrs, u16 max_attrs) {
    u16 count = 0;
    const char *p = str;
    
    while (*p && count < max_attrs) {
        // Пропускаем пробелы
        while (isspace((unsigned char)*p)) p++;
        if (!*p || *p == '>') break;
        
        // Имя атрибута
        const char *name_start = p;
        while (*p && !isspace((unsigned char)*p) && *p != '=' && *p != '>') p++;
        size_t name_len = p - name_start;
        
        // Пропускаем '='
        if (*p == '=') p++;
        while (isspace((unsigned char)*p)) p++;
        
        // Значение атрибута
        char quote = '\0';
        if (*p == '"' || *p == '\'') {
            quote = *p;
            p++;
        }
        
        const char *val_start = p;
        while (*p && *p != '>' && 
              (!quote ? !isspace((unsigned char)*p) : *p != quote)) {
            p++;
        }
        size_t val_len = p - val_start;
        
        // Пропускаем закрывающую кавычку
        if (quote && *p == quote) p++;
        
        // Сохраняем атрибут
        if (name_len > 0 && val_len > 0) {
            safe_strncpy(attrs[count].name, name_start, 
                        name_len < MAX_ATTR_NAME ? name_len + 1 : MAX_ATTR_NAME);
            safe_strncpy(attrs[count].value, val_start, 
                        val_len < MAX_ATTR_VALUE ? val_len + 1 : MAX_ATTR_VALUE);
            count++;
        }
    }
    
    return count;
}

// Основная функция парсинга
struct tag* parse_xml(const char *input) {
    if (!input || !*input) return NULL;
    
    struct tag *root = NULL;
    struct tag *stack[MAX_CHILDREN];
    int stack_top = -1;
    const char *p = input;
    
    while (*p) {
        // Пропускаем пробелы и переносы
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        
        if (*p == '<') {
            p++; // Пропускаем '<'
            
            // Закрывающий тег
            if (*p == '/') {
                p++;
                char name_buf[MAX_NAME] = {0};
                int i = 0;
                
                while (*p && *p != '>') {
                    if (i < MAX_NAME - 1) name_buf[i++] = *p;
                    p++;
                }
                name_buf[i] = '\0';
                p++; // Пропускаем '>'
                
                if (stack_top >= 0) {
                    stack_top--; // Выходим из текущего тега
                }
                continue;
            }
            
            // Открывающий тег
            char name_buf[MAX_NAME] = {0};
            int i = 0;
            
            // Имя тега
            while (*p && !isspace((unsigned char)*p) && *p != '>' && *p != '/') {
                if (i < MAX_NAME - 1) name_buf[i++] = *p;
                p++;
            }
            name_buf[i] = '\0';
            
            // Создаем новый тег
            struct tag *new_tag = (struct tag *)calloc(1, sizeof(struct tag));
            if (!new_tag) return NULL;
            
            safe_strncpy(new_tag->name, name_buf, MAX_NAME);
            new_tag->uid = next_uid++;
            new_tag->content = NULL;
            
            // Атрибуты
            if (isspace((unsigned char)*p)) {
                p++; // Пропускаем пробел
                const char *attr_start = p;
                
                while (*p && *p != '>') p++;
                int attr_len = p - attr_start;
                
                if (attr_len > 0) {
                    char attr_buf[256] = {0};
                    if (attr_len < (int)sizeof(attr_buf) - 1) {
                        memcpy(attr_buf, attr_start, attr_len);
                        new_tag->attrs_used = parse_attributes(
                            attr_buf, new_tag->attrs, MAX_ATTRS);
                    }
                }
                p++; // Пропускаем '>'
            } 
            // Самозакрывающийся тег
            else if (*p == '/') {
                p += 2; // Пропускаем '/>'
            } 
            // Обычный открывающий тег
            else if (*p == '>') {
                p++;
            }
            
            // Добавляем в дерево
            if (stack_top >= 0) {
                struct tag *parent = stack[stack_top];
                
                // Увеличиваем массив детей
                struct tag *new_children = (struct tag *)realloc(
                    parent->children, 
                    (parent->childrens_num + 1) * sizeof(struct tag)
                );
                
                if (!new_children) {
                    free(new_tag);
                    return NULL;
                }
                
                parent->children = new_children;
                parent->children[parent->childrens_num] = *new_tag;
                parent->childrens_num++;
                
                // Освобождаем временный указатель
                free(new_tag);
                new_tag = &parent->children[parent->childrens_num - 1];
            } else {
                root = new_tag;
            }
            
            // Добавляем в стек для обработки вложенных тегов
            if (stack_top < (int)(sizeof(stack)/sizeof(stack[0])) - 1) {
                stack[++stack_top] = new_tag;
            } else {
                // Слишком глубокая вложенность
                free(new_tag);
                return NULL;
            }
        } 
        // Текстовое содержимое
        else {
            const char *text_start = p;
            while (*p && *p != '<') p++;
            size_t text_len = p - text_start;
            
            if (text_len > 0 && stack_top >= 0) {
                char *text = (char *)malloc(text_len + 1);
                if (text) {
                    memcpy(text, text_start, text_len);
                    text[text_len] = '\0';
                    trim_whitespace(text);
                    
                    // Сохраняем только если есть содержимое
                    if (text[0] != '\0') {
                        free(stack[stack_top]->content);
                        stack[stack_top]->content = text;
                    } else {
                        free(text);
                    }
                }
            }
        }
    }
    
    return root;
}

// Освобождение памяти (для полноты)
void free_tag(struct tag *t) {
    if (!t) return;
    
    for (u64 i = 0; i < t->childrens_num; i++) {
        free_tag(&t->children[i]);
    }
    
    free(t->children);
    free(t->content);
    free(t);
}
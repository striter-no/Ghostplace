#include <ghpl/utils/int.h>
#include <ghpl/utils/utf8.h>

int utf8_conv(const uint8_t *utf8_cs, int32_t **wide_cs) {
    // Проверка входных параметров
    if (!utf8_cs || !wide_cs) {
        return -1;
    }
    
    // Подсчитываем количество символов в строке (первый проход)
    size_t count = 0;
    const uint8_t *p = utf8_cs;
    i64 byte_len = strlen((const char *)utf8_cs);
    utf8proc_int32_t codepoint;
    i64 char_len;
    
    while (byte_len > 0) {
        char_len = utf8proc_iterate(p, byte_len, &codepoint);
        if (char_len <= 0) {
            return (int)char_len; // Ошибка декодирования UTF-8
        }
        count++;
        p += char_len;
        byte_len -= char_len;
    }
    
    // Выделяем память для массива code points (включая место для нуля)
    *wide_cs = (int32_t *)malloc((count + 1) * sizeof(int32_t));
    if (!*wide_cs) {
        return -2; // Ошибка выделения памяти
    }
    
    // Заполняем массив (второй проход)
    p = utf8_cs;
    byte_len = strlen((const char *)utf8_cs);
    int32_t *out = *wide_cs;
    
    while (byte_len > 0) {
        char_len = utf8proc_iterate(p, byte_len, &codepoint);
        if (char_len <= 0) {
            free(*wide_cs);
            *wide_cs = NULL;
            return (int)char_len;
        }
        *out++ = (int32_t)codepoint;
        p += char_len;
        byte_len -= char_len;
    }
    
    // Добавляем завершающий ноль
    *out = 0;
    
    return 0;
}

int utf8_nconv(const uint8_t *utf8_cs, int32_t **wide_cs, u64 byte_len) {
    // Проверка входных параметров
    if (!utf8_cs || !wide_cs) {
        return -1;
    }
    
    // Подсчитываем количество символов в строке (первый проход)
    size_t count = 0;
    const uint8_t *p = utf8_cs;
    utf8proc_int32_t codepoint;
    i64 char_len;
    u64 original_byte_len = byte_len;
    
    while (byte_len > 0) {
        char_len = utf8proc_iterate(p, byte_len, &codepoint);
        if (char_len <= 0) {
            return (int)char_len; // Ошибка декодирования UTF-8
        }
        count++;
        p += char_len;
        byte_len -= char_len;
    }
    
    // Выделяем память для массива code points (включая место для нуля)
    *wide_cs = (int32_t *)malloc((count + 1) * sizeof(int32_t));
    if (!*wide_cs) {
        return -2; // Ошибка выделения памяти
    }
    
    // Заполняем массив (второй проход)
    p = utf8_cs;
    byte_len = original_byte_len;
    int32_t *out = *wide_cs;
    
    while (byte_len > 0) {
        char_len = utf8proc_iterate(p, byte_len, &codepoint);
        if (char_len <= 0) {
            free(*wide_cs);
            *wide_cs = NULL;
            return (int)char_len;
        }
        *out++ = (int32_t)codepoint;
        p += char_len;
        byte_len -= char_len;
    }
    
    // Добавляем завершающий ноль
    *out = 0;
    
    return 0;
}

int cstr_conv(const int32_t *wide_cs, uint8_t **utf8_cs) {
    // Проверка входных параметров
    if (!wide_cs || !utf8_cs) {
        return -1;
    }
    
    // Подсчитываем необходимый размер буфера (первый проход)
    size_t total_bytes = 0;
    const int32_t *p = wide_cs;
    uint8_t temp_buf[4]; // Временный буфер для определения длины
    
    while (*p != 0) {
        i64 char_len = utf8proc_encode_char(*p, temp_buf);
        if (char_len < 0) {
            return (int)char_len; // Ошибка кодирования
        }
        total_bytes += char_len;
        p++;
    }
    
    // Выделяем память для UTF-8 строки (включая завершающий ноль)
    *utf8_cs = (uint8_t *)malloc(total_bytes + 1);
    if (!*utf8_cs) {
        return -2; // Ошибка выделения памяти
    }
    
    // Заполняем буфер (второй проход)
    p = wide_cs;
    uint8_t *out = *utf8_cs;
    
    while (*p != 0) {
        i64 char_len = utf8proc_encode_char(*p, out);
        if (char_len < 0) {
            free(*utf8_cs);
            *utf8_cs = NULL;
            return (int)char_len;
        }
        out += char_len;
        p++;
    }
    
    // Добавляем завершающий ноль
    *out = 0;
    
    return 0;
}

size_t utf32_count(const int32_t *s, int32_t c){
    size_t i = 0, o = 0;
    if (!s) return 0;

    while (s[i] && s[i] != '\0'){
        o += s[i++] == c;
    }
    return o;
}

// Аналог strlen для UTF-32
size_t utf32_strlen(const int32_t *s) {
    const int32_t *p = s;
    while (*p) p++;
    return (size_t)(p - s);
}

// Аналог strcpy для UTF-32
int32_t* utf32_strcpy(int32_t *dest, const int32_t *src) {
    int32_t *d = dest;
    while ((*d++ = *src++));
    return dest;
}

// Аналог strcmp для UTF-32
int utf32_strcmp(const int32_t *s1, const int32_t *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (int)(*s1 - *s2);
}

// Аналог strchr для UTF-32
int32_t* utf32_strchr(const int32_t *s, int32_t c) {
    while (*s && *s != c) s++;
    return (int32_t*)(*s ? s : NULL);
}

int32_t *utf32_strncpy(int32_t *dest, const int32_t *src, size_t n) {
    int32_t *original_dest = dest;
    size_t i;
    
    // Копируем символы, пока не достигнем конца строки или лимита n
    for (i = 0; i < n && src[i] != 0; i++) {
        dest[i] = src[i];
    }
    
    // Заполняем оставшееся пространство нулями, если нужно
    for (; i < n; i++) {
        dest[i] = 0;
    }
    
    return original_dest;
}

int32_t *utf32_strtok_r(int32_t *str, const int32_t *delim, int32_t **saveptr) {
    int32_t *current_pos;
    int32_t *token_start = NULL;
    
    // Определяем текущую позицию
    if (str != NULL) {
        current_pos = str;
    } else {
        if (*saveptr == NULL) {
            return NULL;
        }
        current_pos = *saveptr;
    }
    
    // Пропускаем разделители в начале
    bool is_delim;
    while (*current_pos != 0) {
        is_delim = false;
        const int32_t *d = delim;
        
        while (*d != 0) {
            if (*current_pos == *d) {
                is_delim = true;
                break;
            }
            d++;
        }
        
        if (!is_delim) {
            break;
        }
        
        current_pos++;
    }
    
    if (*current_pos == 0) {
        *saveptr = NULL;
        return NULL;
    }
    
    token_start = current_pos;
    
    // Ищем конец токена
    while (*current_pos != 0) {
        is_delim = false;
        const int32_t *d = delim;
        
        while (*d != 0) {
            if (*current_pos == *d) {
                is_delim = true;
                break;
            }
            d++;
        }
        
        if (is_delim) {
            *current_pos = 0;
            *saveptr = current_pos + 1;
            return token_start;
        }
        
        current_pos++;
    }
    
    *saveptr = NULL;
    return token_start;
}
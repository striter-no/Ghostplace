#include "../src/table.h"


struct Table create_table(u64 ksize, u64 vsize){
    struct Table tbl = {0};
    tbl.key_size = ksize;
    tbl.value_size = vsize;
    
    return tbl;
}

// Вспомогательная функция для поиска индекса ключа (устраняет дублирование)
static u64 find_key_index(struct Table *tb, const void *key) {
    for (u64 i = 0; i < tb->size; i++) {
        if (memcmp(key, (const ubyte*)tb->keys + i * tb->key_size, tb->key_size) == 0)
            return i;
    }
    return x_u64;
}

int table_in(struct Table *tb, const void *key) {
    return find_key_index(tb, key) != x_u64;
}

int table_add(struct Table *tb, const void *key, const void *val) {
    u64 kindex = find_key_index(tb, key);
    
    // Обновление существующего элемента
    if (kindex != x_u64) {
        memcpy((ubyte*)tb->keys + kindex * tb->key_size, key, tb->key_size);
        memcpy((ubyte*)tb->values + kindex * tb->value_size, val, tb->value_size);
        return 0;
    }

    // Проверка на переполнение
    assert(tb->size < SIZE_MAX / tb->key_size && 
           tb->size < SIZE_MAX / tb->value_size);

    // Выделяем память для нового элемента
    void *kbuffer = realloc(tb->keys, tb->key_size * (tb->size + 1));
    if (!kbuffer) return 1;
    
    void *vbuffer = realloc(tb->values, tb->value_size * (tb->size + 1));
    if (!vbuffer) {
        // Откатываем изменения при ошибке
        tb->keys = realloc(tb->keys, tb->key_size * tb->size);
        return 2;
    }

    // Копируем новые данные
    memcpy((ubyte*)kbuffer + tb->size * tb->key_size, key, tb->key_size);
    memcpy((ubyte*)vbuffer + tb->size * tb->value_size, val, tb->value_size);

    tb->keys = kbuffer;
    tb->values = vbuffer;
    tb->size++;
    return 0;
}

int table_rem(struct Table *tb, const void *key) {
    if (tb->size == 0) return 2;
    
    u64 kindex = find_key_index(tb, key);
    if (kindex == x_u64) return 1;

    // Правильный сдвиг элементов (исправлена критическая ошибка)
    for (u64 k = kindex; k < tb->size - 1; k++) {
        memcpy((ubyte*)tb->keys + k * tb->key_size,
               (ubyte*)tb->keys + (k + 1) * tb->key_size,
               tb->key_size);
        memcpy((ubyte*)tb->values + k * tb->value_size,
               (ubyte*)tb->values + (k + 1) * tb->value_size,
               tb->value_size);
    }

    tb->size--;  // Уменьшаем размер ДО попытки уменьшения памяти

    // Безопасное уменьшение памяти (ошибки не критичны)
    if (tb->size > 0) {
        void *kbuffer = realloc(tb->keys, tb->key_size * tb->size);
        void *vbuffer = realloc(tb->values, tb->value_size * tb->size);
        
        // Сохраняем новые указатели только при успехе
        if (kbuffer) tb->keys = kbuffer;
        if (vbuffer) tb->values = vbuffer;
    } else {
        free(tb->keys);
        free(tb->values);
        tb->keys = NULL;
        tb->values = NULL;
    }

    return 0;
}

int table_at(struct Table *tb, const void *key, void *val) {
    u64 kindex = find_key_index(tb, key);
    if (kindex == x_u64) return 1;
    
    memcpy(val, (const ubyte*)tb->values + kindex * tb->value_size, tb->value_size);
    return 0;
}

// Исправленная версия: передаем указатель на указатель
int table_ptr(struct Table *tb, const void *key, void **val) {
    u64 kindex = find_key_index(tb, key);
    if (kindex == x_u64) return 1;
    
    *val = (void*)((ubyte*)tb->values + kindex * tb->value_size);
    return 0;
}

void clear_table(struct Table *tb){

    free(tb->keys);
    free(tb->values);
    tb->keys = NULL;
    tb->values = NULL;
    tb->size = 0;
}
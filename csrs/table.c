#include "../src/table.h"


struct Table create_table(u64 ksize, u64 vsize){
    struct Table tbl = {0};
    tbl.key_size = ksize;
    tbl.value_size = vsize;
    
    return tbl;
}

int table_in(struct Table *tb, const void *key){
    for (u64 i = 0; i < tb->size; i++){
        if (0 == memcmp(key, (ubyte*)tb->keys + i * tb->key_size, tb->key_size)) 
            return 1;
    }
    return 0;
}

static u64 __key_index_table(struct Table *tb, const void *key){
    for (u64 i = 0; i < tb->size; i++){
        if (0 == memcmp(key, (ubyte*)tb->keys + i * tb->key_size, tb->key_size)) 
            return i;
    }
    return x_u64;
}

// static int __val_in_table(struct Table *tb, const void *val){
//     for (u64 i = 0; i < tb->size; i++){
//         if (0 == memcmp(val, (ubyte*)tb->values + i * tb->value_size, tb->value_size)) 
//             return 1;
//     }
//     return 0;
// }

int table_add(struct Table *tb, const void *key, const void *val){
    if (!table_in(tb, key)){
        void *kbuffer = realloc(tb->keys, tb->key_size * (tb->size + 1));
        if (kbuffer == NULL) return 1;

        void *vbuffer = realloc(tb->values, tb->value_size * (tb->size + 1));
        if (vbuffer == NULL) return 2;

        memcpy((ubyte*)kbuffer + tb->size * tb->key_size, key, tb->key_size);
        memcpy((ubyte*)vbuffer + tb->size * tb->value_size, val, tb->value_size);

        tb->size++;
        tb->keys = kbuffer;
        tb->values = vbuffer;

    } else {
        u64 kindex = __key_index_table(tb, key);
        rmove((ubyte*)tb->keys + kindex * tb->key_size, key, tb->key_size);
        rmove((ubyte*)tb->values + kindex * tb->value_size, val, tb->value_size);
    }

    return 0;
}

int table_rem(struct Table *tb, const void *key){
    if (tb->size == 0) return 2;

    if (!table_in(tb, key))
        return 1;

    u64 kindex = __key_index_table(tb, key);

    for (u64 k = kindex + 1; k < tb->size; k++){
        rmove(
            (ubyte*)tb->keys + (kindex - 1) * tb->key_size,
            (ubyte*)tb->keys + kindex * tb->key_size,
            tb->key_size
        );

        rmove(
            (ubyte*)tb->values + (kindex - 1) * tb->value_size,
            (ubyte*)tb->values + kindex * tb->value_size,
            tb->value_size
        );
    }

    if (tb->size > 1){
        void *kbuffer = realloc(tb->keys, tb->key_size * (tb->size - 1));
        if (kbuffer == NULL) return -1;

        void *vbuffer = realloc(tb->values, tb->value_size * (tb->size - 1));
        if (vbuffer == NULL) return -2;

        tb->keys = kbuffer;
        tb->values = vbuffer;
        tb->size--;

    } else {
        free(tb->values);
        free(tb->keys);
        tb->values = NULL;
        tb->keys = NULL;
        tb->size = 0;
    }

    return 0;
}

int table_at(struct Table *tb, const void *key, void *val){
    if (!table_in(tb, key))
        return 1;
    
    u64 kindex = __key_index_table(tb, key);
    memcpy(val, (ubyte*)tb->values + tb->value_size * kindex, tb->value_size);

    return 0;
}

void clear_table(struct Table *tb){

    free(tb->keys);
    free(tb->values);
    tb->keys = NULL;
    tb->values = NULL;
    tb->size = 0;
}

void rmove(void *to, void *from, u64 num){
    memcpy(to, from, num);
    free(to);
    to = NULL;
}

void bytearr(void *data, ubyte *buffer, u64 elsize, u64 size){
    buffer = (ubyte*)realloc(buffer, elsize * size);
    memcpy(buffer, data, elsize * size);
}
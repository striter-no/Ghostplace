#pragma once
#include <int.h>
#include <string.h>
#include <stdlib.h>

struct Table {
    u64 size;
    
    void *keys;
    u64 key_size;

    void *values;
    u64 value_size;
};

struct Table create_table(u64 ksize, u64 vsize);

int table_add(struct Table *tb, const void *key, const void *val);
int table_rem(struct Table *tb, const void *key);
int table_at(struct Table *tb, const void *key, void *val);
int table_in(struct Table *tb, const void *key);

void clear_table(struct Table *tb);
void rmove(void *src, void *dest, u64 num);
void bytearr(void *data, ubyte *buffer, u64 elsize, u64 size);
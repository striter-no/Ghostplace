#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <int.h>

#define MAX_ATTRS 7
#define MAX_CHILDREN 100
#define MAX_NAME 20
#define MAX_ATTR_NAME 15
#define MAX_ATTR_VALUE 100

struct attribute {
    char name[MAX_ATTR_NAME];
    char value[MAX_ATTR_VALUE];
};

struct tag {
    struct attribute attrs[MAX_ATTRS];
    u16 attrs_used;

    char *content;
    char name[MAX_NAME];
    struct tag *children;
    u64 childrens_num;
    u64 uid;
};

static u64 next_uid = 1;

// Утилиты для работы со строками
static void trim_whitespace(char *str);

static int safe_strncpy(char *dest, const char *src, size_t size);

// Парсинг атрибутов
static u16 parse_attributes(const char *str, struct attribute *attrs, u16 max_attrs);

// Основная функция парсинга
struct tag* parse_xml(const char *input);

// Освобождение памяти (для полноты)
void free_tag(struct tag *t);
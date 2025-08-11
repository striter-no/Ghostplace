#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ghpl/utils/int.h>

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
    struct tag **children;  // ИЗМЕНЕНО: теперь массив указателей
    size_t childrens_num;
    size_t uid;
};

struct tag* parse_xml(const char *input);
void free_tag(struct tag *t);
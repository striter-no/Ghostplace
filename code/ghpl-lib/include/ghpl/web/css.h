#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <ghpl/web/xml.h>  // Для использования struct attribute

struct css_block {
    struct attribute *attrs;
    u64 attrs_used;
    char *name;
};

struct css_block* parse_css(const char *css);
void free_css(struct css_block *blocks);
#pragma once
#include <string.h>
#include <stdlib.h>

void smart_strcat(char **output, char *to_add);
char str_startsw(const char *orig, const char *substr);
char str_endsw(const char *orig, const char *substr);
size_t toksplit(const char *src, const char *delim, char ***tokens);
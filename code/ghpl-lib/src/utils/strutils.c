#include <ghpl/utils/strutils.h>
#include <stddef.h>

void smart_strcat(char **output, char *to_add){
    (*output) = (char*)realloc((*output), strlen((*output)) + strlen(to_add) + 1);
    strcat((*output), to_add);
}

char str_startsw(const char *orig, const char *substr) {
    if (!orig || !substr) return 0;

    size_t i = 0;
    while (substr[i] != '\0') {
        if (orig[i] != substr[i]) {
            return 0;
        }
        i++;
    }
    return 1;
}

char str_endsw(const char *orig, const char *substr) {
    if (!orig || !substr) return 0;

    size_t orig_len = 0, substr_len = 0;

    while (orig[orig_len] != '\0') orig_len++;
    while (substr[substr_len] != '\0') substr_len++;

    if (substr_len > orig_len) return 0;

    size_t start_index = orig_len - substr_len;
    for (size_t i = 0; i < substr_len; i++) {
        if (orig[start_index + i] != substr[i]) {
            return 0;
        }
    }
    return 1;
}

size_t toksplit(const char *src, const char *delim, char ***tokens) {
    if (src == NULL || delim == NULL || tokens == NULL)
        return 0;

    *tokens = NULL;
    
    char *str_copy = strdup(src);
    if (str_copy == NULL)
        return 0;
    
    size_t count = 0;
    char *saveptr;
    char *token = strtok_r(str_copy, delim, &saveptr);
    
    while (token != NULL) {
        count++;
        token = strtok_r(NULL, delim, &saveptr);
    }
    
    if (count == 0) {
        free(str_copy);
        return 0;
    }
    
    *tokens = malloc(count * sizeof(char *));
    if (*tokens == NULL) {
        free(str_copy);
        return 0;
    }
    
    free(str_copy);
    str_copy = strdup(src);
    if (str_copy == NULL) {
        free(*tokens);
        *tokens = NULL;
        return 0;
    }
    
    size_t index = 0;
    token = strtok_r(str_copy, delim, &saveptr);
    
    while (token != NULL) {
        (*tokens)[index] = strdup(token);
        if ((*tokens)[index] == NULL) {
            for (size_t i = 0; i < index; i++)
                free((*tokens)[i]);

            free(*tokens);
            *tokens = NULL;
            free(str_copy);
            return 0;
        }
        index++;
        token = strtok_r(NULL, delim, &saveptr);
    }
    
    free(str_copy);
    return count;
}

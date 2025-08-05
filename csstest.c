#include <css.h>

int main() {
    char *css;
    readfile("./assets/pages/test.gss", &css);
    
    struct css_block *blocks = parse_css(css);
    free(css);
    
    if (!blocks) {
        printf("Failed to parse CSS\n");
        return 1;
    }
    
    for (u64 i = 0; blocks[i].name != NULL; i++) {
        printf("CSS Block: %s\n", blocks[i].name);
        
        for (u64 j = 0; j < blocks[i].attrs_used; j++) {
            printf("  %s = \"%s\"\n", 
                   blocks[i].attrs[j].name, 
                   blocks[i].attrs[j].value);
        }
    }
    
    free_css(blocks);
    return 0;
}
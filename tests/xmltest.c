#include <xml.h>


void showstruct(
    struct tag *root,
    int level
){
    for (int i = 0; i < level; i++) printf("  ");
    printf("Tag \"%s\" (uid: %llu)\n", root->name, root->uid);
    for (int i = 0; i < root->attrs_used; i++){
        for (int i = 0; i <= level; i++) printf("  ");
        printf("Attribute: %s=\"%s\"\n", root->attrs[i].name, root->attrs[i].value);
    }
    if (root->content && strlen(root->content) != 0){
        for (int i = 0; i < level; i++) printf("  ");
        printf("Content: %s\n", root->content);
    }

    for (int i = 0; i < root->childrens_num; i++){
        showstruct(root->children[i], level + 1);
    }
}

int main() {
    char *xml;
    readfile("./assets/pages/test.ghml", &xml);
    
    struct tag *root = parse_xml(xml);
    free(xml);
    if (!root) {
        printf("Failed to parse XML\n");
        return 1;
    }

    showstruct(root, 0);
    
    free_tag(root);
    return 0;
}
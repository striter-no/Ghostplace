#include <xml.h>

int main() {
    const char *xml = 
        "<main>\n"
        "     <dif pos=\"left\">\n"
        "          <text>\n"
        "             Lorem ipsum...\n"
        "          </text>\n"
        "     </dif>\n"
        "</main>";
    
    struct tag *root = parse_xml(xml);
    if (!root) {
        printf("Failed to parse XML\n");
        return 1;
    }
    
    // Вывод структуры
    printf("Root tag: %s (UID: %llu)\n", root->name, root->uid);
    
    if (root->childrens_num > 0) {
        struct tag *child1 = root->children[0];
        printf("  Child 1: %s (UID: %llu)\n", child1->name, child1->uid);
        
        if (child1->attrs_used > 0) {
            printf("    Attribute: %s=\"%s\"\n", 
                   child1->attrs[0].name, child1->attrs[0].value);
        }
        
        if (child1->childrens_num > 0) {
            struct tag *child2 = child1->children[0];
            printf("    Child 2: %s (UID: %llu)\n", child2->name, child2->uid);
            
            if (child2->content) {
                printf("      Content: \"%s\"\n", child2->content);
            }
        }
    }
    
    free_tag(root);
    return 0;
}
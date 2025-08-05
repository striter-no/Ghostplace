#include <web/parser.h>

void xml_addwidgets(
    struct tgr_app *app,
    struct Widget **ptr,
    struct tag *root,
    int level
){    
    // printf("[log] adding tag %s\n", root->name);
    if (level == 0 && strcmp(root->name, "cnt") != 0){
        fprintf(stderr, "First tag has to be \"cnt\", not a \"%s\"\n", root->name);
        exit(-1);
    }

    init_widget(root, ptr);
    struct Widget *cwg = *ptr;
    
    if (level == 0){
        cwg->rect.w = app->TERM_WIDTH - 1;
        cwg->rect.h = app->TERM_HEIGHT;
    }
    
    for (int i = 0; i < root->childrens_num; i++){
        struct Widget *wg = NULL;
        xml_addwidgets(app, &wg, root->children[i], level + 1);
        
        if (cwg->wgtype == CONTAINER_WIDGET){
            u64 uid;
            add_widget(cwg, *wg, &uid, gmargin(M_LEFT, M_UP, 0, 0));

            free_widget(wg);
            free(wg);
        } else {
            free_widget(wg);
            free(wg);

            fprintf(stderr, "[xml_addwidgets][error] no widgets excpet containers can have children widgets\n");
            exit(-12);
        }
    }

    adjust_rect(cwg);
}

void init_widget(struct tag *tag, struct Widget **out){
    if (strcmp(tag->name, "cnt") == 0){
        init_container(tag, out);
    } else if (strcmp(tag->name, "box") == 0){
        init_box(tag, out);
    } else if (strcmp(tag->name, "text") == 0){
        init_text(tag, out);
    } else if (strcmp(tag->name, "img") == 0){
        init_img(tag, out);
    } else {
        fprintf(stderr, "[init_widget][error] widget from tag \"%s\" - unknown widget\n", tag->name);
        exit(-2);
    }
}

void init_container(struct tag *tag, struct Widget **out){
    struct Rect rect = {0, 0, -1, -1};
    enum WG_CONTAINER_POS pos = CWG_VERTICLLY;
    byte scrollable = 1;

    char *class = NULL;
    for (int i = 0; i < tag->attrs_used; i++){
        struct attribute *attr = &tag->attrs[i];
        
        if (strcmp(attr->name, "class") == 0){
            class = malloc(strlen(attr->value) + 1);
            strcpy(class, attr->value);
        } else if (strcmp(attr->name, "store") == 0){
            pos = (strcmp(attr->value, "hz") == 0) ? CWG_HORIZONTALLY: CWG_VERTICLLY; 
        } else if (strcmp(attr->name, "scroll") == 0){
            scrollable = strcmp(attr->value, "true") == 0;
        } else if (strcmp(attr->name, "w") == 0){
            rect.w = atoi(attr->value);
        } else if (strcmp(attr->name, "h") == 0){
            rect.h = atoi(attr->value);
        } else {
            fprintf(stderr, "[init_container][error] container attr \"%s\" is unknown\n", attr->name);
            exit(-3);
        }
    }

    create_widget(out, CONTAINER_WIDGET, rect);
    create_cont((*out)->wgdata, pos, scrollable);
    
    if (class){
        strcpy((*out)->class, class);
        free(class);
    }
    
}

void init_box(struct tag *tag, struct Widget **out){
    struct Rect rect = {0, 0, -1, -1};
    struct Rect srect = {0, 0, 2, 2};
    enum BOX_TYPE type = BOX_ONE_LINE;
    struct rgb color = {255, 255, 255};

    char *class = NULL;
    for (int i = 0; i < tag->attrs_used; i++){
        struct attribute *attr = &tag->attrs[i];

        if (strcmp(attr->name, "class") == 0){
            class = malloc(strlen(attr->value) + 1);
            strcpy(class, attr->value);
        } else if (strcmp(attr->name, "w") == 0){
            srect.w = atoi(attr->value);
        } else if (strcmp(attr->name, "h") == 0){
            srect.h = atoi(attr->value);
        } else if (strcmp(attr->name, "clr") == 0){
            int r, g, b;
            if (sscanf(attr->value, "%d %d %d", &r, &g, &b) != 3){
                fprintf(stderr, "[init_box][error] clr needs to be in format \"R G B\", but it is: \"%s\"\n", attr->value);
                exit(-5);
            }
            color.r = r;
            color.g = g;
            color.b = b;
        } else if (strcmp(attr->name, "type") == 0){
            if (strcmp(attr->value, "one") == 0){
                type = BOX_ONE_LINE;
            } else if (strcmp(attr->value, "double") == 0){
                type = BOX_DOUBLE;
            } else if (strcmp(attr->value, "round") == 0){
                type = BOX_ROUNDED;
            } else if(strcmp(attr->value, "prim")){
                type = BOX_PRIMITIVE;
            } else {
                fprintf(stderr, "[init_box][error] box type \"%s\" is unknown\n", attr->value);
                exit(-13);
            }
        } else {
            fprintf(stderr, "[init_box][error] box attr \"%s\" is unknown\n", attr->name);
            exit(-4);
        }
    }

    create_widget(out, BOX_WIDGET, rect);
    if (class){
        strcpy((*out)->class, class);
        free(class);
    }
    
    *(struct Box *)((*out)->wgdata) = (struct Box){
        .color = color,
        .type = type,
        .srect = srect
    };
}

void init_text(struct tag *tag, struct Widget **out){
    struct Rect rect = {0, 0, -1, -1};
    struct rgb color = {255, 255, 255};
    int style_inx = NO_STYLE;
    char *content_bytes;

    char *class = NULL;
    for (int i = 0; i < tag->attrs_used; i++){
        struct attribute *attr = &tag->attrs[i];

        if (strcmp(attr->name, "class") == 0){
            class = malloc(strlen(attr->value) + 1);
            strcpy(class, attr->value);
        } else if (strcmp(attr->name, "style") == 0){
            if (strcmp(attr->value, "bold") == 0)
                style_inx = STYLE_BOLD;
            else if (strcmp(attr->value, "dim") == 0)
                style_inx = STYLE_DIM;
            else if (strcmp(attr->value, "cursive") == 0)
                style_inx = STYLE_CURSIVE;
            else if (strcmp(attr->value, "underline") == 0)
                style_inx = STYLE_UNDERLINE;
            else if (strcmp(attr->value, "blink") == 0)
                style_inx = STYLE_BLINK;
            else if (strcmp(attr->value, "inv") == 0)
                style_inx = STYLE_INVERSE;
            else if (strcmp(attr->value, "strike") == 0)
                style_inx = STYLE_STRIKE;
            else if (strcmp(attr->value, "normal") == 0)
                style_inx = NO_STYLE;
            else {
                fprintf(stderr, "[init_txt][error] style \"%s\" is unknown\n", attr->value);
                exit(-7);
            }
        } else if (strcmp(attr->name, "clr") == 0){
            int r, g, b;
            if (sscanf(attr->value, "%d %d %d", &r, &g, &b) != 3){
                fprintf(stderr, "[init_txt][error] clr needs to be in format \"R G B\", but it is: \"%s\"\n", attr->value);
                exit(-6);
            }
            color.r = r;
            color.g = g;
            color.b = b;
        } else if (strcmp(attr->name, "w") == 0){
            rect.w = atoi(attr->value);
        } else if (strcmp(attr->name, "h") == 0){
            rect.h = atoi(attr->value);
        } else {
            fprintf(stderr, "[init_text][error] text attr \"%s\" is unknown\n", attr->name);
            exit(-5);
        }
    }

    if (!tag->content){
        fprintf(stderr, "[init_text][error] there is no content in text tag\n");
        exit(-8);
    }

    create_widget(out, TEXT_WIDGET, rect);
    if (class){
        strcpy((*out)->class, class);
        free(class);
    }
    
    *(struct Text *)((*out)->wgdata) = (struct Text){
        .base_clr = color,
        .style = styles[style_inx],
        .unicode_txt = NULL
    };
    utf8_conv(tag->content, &(((struct Text *)((*out)->wgdata))->unicode_txt));
    // printf("adding: \"%s\"\n", tag->content);
}

void init_img(struct tag *tag, struct Widget **out){
    struct Rect rect = {0, 0, -1, -1};
    struct rgb color = {255, 255, 255};
    byte is_dense = 0;
    char *path = NULL; // pointer, not cstr

    char *class = NULL;
    for (int i = 0; i < tag->attrs_used; i++){
        struct attribute *attr = &tag->attrs[i];

        if (strcmp(attr->name, "class") == 0){
            class = malloc(strlen(attr->value) + 1);
            strcpy(class, attr->value);
        } else if (strcmp(attr->name, "dense") == 0){
            is_dense = strcmp(attr->value, "true") == 0;
        } else if (strcmp(attr->name, "src") == 0){
            path = attr->value;
        } else if (strcmp(attr->name, "clr") == 0){
            int r, g, b;
            if (sscanf(attr->value, "%d %d %d", &r, &g, &b) != 3){
                fprintf(stderr, "[init_img][error] clr needs to be in format \"R G B\", but it is: \"%s\"\n", attr->value);
                exit(-10);
            }
            color.r = r;
            color.g = g;
            color.b = b;
        } else {
            fprintf(stderr, "[init_img][error] img attr \"%s\" is unknown\n", attr->name);
            exit(-9);
        }
    }

    if (path == NULL){
        fprintf(stderr, "[init_img][error] img without \"src\" attribute is not possible\n");
        exit(-11);
    }

    create_widget(out, IMAGE_WIDGET, rect);
    if (class){
        strcpy((*out)->class, class);
        free(class);
    }
    
    *(struct Image *)((*out)->wgdata) = (struct Image){
        .base_clr = color,
        .is_dense = is_dense,
        .img = {0}
    };
    img_load(path, &(((struct Image*)((*out)->wgdata))->img), 0);
}
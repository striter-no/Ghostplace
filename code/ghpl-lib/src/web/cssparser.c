#include <ghpl/web/cssparser.h>

void __get_widgets(struct Widget *main_cnt, struct Widget ***widgets, u64 *widgets_sz){
    // !*printf("[__get_widgets][log] iter... %llu\n", main_cnt->wgtype);

    struct Container *cnt = main_cnt->wgdata;
    u64 old_size = *widgets_sz;
    *widgets_sz += cnt->widgets.size;
    
    struct Widget **buffer = (struct Widget **)realloc(*widgets, sizeof(struct Widget*) * (*widgets_sz));
    if (!buffer){
        fprintf(stderr, "[__get_widgets][failure] realloc\n");
        exit(-40);
    }
    *widgets = buffer;

    for (u64 i = 0; i < cnt->widgets.size; i++){
        struct ExtCWidget *wg;
        table_ptr(&cnt->widgets, (ubyte*)cnt->widgets.keys + i * cnt->widgets.key_size, (void**)&wg);
        
        (*widgets)[old_size + i] = &wg->widget;
        if (wg->widget.wgtype == CONTAINER_WIDGET)
            __get_widgets(&wg->widget, widgets, widgets_sz);
    }
}

void __get_relp(struct Widget *cntwg, u64 uid, struct WidgetRelp **out){
    struct Container *cnt = cntwg->wgdata;
    for (u64 i = 0; i < cnt->widgets.size; i++){
        struct ExtCWidget *wg;
        table_ptr(&cnt->widgets, (ubyte*)cnt->widgets.keys + i * cnt->widgets.key_size, (void**)&wg);

        if (wg->widget.uid == uid){
            *out = &wg->positioning;
            return;
        }

        if (wg->widget.wgtype == CONTAINER_WIDGET){
            struct WidgetRelp *temp = NULL;
            __get_relp(&wg->widget, uid, &temp);
            if (temp != NULL) {
                *out = temp;
                return;
            }
        }
    }
    *out = NULL;
}

const char *__str_wgtype(enum WIDGET_TYPE type){
    switch (type){
        case TEXT_WIDGET: return "text";
        case BOX_WIDGET: return "box";
        case IMAGE_WIDGET: return "image";
        case CONTAINER_WIDGET: return "container";
        case WIDGET_UNDEFINED: return "undef";
    }
    return "undef";
}

void css_parsewidgets(
    struct Widget **main_cnt,
    struct css_block *blocks
){
    struct Widget **widgets = (struct Widget **)malloc(sizeof(struct Widget*));
    u64 size = 1;

    widgets[0] = (*main_cnt);
    __get_widgets((*main_cnt), &widgets, &size);

    for (u64 i = 0; blocks[i].name != NULL; i++){
        struct css_block *curr = &blocks[i];
        // !*printf("[css] block: %s\n", curr->name);

        enum RELP_ENUM relp[2] = {M_RELP_UNDEFINED, M_RELP_UNDEFINED};
        f32 relp_v[2] = {-1, -1};

        enum BOX_TYPE box_style = BOX_TYPE_UNDEFINED;
        struct rgb color = {-1, -1, -1}; 

        enum BOX_TYPE border_style = BOX_TYPE_UNDEFINED;

        int txt_style = -1;

        i16 h_absolute = -1, v_absolute = -1;
        byte has_habs = 0, has_vabs = 0;
        byte fixed = 0;

        for (u64 k = 0; k < curr->attrs_used; k++){
            struct attribute *attr = &curr->attrs[k];
            // !*printf("[css] attr: %s = %s\n", attr->name, attr->value);
            if (strcmp(attr->name, "h.absolute") == 0){
                h_absolute = atoi(attr->value);
                has_habs = 1;
            } else if (strcmp(attr->name, "v.absolute") == 0){
                v_absolute = atoi(attr->value);
                has_vabs = 1;
            } else if (strcmp(attr->name, "fixed") == 0) {
                fixed = strcmp(attr->value, "true") == 0;
            } else if (strcmp(attr->name, "mg.right") == 0){
                relp[0] = M_RIGHT;
                relp_v[0] = atof(attr->value);
            } else if (strcmp(attr->name, "mg.left") == 0){
                relp[0] = M_LEFT;
                relp_v[0] = atof(attr->value);
            } else if (strcmp(attr->name, "mg.up") == 0){
                relp[1] = M_UP;
                relp_v[1] = atof(attr->value);
            } else if (strcmp(attr->name, "mg.down") == 0){
                relp[1] = M_DOWN;
                relp_v[1] = atof(attr->value);
            } else if (strcmp(attr->name, "mg.hcenter") == 0){
                relp[0] = M_HCENTER;
                relp_v[0] = atof(attr->value);
            } else if (strcmp(attr->name, "mg.vcenter") == 0){
                relp[1] = M_VCENTER;
                relp_v[1] = atof(attr->value);
            } else if (strcmp(attr->name, "clr.r") == 0){
                color.r = atoi(attr->value);
            } else if (strcmp(attr->name, "clr.g") == 0){
                color.g = atoi(attr->value);
            } else if (strcmp(attr->name, "clr.b") == 0){
                color.b = atoi(attr->value);
            } else if (strcmp(attr->name, "txt.style") == 0){
                if (strcmp(attr->value, "bold") == 0)
                    txt_style = STYLE_BOLD;
                else if (strcmp(attr->value, "dim") == 0)
                    txt_style = STYLE_DIM;
                else if (strcmp(attr->value, "cursive") == 0)
                    txt_style = STYLE_CURSIVE;
                else if (strcmp(attr->value, "underline") == 0)
                    txt_style = STYLE_UNDERLINE;
                else if (strcmp(attr->value, "blink") == 0)
                    txt_style = STYLE_BLINK;
                else if (strcmp(attr->value, "inv") == 0)
                    txt_style = STYLE_INVERSE;
                else if (strcmp(attr->value, "strike") == 0)
                    txt_style = STYLE_STRIKE;
                else if (strcmp(attr->value, "normal") == 0)
                    txt_style = NO_STYLE;
                else {
                    fprintf(stderr, "[css_parse][error] text style \"%s\" is unknown\n", attr->value);
                    exit(-7);
                }
            } else if (strcmp(attr->name, "box.style") == 0){
                if (strcmp(attr->value, "one") == 0){
                    box_style = BOX_ONE_LINE;
                } else if (strcmp(attr->value, "double") == 0){
                    box_style = BOX_DOUBLE;
                } else if (strcmp(attr->value, "round") == 0){
                    box_style = BOX_ROUNDED;
                } else if(strcmp(attr->value, "prim")){
                    box_style = BOX_PRIMITIVE;
                } else {
                    fprintf(stderr, "[css_parse][error] box type \"%s\" is unknown\n", attr->value);
                    exit(-13);
                }
            } else if (strcmp(attr->name, "brd.style") == 0){
                if (strcmp(attr->value, "one") == 0){
                    border_style = BOX_ONE_LINE;
                } else if (strcmp(attr->value, "double") == 0){
                    border_style = BOX_DOUBLE;
                } else if (strcmp(attr->value, "round") == 0){
                    border_style = BOX_ROUNDED;
                } else if(strcmp(attr->value, "prim")){
                    border_style = BOX_PRIMITIVE;
                } else {
                    fprintf(stderr, "[css_parse][error] border type \"%s\" is unknown\n", attr->value);
                    exit(-13);
                }
            } else {
                fprintf(stderr, "[css_parse][error] css attribute \"%s\" is unknown\n", attr->name);
                exit(-20);
            } 
        }

        for (u64 i = 0; i < size; i++){
            struct Widget **wg = &widgets[i];

            if (strcmp(curr->name, "all") != 0){
                if (
                !(curr->name[0] == '#' && 0 == strcmp(curr->name + 1, (*wg)->class)) && 
                !(curr->name[0] != '#' && strcmp(curr->name, __str_wgtype((*wg)->wgtype)) == 0)
                )
                continue;
            }
            
            // !*printf("[css] block %s passed\n", curr->name);            

            for (int _i = 0; _i < 2; _i++){
                // !*printf("[css][2log] %d\n", _i);
                if (relp[_i] != M_RELP_UNDEFINED){
                    struct WidgetRelp *prelp;
                    __get_relp(*main_cnt, (*wg)->uid, &prelp);

                    // Проверяем, указывает ли prelp на освобожденную память
                    if (prelp == NULL){
                        // printf("[warning][css_parse][failure] widget with UID %llu was not found\n", (*wg)->uid);
                        continue;
                    }

                    prelp->h_absolute = h_absolute;
                    prelp->v_absolute = v_absolute;
                    prelp->has_habs = has_habs;
                    prelp->has_vabs = has_vabs;
                    prelp->is_fixed = fixed;
                    // !*printf("[css][log] setting %d to relp: ", relp_v[_i]);
                    switch (relp[_i]){
                        case M_RIGHT:
                            // printf("right\n");
                            prelp->margin_right = relp_v[_i];
                            prelp->margin_left = -1;
                            prelp->margin_hcenter = -1;
                            break;
                        case M_LEFT:
                            // printf("left\n");
                            prelp->margin_right = -1;
                            prelp->margin_left = relp_v[_i];
                            prelp->margin_hcenter = -1;
                            break;
                        case M_UP:
                            // printf("up\n");
                            prelp->margin_up = relp_v[_i];
                            prelp->margin_down = -1;
                            prelp->margin_vcenter = -1;
                            break;
                        case M_DOWN:
                            // printf("down\n");
                            prelp->margin_up = -1;
                            prelp->margin_down = relp_v[_i];
                            prelp->margin_vcenter = -1;
                            break;
                        case M_HCENTER:
                            // printf("hcenter\n");
                            prelp->margin_right = -1;
                            prelp->margin_left = -1;
                            prelp->margin_hcenter = relp_v[_i];
                            break;
                        case M_VCENTER:
                            // printf("vcenter\n");
                            prelp->margin_up = -1;
                            prelp->margin_down = -1;
                            prelp->margin_vcenter = relp_v[_i];
                            break;
                        
                        case M_RELP_UNDEFINED:
                        default:
                            break;
                    }
                }
            }
            
            switch ((*wg)->wgtype){
                case TEXT_WIDGET: {
                    // !*printf("[css][log] text widget\n");
                    struct Text* raw = (struct Text*)((*wg)->wgdata);
                    if (color.r != -1) raw->base_clr.r = color.r;
                    if (color.g != -1) raw->base_clr.g = color.g;
                    if (color.b != -1) raw->base_clr.b = color.b;
                    if (txt_style != -1) raw->style = styles[txt_style];
                    break;
                }
                case IMAGE_WIDGET: {
                    // !*printf("[css][log] image widget\n");
                    struct Image* raw = (struct Image*)((*wg)->wgdata);
                    if (color.r != -1) raw->base_clr.r = color.r;
                    if (color.g != -1) raw->base_clr.g = color.g;
                    if (color.b != -1) raw->base_clr.b = color.b;
                    break;
                }
                case BOX_WIDGET: {
                    // !*printf("[css][log] box widget\n");
                    struct Box* raw = (struct Box*)((*wg)->wgdata);
                    if (color.r != -1) raw->color.r = color.r;
                    if (color.g != -1) raw->color.g = color.g;
                    if (color.b != -1) raw->color.b = color.b;
                    if (box_style != BOX_TYPE_UNDEFINED) raw->type = box_style;
                    break;
                }
                case CONTAINER_WIDGET: {
                    struct Container* raw = (struct Container*)((*wg)->wgdata);
                    if (color.r != -1) raw->border_clr.r = color.r;
                    if (color.g != -1) raw->border_clr.g = color.g;
                    if (color.b != -1) raw->border_clr.b = color.b;
                    if (box_style != BOX_TYPE_UNDEFINED) raw->border_type = border_style;
                }

                case WIDGET_UNDEFINED:
                default:
                    break;
            }
        }

    }
    free(widgets);
}
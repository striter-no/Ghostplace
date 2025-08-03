#include <web/widgets/widget.h>

// ================================ CONTAINER ===================================

void create_cont(
    struct Container *cont,
    enum WG_CONTAINER_POS storing
){
    cont->widgets = create_table(sizeof(u64), sizeof(struct ExtCWidget));
    cont->storing_wgc = storing;
}

void free_container(
    struct Container *cont
){
    if (!cont) return;
    struct Table *tb = &cont->widgets;

    for (u64 i = 0; i < tb->size; i++){
        struct ExtCWidget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);
        free_widget(&wg.widget);
    }

    clear_table(tb);
}

void container_cpy(
    struct Container *dst,
    struct Container *src
){
    dst->storing_wgc = src->storing_wgc;
    dst->widgets = create_table(src->widgets.key_size, src->widgets.value_size);
    
    struct Table *tbd = &dst->widgets;
    struct Table *tb =  &src->widgets;
    for (u64 i = 0; i < tb->size; i++){
        struct ExtCWidget wg, nwg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);

        nwg.positioning = wg.positioning;
        widget_copy(&nwg.widget, &wg.widget);
        table_add(tbd, (ubyte*)tb->keys + i * tb->key_size, &nwg);
    }
}

void upd_contianer(
    struct Widget *cont_wg
){
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;

    for (u64 i = 0; i < tb->size; i++){
        struct ExtCWidget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);
        
        u64 h_dt = cont_wg->orig_state.x - wg.widget.orig_state.x;
        u64 v_dt = cont_wg->orig_state.y - wg.widget.orig_state.y;
        
        wg.widget.rect.x = cont_wg->rect.x + h_dt;
        wg.widget.rect.y = cont_wg->rect.y + v_dt;

        if (wg.widget.wgtype == CONTAINER_WIDGET){
            upd_contianer(&wg.widget);
        }
    }
}

void add_widget(
    struct Widget *cont_wg,
    struct Widget to_add,
    u64           *uid,
    struct WidgetRelp relp
){
    if (cont_wg->wgtype != CONTAINER_WIDGET) 
        return;
    
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;

    struct ExtCWidget ewg = {0};
    struct Widget copy = {0};
    widget_copy(&copy, &to_add);

    ewg.positioning = relp;
    ewg.widget = copy;

    *uid = rand() % x_u64;
    table_add(tb, uid, &ewg);
}

void rem_widget(
    struct Widget *cont_wg,
    u64           uid
){
    if (cont_wg->wgtype != CONTAINER_WIDGET) 
        return;
    
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;

    table_rem(tb, &uid);
}

void draw_container(
    struct tgr_app *app, 
    const struct Container *cont, 
    struct Rect rect
){
    struct Table *tb = &cont->widgets;

    u64 offset_x = 0, offset_y = 0;
    for (u64 i = 0; i < tb->size; i++){
        struct ExtCWidget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);

        struct Rect *rwg = &wg.widget.rect;

        // X positioning
        switch (wg.positioning.hr){
            case NORMAL_H: {
                rwg->x = rect.x + offset_x;
                break;
            }
            case ABSOLUTE:
                rwg->x = rect.x;
                break;
        }

        // Y positioning
        switch (wg.positioning.vr){
            case NORMAL_V: {
                rwg->y = rect.y + offset_y;
                if (cont->storing_wgc == CWG_VERTICLLY)
                    offset_y += rwg->h;
                else
                    offset_x += rwg->w;
                break;
            }
            case ABSOLUTE:
                rwg->y = rect.y;
                break;
        }
        
        draw_widget(app, &wg.widget);
    }
}


// ============================= WIDGET ==================================

void create_widget(
    struct Widget **out,
    enum WIDGET_TYPE type,
    struct Rect rect
){
    u64 bytes = 0;
    switch (type) {
        case TEXT_WIDGET:
            bytes = sizeof(struct Text); break;
        case BOX_WIDGET:
            bytes = sizeof(struct Box); break;
        case IMAGE_WIDGET:
            bytes = sizeof(struct Image); break;
        case CONTAINER_WIDGET:
            bytes = sizeof(struct Container); break;
        case WIDGET_UNDEFINED:
            return;
    }

    (*out) = (struct Widget*)malloc(sizeof(struct Widget));
    (*out)->wgtype = type;
    (*out)->typesize = bytes;
    (*out)->wgdata = malloc(bytes);
    (*out)->orig_state = rect;
    (*out)->rect       = rect;
}

void adjust_rect(
    struct Widget *widget
){
    struct Rect rect = widget->rect;
    if (rect.w == -1 || rect.h == -1){
        switch (widget->wgtype){
            case IMAGE_WIDGET: {
                rect.w = (rect.w == -1) ? ((struct Image*)(widget->wgdata))->img.width : rect.w;
                rect.h = (rect.h == -1) ? ((struct Image*)(widget->wgdata))->img.height : rect.h;
                break;
            }
            case TEXT_WIDGET: {
                struct Text *txt = widget->wgdata;

                if (rect.w == -1){
                    u64 mx_sz = 0, crr_sz = 0;
                    for (u64 i = 0; i < utf32_strlen(txt->unicode_txt); i++){
                        if (txt->unicode_txt[i] == '\n'){
                            mx_sz = max(mx_sz, crr_sz);
                            crr_sz = 0;
                            continue;
                        }
                        crr_sz++;
                    }
                    rect.w = mx_sz;
                }

                if (rect.h == -1){
                    u64 x = 0, y = 0;
                    u64 bx = rect.w; // borders
                    for (u64 i = 0; i < utf32_strlen(txt->unicode_txt); i++) {
                        uint32_t codepoint = (uint32_t)txt->unicode_txt[i];
                        
                        if (codepoint == '\n'){
                            x = 0;
                            y++;
                            continue;
                        }

                        if (x >= bx){
                            x = 0;
                            y++;
                        }
                        x++;
                    }
                    rect.h = y + 1;
                }
                break;
            }
            case CONTAINER_WIDGET: {
                u64 mx_width = 0, mx_height = 0;
                u64 offset_x = 0, offset_y = 0;
                
                struct Container *cnt = widget->wgdata;
                struct Table     *tb  = &cnt->widgets;
                for (u64 i = 0; i < tb->size; i++){
                    struct ExtCWidget wg;
                    table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);

                    struct Rect rwg = wg.widget.rect;
                    // X positioning
                    switch (wg.positioning.hr){
                        case NORMAL_H: {
                            rwg.x = rect.x + offset_x;
                            break;
                        }
                        case ABSOLUTE:
                            rwg.x = rect.x;
                            break;
                    }

                    // Y positioning
                    switch (wg.positioning.vr){
                        case NORMAL_V: {
                            rwg.y = rect.y + offset_y;
                            if (cnt->storing_wgc == CWG_VERTICLLY)
                                offset_y += rwg.h;
                            else
                                offset_x += rwg.w;
                            break;
                        }
                        case ABSOLUTE:
                            rwg.y = rect.y;
                            break;
                    }

                    mx_width = max(mx_width, rwg.x + rwg.w);
                    mx_height = max(mx_height, rwg.y + rwg.h);
                }

                rect.w = mx_width;
                rect.h = mx_height;
                break;
            }
            default: {
                rect.w = 1;
                rect.h = 1;
            }
        }
    }
    widget->rect = rect;
}

void free_widget(
    struct Widget *widget
){
    if (!widget->wgdata) return;

    switch (widget->wgtype){
        case IMAGE_WIDGET: {
            img_free(widget->wgdata);
            break;
        }
        case TEXT_WIDGET: {
            free_text_wg(widget->wgdata);
            break;
        }
        case CONTAINER_WIDGET: {
            free_container(widget->wgdata);
            break;
        }
    }

    free(widget->wgdata);
    widget->wgdata = NULL;
    widget->typesize = 0;
    widget->rect = (struct Rect){0};
    widget->wgtype = WIDGET_UNDEFINED;
}

void draw_widget(
    struct tgr_app *app,
    const struct Widget *widg
){
    switch (widg->wgtype){
        case TEXT_WIDGET: {
            draw_text(app, widg->wgdata, widg->rect);
            break;
        }
        case IMAGE_WIDGET: {
            draw_image(app, widg->wgdata, widg->rect);
            break;
        }
        case BOX_WIDGET: {
            draw_box(app, widg->wgdata, widg->rect);
            break;
        }
        case CONTAINER_WIDGET: {
            draw_container(app, widg->wgdata, widg->rect);
            break;
        }
        case WIDGET_UNDEFINED: {
            break;
        }
    }
}

void widget_copy(
    struct Widget *dest,
    struct Widget *src
){
    dest->orig_state = src->orig_state;
    dest->rect = src->rect;
    dest->typesize = src->typesize;
    dest->wgtype = src->wgtype;
    dest->wgdata = malloc(src->typesize);
    
    switch (dest->wgtype){
        case IMAGE_WIDGET: {
            imgwg_cpy(dest->wgdata, src->wgdata);
            break;
        }
        case TEXT_WIDGET: {
            text_cpy(dest->wgdata, src->wgdata);
            break;
        }
        case BOX_WIDGET: {
            box_cpy(dest->wgdata, src->wgdata);
            break;
        }
        case CONTAINER_WIDGET: {
            container_cpy(dest->wgdata, src->wgdata);
            break;
        }
        default: {
            memcpy(dest->wgdata, src->wgdata, src->typesize);
        }
    }
}

// ============================== RECT ===========================

struct Rect rect_intersection(
    struct Rect r1, struct Rect r2
){
    u64 ri1 = r1.x + r1.w, 
        bo1 = r1.y + r1.h,
        ri2 = r2.x + r2.w,
        bo2 = r2.y + r2.h;
    
    u64 left = max(r1.x, r2.x),
        top  = max(r1.y, r2.y),
        right = min(ri1, ri2),
        bottom = min(bo1, bo2);
    
    if (left >= right || top >= bottom)
        return (struct Rect){0};
    
    return (struct Rect){left, top, right - left, bottom - top}; 
}

struct Rect rect_union(
    struct Rect r1, struct Rect r2
){
    u64 x = min(r1.x, r2.x),
        y = min(r1.y, r2.y);
    u64 w = max(r1.x, r2.x) - x,
        h = max(r1.y, r2.y) - y;

    return (struct Rect){x, y, w, h};
}

byte in_rect(
    struct Rect r1, u64 x, u64 y
){
    return x > r1.x && x < (r1.x + r1.w) && 
           y > r1.y && y < (r1.y + r1.h);
}
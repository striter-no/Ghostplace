#include <web/widgets/widget.h>


// ================================ CONTAINER ===================================

void create_cont(
    struct Container *cont,
    enum WG_CONTAINER_POS storing,
    byte scroll
){
    cont->widgets = create_table(sizeof(u64), sizeof(struct ExtCWidget));
    cont->storing_wgc = storing;
    cont->is_focused = 0;
    cont->int_xofs = 0;
    cont->int_yofs = 0;
    cont->scrollable = scroll;
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

struct WidgetRelp gmargin(enum RELP_ENUM x, enum RELP_ENUM y, f32 v1, f32 v2){
    struct WidgetRelp o = {
        .margin_right = -1,
        .margin_left = -1,
        .margin_up = -1,
        .margin_down = -1,
        .margin_hcenter = -1,
        .margin_vcenter = -1,
        .h_absolute = 0, 
        .v_absolute = 0,
        .has_habs = 0,
        .has_vabs = 0,
        .is_fixed = 0
    };

    switch (x){
        case M_RIGHT: 
            o.margin_right = v1;
            break;
        case M_LEFT: 
            o.margin_left = v1;
            break;
        case M_HCENTER: 
            o.margin_hcenter = v1;
            break;
    }

    switch (y){
        case M_UP: 
            o.margin_up = v2;
            break;
        case M_DOWN: 
            o.margin_down = v2;
            break;
        case M_VCENTER: 
            o.margin_vcenter = v2;
            break;
    }

    return o;
}

void container_cpy(
    struct Container *dst,
    struct Container *src
){
    dst->storing_wgc = src->storing_wgc;
    dst->is_focused = src->is_focused;
    dst->int_xofs = src->int_xofs;
    dst->int_yofs = src->int_yofs;
    dst->scrollable = src->scrollable;
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

void upd_container_focus(struct tgr_app *app, struct Widget *cont_wg, struct Mouse *mouse) {
    struct Container *cont = cont_wg->wgdata;
    int mouse_inside = in_rect(cont_wg->rect, mouse->x, mouse->y);
    
    struct Box __debug_box = {
        .color = (struct rgb){255, 0, 0},
        .type = BOX_ONE_LINE
    };

    struct Rect r = cont_wg->rect;
    draw_box(app, &__debug_box, r);

    // tgr_pixel(app, (struct rgb){mouse_inside ? 0: 255, 0, mouse_inside ? 255: 0}, mouse->x, mouse->y, 0);

    // Если клик вне контейнера и нажата левая кнопка - сбрасываем фокус
    if (!mouse_inside && mouse->btn != MOUSE_NO_BTN) {
        cont->is_focused = 0;
        return;
    }
    
    // Если клик внутри контейнера и нажата левая кнопка
    if (mouse_inside && mouse->btn != MOUSE_NO_BTN) {
        // cont->is_focused = true;

        // Проверяем, есть ли дочерний элемент под мышью
        int has_child_under_mouse = 0;
        struct Table *tb = &cont->widgets;
        
        i64 offset_x = 0, offset_y = 0;
        for (u64 i = 0; i < tb->size; i++) {
            struct ExtCWidget *wg;
            table_ptr(tb, (ubyte*)tb->keys + i * tb->key_size, (void**)&wg);
            
            if (wg->widget.wgtype == CONTAINER_WIDGET) {
                // Проверяем, попадает ли мышь в дочерний контейнер
                
                struct Rect cpy = wg->widget.rect;
                struct Rect *rwg = &wg->widget.rect;
                *rwg = snap_rect(cont_wg->rect, *rwg, wg->widget.orig_state, wg->positioning, wg->widget.wgtype, cont->storing_wgc, &offset_x, &offset_y);
                rwg->x += cont->int_xofs;
                rwg->y += cont->int_yofs;

                upd_container_focus(app, &wg->widget, mouse);
                has_child_under_mouse = has_child_under_mouse || ((struct Container*)wg->widget.wgdata)->is_focused;
                wg->widget.rect = cpy;
            }
        }
        
        // Устанавливаем фокус ТОЛЬКО если НЕТ дочернего элемента под мышью
        cont->is_focused = !has_child_under_mouse;
    }
}

void upd_container(
    struct tgr_app *app,
    struct Widget *cont_wg,
    struct Mouse  *mouse
){
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;
    
    if (cont->scrollable && cont->is_focused){
        if (mouse->scroll_h == 1){
            if (cont->storing_wgc == CWG_VERTICLLY)
                cont->int_yofs -= 2;
            else
                cont->int_xofs -= 2;
        } else if (mouse->scroll_h == -1) {
            if (cont->storing_wgc == CWG_VERTICLLY)
                cont->int_yofs += 2;
            else
                cont->int_xofs += 2;
        }
    }
    for (u64 i = 0; i < tb->size; i++){
        struct ExtCWidget *wg;
        table_ptr(tb, (ubyte*)tb->keys + i * tb->key_size, (void**)&wg);

        if (wg->widget.wgtype == CONTAINER_WIDGET){
            upd_container(app, &wg->widget, mouse);
        }
    }
}

void update_positions(struct Widget *widget) {
    if (widget->wgtype != CONTAINER_WIDGET) 
        return;
    
    struct Container *cont = widget->wgdata;
    struct Table *tb = &cont->widgets;
    i64 offset_x = 0, offset_y = 0;
    
    for (u64 i = 0; i < tb->size; i++) {
        struct ExtCWidget *child;
        table_ptr(tb, (ubyte*)tb->keys + i * tb->key_size, (void**)&child);
        
        struct Rect orig_rect = child->widget.rect;
        
        child->widget.rect = snap_rect(
            widget->rect,
            child->widget.rect,
            child->widget.orig_state,
            child->positioning,
            child->widget.wgtype,
            cont->storing_wgc,
            &offset_x,
            &offset_y
        );
        
        if (!child->positioning.is_fixed){
            child->widget.rect.x += cont->int_xofs;
            child->widget.rect.y += cont->int_yofs;
        }
        
        if (child->widget.wgtype == CONTAINER_WIDGET) {
            update_positions(&child->widget);
        }
        
        child->widget.rect.w = orig_rect.w;
        child->widget.rect.h = orig_rect.h;
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
    copy.uid = rand() % x_u64;

    ewg.positioning = relp;
    ewg.widget = copy;

    table_add(tb, &copy.uid, &ewg);
}

void get_widget(
    struct Widget *from,
    u64           uid,
    struct ExtCWidget **dest
){
    if (from->wgtype != CONTAINER_WIDGET) 
        return ;
    
    struct Container *cont = from->wgdata;
    struct Table *tb = &cont->widgets;
    
    table_ptr(tb, &uid, (void**)dest);
}

void set_widget(
    struct Widget *from,
    u64 uid,
    struct ExtCWidget *wg
){
    if (from->wgtype != CONTAINER_WIDGET) 
        return;
    
    struct Container *cont = from->wgdata;
    struct Table *tb = &cont->widgets;
    // struct Widget copy = {0};
    // widget_copy(&copy, &wg);
    table_add(tb, &uid, wg);
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

struct Rect snap_rect(
    struct Rect parrent, 
    struct Rect widget,
    struct Rect og_rect,
    struct WidgetRelp relp, 
    enum WIDGET_TYPE type,
    enum WG_CONTAINER_POS parrelp, 
    i64 *offset_x, i64 *offset_y
){

    i64 loffset_x = 0 , loffset_y = 0;
    if (type == CONTAINER_WIDGET){
        loffset_x = og_rect.x;
        loffset_y = og_rect.y;
    }
    
    i64 *offx = (parrelp == CWG_VERTICLLY ? offset_x : offset_y);
    i64 *offy = (parrelp == CWG_VERTICLLY ? offset_y : offset_x);
    i64 *wx = (parrelp == CWG_VERTICLLY ? &widget.x : &widget.y);
    i64 *wy = (parrelp == CWG_VERTICLLY ? &widget.y : &widget.x);
    i64 ww = (parrelp == CWG_VERTICLLY ? widget.w : widget.h);
    i64 wh = (parrelp == CWG_VERTICLLY ? widget.h : widget.w);
    i64 px = (parrelp == CWG_VERTICLLY ? parrent.x : parrent.y);
    i64 py = (parrelp == CWG_VERTICLLY ? parrent.y : parrent.x);
    i64 pw = (parrelp == CWG_VERTICLLY ? parrent.w : parrent.h);
    i64 ph = (parrelp == CWG_VERTICLLY ? parrent.h : parrent.w);

    if (relp.margin_left != -1) {
        *wx = px + (*offx) + pw * relp.margin_left;
    } else if (relp.margin_right != -1) {
        *wx = px + (*offx) + pw * (1.0f - relp.margin_right) - ww;
    } else if (relp.margin_hcenter != -1) {
        *wx = px + (*offx) + ((pw / 2) - ww / 2) * (1 + relp.margin_hcenter);
    } else {
        *wx = px + (*offx);
    }

    if (relp.margin_up != -1) {
        *wy = py + (*offy) + ph * relp.margin_up;
    } else if (relp.margin_down != -1) {
        *wy = py + (*offy) + ph * (1.0f - relp.margin_down) - wh;
    } else if (relp.margin_vcenter != -1) {
        *wy = py + (*offy) + (ph / 2 - wh / 2) * (1 + relp.margin_vcenter);
    } else {
        *wy = py + (*offy);
    }

    if ((!relp.has_vabs && parrelp == CWG_VERTICLLY) || 
        (!relp.has_habs && parrelp != CWG_VERTICLLY))
        (*offy) += wh;

    widget.x += loffset_x + (relp.has_habs ? relp.v_absolute: 0);
    widget.y += loffset_y + (relp.has_vabs ? relp.h_absolute: 0);
    return widget;
}

void draw_container(struct tgr_app *app, const struct Container *cont, struct Rect rect) {
    struct Table *tb = &cont->widgets;
    for (u64 i = 0; i < tb->size; i++) {
        struct ExtCWidget *wg;
        table_ptr(tb, (ubyte*)tb->keys + i * tb->key_size, (void**)&wg);
        
        struct Rect clipped = rect_clipping(rect, wg->widget.rect);
        if (clipped.w != 0 && clipped.h != 0) {
            if (wg->widget.wgtype == BOX_WIDGET) {
                ((struct Box*)wg->widget.wgdata)->srect = clipped;
            }
            draw_widget(app, &wg->widget);
        }
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
    (*out)->uid        = rand() % x_u64;
    (*out)->class[0] = '\0';
}

void adjust_rect(
    struct Widget *widget
){
    struct Rect rect = widget->rect;
    if (rect.w == -1 || rect.h == -1){
        switch (widget->wgtype){
            case IMAGE_WIDGET: {
                struct Image *img = widget->wgdata;
                rect.w = (rect.w == -1) ? img->img.width: rect.w;
                rect.h = (rect.h == -1) ? img->img.height / (img->is_dense ? 2: 1): rect.h;
                break;
            }
            case TEXT_WIDGET: {
                struct Text *txt = widget->wgdata;

                if (rect.w == -1){
                    i64 mx_sz = -1, crr_sz = 0;
                    for (u64 i = 0; i < utf32_strlen(txt->unicode_txt); i++){
                        if (txt->unicode_txt[i] == '\n'){
                            mx_sz = max(mx_sz, crr_sz);
                            crr_sz = 0;
                            continue;
                        }
                        crr_sz++;

                    }
                    rect.w = max(crr_sz, mx_sz);
                }

                if (rect.h == -1){
                    i64 x = 0, y = 0;
                    i64 bx = rect.w; // borders
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
                    rect.h = y + 2;
                }
                break;
            }
            case CONTAINER_WIDGET: {
                i64 mx_width = 0, mx_height = 0;
                i64 offset_x = 0, offset_y = 0;
                
                struct Container *cnt = widget->wgdata;
                struct Table     *tb  = &cnt->widgets;
                for (u64 i = 0; i < tb->size; i++){
                    struct ExtCWidget wg;
                    table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);

                    struct Rect rwg = wg.widget.rect;
                    rwg = snap_rect(rect, rwg, wg.widget.orig_state, wg.positioning, wg.widget.wgtype, cnt->storing_wgc, &offset_x, &offset_y);

                    mx_width = max(mx_width, rwg.x + rwg.w);
                    mx_height = max(mx_height, rwg.y + rwg.h);
                }

                rect.w = mx_width;
                rect.h = mx_height;
                break;
            }

            case BOX_WIDGET: {
                struct Box *bx = widget->wgdata;
                rect.w = (rect.w == -1) ? bx->srect.w: rect.w;
                rect.h = (rect.h == -1) ? bx->srect.h: rect.h;
                // printf("%i %i\n", rect.w, rect.h);
                break;
            }

            default: {
                rect.w = 1;
                rect.h = 1;
            }
        }
    }
    // widget->orig_state = rect;
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
    widget->uid = 0;
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
    dest->uid = src->uid;
    strcpy(dest->class, src->class);

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
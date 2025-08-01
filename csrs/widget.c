#include <web/widgets/widget.h>

// ================================ CONTAINER ===================================

void create_cont(
    struct Container *cont
){
    cont->widgets = create_table(sizeof(u64), sizeof(struct Widget));
}

void free_container(
    struct Container *cont
){
    struct Table *tb = &cont->widgets;

    for (u64 i = 0; i < tb->size; i++){
        struct Widget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);
        // printf("freeing %d/%d\n", i + 1, tb->size);
        if (wg.wgdata){
            free_widget(&wg);
        }
    }

    clear_table(tb);
}

void upd_contianer(
    struct Widget *cont_wg
){
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;

    for (u64 i = 0; i < tb->size; i++){
        struct Widget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);
        
        u64 h_dt = cont_wg->orig_state.x - wg.orig_state.x;
        u64 v_dt = cont_wg->orig_state.y - wg.orig_state.y;
        
        wg.rect.x = cont_wg->rect.x + h_dt;
        wg.rect.y = cont_wg->rect.y + v_dt;
    }
}

void add_widget(
    struct Widget *cont_wg,
    struct Widget to_add,
    u64           *uid
){
    if (cont_wg->wgtype != CONTAINER_WIDGET) 
        return;
    
    struct Container *cont = cont_wg->wgdata;
    struct Table *tb = &cont->widgets;

    struct Widget copy;
    widget_copy(&copy, &to_add);

    *uid = rand() % x_u64;
    table_add(tb, uid, &copy);
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
    struct BoundingRect rect
){
    struct Table *tb = &cont->widgets;

    for (u64 i = 0; i < tb->size; i++){
        struct Widget wg;
        table_at(tb, (ubyte*)tb->keys + i * tb->key_size, &wg);
        draw_widget(app, &wg);
    }
}


// ============================= WIDGET ==================================

void create_widget(
    struct Widget **out,
    enum WIDGET_TYPE type,
    struct BoundingRect rect
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

void free_widget(
    struct Widget *widget
){
    if (!widget->wgdata) return;
    free(widget->wgdata);
    widget->wgdata = NULL;
    widget->typesize = 0;
    widget->rect = (struct BoundingRect){0};
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

    memcpy(dest->wgdata, src->wgdata, src->typesize);
}
#pragma once
#include <imgm/imgs.h>
#include <console/tgr.h>
#include <utf8.h>
#include <int.h>

#ifndef min
#define min(a, b) a > b ? b: a
#endif

#ifndef max
#define max(a, b) a < b ? b: a
#endif

#define __SYMBOLS_BOX_ONE "┌┐└┘─│"
#define __SYMBOLS_BOX_DOUBLE "╔╗╚╝═║"
#define __SYMBOLS_BOX_ROUNDED "╭╮╰╯─│"

struct txt_style {
    const char *effect;
    const char *reset;
};

txt_style styles[] = {
    {"\033[1m", "\033[22m"}, // bold
    {"\033[2m", "\033[22m"}, // dim
    {"\033[3m", "\033[23m"}, // cursive
    {"\033[4m", "\033[24m"}, // underline
    {"\033[5m", "\033[25m"}, // blink
    {"\033[7m", "\033[27m"}, // inverse
    {"\033[9m", "\033[29m"}  // strike
};

enum WIDGET_TYPE {
    TEXT_WIDGET,
    IMAGE_WIDGET
};

enum BOX_TYPE {
    BOX_ONE_LINE,
    BOX_DOUBLE,
    BOX_ROUNDED
};

struct Image {
    stb_img img;
    struct rgb base_clr;
};

struct Text {
    int32_t* unicode_txt; // \0 on the end, therefore we dont need to store size
    struct rgb base_clr;
    struct txt_style style;
};

struct Box {
    enum BOX_TYPE type;
    struct rgb color;
};

struct Widget;
struct Container {
    Widget *widgets;
    u64 widgets_sz;
};

struct BoundingRect {
    u64 x, y;
    u64 w, h;
};

struct Widget {
    enum WIDGET_TYPE wgtype;
    void *wgdata;
    u64 typesize;

    struct BoundingRect rect;
};

void draw_image(
    struct tgr_app *app, 
    const struct Image *img_wdg, 
    struct BoundingRect rect
){
    const struct stb_img *img = &img_wdg->img;
    for (u64 y = 0; y < min(img->height, rect.h); y++){
        for (u64 x = 0; x < min(img->width * 2, rect.w); x+=2){
            struct rgb clr;
            int alpha;

            get_pxa(img, x / 2, y, &clr, &alpha);
            clr.r *= img_wdg->base_clr.r / 255.f;
            clr.g *= img_wdg->base_clr.g / 255.f;
            clr.b *= img_wdg->base_clr.b / 255.f;
            if (alpha != 0){
                tgr_pixel(app, clr, rect.x + x, rect.y + y, 0);
                tgr_pixel(app, clr, rect.x + x + 1, rect.y + y, 1);
            }
        }
    }
}

void draw_text(
    struct tgr_app *app, 
    const struct Text *text, 
    struct BoundingRect rect
) {
    u64 max_chars = rect.w;
    u64 text_len = utf32_strlen(text->unicode_txt);
    u64 num_chars = min(text_len, max_chars);
    
    u64 x = rect.x, y = rect.y;
    u64 bx = rect.x + rect.w, by = rect.y + rect.h; // borders
    for (u64 i = 0; i < num_chars; i++) {
        uint32_t codepoint = (uint32_t)text->unicode_txt[i];
        
        if (codepoint == '\n'){
            if (y >= by) break;
            x = rect.x;
            y++;
            continue;
        }

        if (x >= bx){
            if (y >= by) break;
            x = 0;
            y++;
        }

        struct pixel *pix = tgr_tpx_get(app, x++, y);
        
        if (!app->__frame_changed || pix->unich != codepoint) {
            app->__frame_changed = 1;
            pix->unich = codepoint;
            pix->color = text->base_clr;
            if (i == num_chars - 1)
                pix->fore_reset = 1;
        }
    }
}

void draw_box(
    struct tgr_app *app, 
    const struct Box *box, 
    struct BoundingRect rect
){
    u64 x = rect.x, y = rect.y;
    u64 bx = rect.x + rect.w, by = rect.y + rect.h; // borders

    if (rect.h < 2 || rect.w < 2) 
        return;

    const uint8_t *chars = (
        box->type == BOX_ONE_LINE ? (uint8_t*)__SYMBOLS_BOX_ONE : (
        box->type == BOX_DOUBLE   ? (uint8_t*)__SYMBOLS_BOX_DOUBLE : (
        box->type == BOX_ROUNDED  ? (uint8_t*)__SYMBOLS_BOX_ROUNDED : NULL
    )));

    if (chars == NULL) abort();

    int32_t *unichars;
    utf8_conv(chars, &unichars);

    struct pixel *pix;
    pix = tgr_tpx_get(app, x, y);
    pix->color = box->color;
    pix->unich = unichars[0];

    for (u64 i = x + 1; i < bx - 1; i++){
        struct pixel *pix = tgr_tpx_get(app, i, y);
        pix->color = box->color;
        pix->unich = unichars[4];
    } y++;

    pix = tgr_tpx_get(app, bx - 1, y);
    pix->color = box->color;
    pix->unich = unichars[1];

    for (; y < by - 1; y++){
        pix = tgr_tpx_get(app, x, y);
        pix->color = box->color;
        pix->unich = unichars[5];
        pix->fore_reset = 1;

        pix = tgr_tpx_get(app, bx - 1, y);
        pix->color = box->color;
        pix->unich = unichars[5];
        pix->fore_reset = 1;
    }

    pix = tgr_tpx_get(app, x, y);
    pix->color = box->color;
    pix->unich = unichars[2];

    for (u64 i = x + 1; i < bx - 1; i++){
        struct pixel *pix = tgr_tpx_get(app, i, y);
        pix->color = box->color;
        pix->unich = unichars[4];
    }

    pix = tgr_tpx_get(app, bx - 1, y);
    pix->color = box->color;
    pix->unich = unichars[3];

    free(unichars);
}

void draw_container(
    struct tgr_app *app, 
    const struct Container *cont, 
    struct BoundingRect rect
){
    ;
}

void upd_contianer(
    struct tgr_app *app, 
    struct Container *cont
){
    ;
}
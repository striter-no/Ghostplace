#pragma once
#include <imgm/imgs.h>
#include <console/tgr.h>

#include <utf8.h>
#include <int.h>

#define __SYMBOLS_BOX_ONE "┌┐└┘─│"
#define __SYMBOLS_BOX_DOUBLE "╔╗╚╝═║"
#define __SYMBOLS_BOX_ROUNDED "╭╮╰╯─│"
#define __SYMBOLS_PRIMITIVE "++++-|"

struct txt_style {
    const char *effect;
    const char *reset;
};

extern const struct txt_style styles[];

enum TEXT_STYLE {
    STYLE_BOLD,
    STYLE_DIM,
    STYLE_CURSIVE,
    STYLE_UNDERLINE,
    STYLE_BLINK,
    STYLE_INVERSE,
    STYLE_STRIKE,
    NO_STYLE
};

enum WIDGET_TYPE {
    TEXT_WIDGET,
    IMAGE_WIDGET,
    BOX_WIDGET,
    CONTAINER_WIDGET,
    WIDGET_UNDEFINED
};

enum BOX_TYPE {
    BOX_ONE_LINE,
    BOX_DOUBLE,
    BOX_ROUNDED,
    BOX_PRIMITIVE,
    BOX_TYPE_UNDEFINED
};

struct Rect {
    i64 x, y;
    i64 w, h;
};

struct Image {
    struct stb_img img;
    struct rgb base_clr;
    byte is_dense;
};

struct Text {
    int32_t* unicode_txt; // \0 on the end, therefore we dont need to store size
    struct rgb base_clr;
    struct txt_style style;
};

struct Box {
    enum BOX_TYPE type;
    struct rgb color;
    struct Rect srect;
};


void draw_image(
    struct tgr_app *app, 
    const struct Image *img_wdg, 
    struct Rect rect
);

void draw_image_dense(
    struct tgr_app *app, 
    const struct Image *img_wdg, 
    struct Rect rect
);

void draw_text(
    struct tgr_app *app, 
    const struct Text *text, 
    struct Rect rect
);

void draw_box(
    struct tgr_app *app, 
    const struct Box *box, 
    struct Rect rect
);

void imgwg_cpy(
    struct Image *dst,
    struct Image *src
);

void text_cpy(
    struct Text *dst,
    struct Text *src
);

void box_cpy(
    struct Box *dst,
    struct Box *src
);

void free_text_wg(
    struct Text *text
);

// ============================== RECT ==============================

struct Rect rect_intersection(
    struct Rect r1, struct Rect r2
);

struct Rect rect_union(
    struct Rect r1, struct Rect r2
);

struct Rect rect_clipping(
    struct Rect base, struct Rect origin
);

byte in_rect(
    struct Rect r1, i64 x, i64 y
);

byte in_between(
    i64 f, i64 t, i64 v
);
#pragma once
#include <table.h>
#include "base.h"

enum WG_POSITIONING {
    LEFT, MIDDLE_H, RIGHT, NORMAL_H,
    UP, MIDDLE_V, DOWN, NORMAL_V,
    ABSOLUTE
};

enum WG_CONTAINER_POS {
    CWG_HORIZONTALLY, 
    CWG_VERTICLLY
};

struct WidgetRelp {
    enum WG_POSITIONING hr, vr; // horizontally, vertically
};
// =============== WIDGET ==============
struct Widget {
    enum WIDGET_TYPE wgtype;
    void *wgdata;
    u64 typesize;

    struct Rect rect;
    struct Rect orig_state;
};

void create_widget(
    struct Widget **out,
    enum WIDGET_TYPE type,
    struct Rect rect
);

void free_widget(
    struct Widget *widget
);

void adjust_rect(
    struct Widget *widget
);

void draw_widget(
    struct tgr_app *app,
    const struct Widget *widg
);

void widget_copy(
    struct Widget *dest,
    struct Widget *src
);

// ===================== CONTAINER =======================

struct ExtCWidget {
    struct WidgetRelp positioning;
    struct Widget widget;
};

struct Container {
    enum WG_CONTAINER_POS storing_wgc;
    struct Table widgets;
};

void create_cont(
    struct Container *cont,
    enum WG_CONTAINER_POS storing
);

void free_container(
    struct Container *cont
);

void upd_contianer( 
    struct Widget *cont_wg
);

void add_widget(
    struct Widget *cont_wg,
    struct Widget to_add,
    u64           *uid,
    struct WidgetRelp relp
);

void rem_widget(
    struct Widget *cont_wg,
    u64           uid
);

void draw_container(
    struct tgr_app *app, 
    const struct Container *cont, 
    struct Rect rect
);

// ============================== RECT ==============================

struct Rect rect_intersection(
    struct Rect r1, struct Rect r2
);

struct Rect rect_union(
    struct Rect r1, struct Rect r2
);

byte in_rect(
    struct Rect r1, u64 x, u64 y
);
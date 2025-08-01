#pragma once
#include <table.h>
#include "base.h"

enum WG_POSITIONING {
    LEFT, MIDDLE_H, RIGHT,
    UP, MIDDLE_V, DOWN
};

struct WidgetRelp {
    enum WG_POSITIONING hr, vr; // horizontally, vertically
};
// =============== WIDGET ==============
struct Widget {
    enum WIDGET_TYPE wgtype;
    void *wgdata;
    u64 typesize;

    struct BoundingRect rect;
    struct BoundingRect orig_state;
};

void create_widget(
    struct Widget **out,
    enum WIDGET_TYPE type,
    struct BoundingRect rect
);

void free_widget(
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

struct Container {
    struct Table widgets;
};

void create_cont(
    struct Container *cont
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
    u64           *uid
);

void rem_widget(
    struct Widget *cont_wg,
    u64           uid
);

void draw_container(
    struct tgr_app *app, 
    const struct Container *cont, 
    struct BoundingRect rect
);


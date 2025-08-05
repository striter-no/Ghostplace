#pragma once
#include <table.h>
#include <console/mouse.h>
#include "base.h"

enum WG_CONTAINER_POS {
    CWG_HORIZONTALLY, 
    CWG_VERTICLLY
};

enum RELP_ENUM {
    M_RIGHT,
    M_LEFT,
    M_UP,
    M_DOWN,
    M_HCENTER,
    M_VCENTER,
    M_RELP_UNDEFINED
};

struct WidgetRelp {
    f32 margin_right, margin_left;
    f32 margin_up,    margin_down;
    f32 margin_hcenter, margin_vcenter;
    f32 h_absolute, v_absolute;
    
    byte has_habs, has_vabs;
    byte is_fixed;
};

struct WidgetRelp gmargin(enum RELP_ENUM x, enum RELP_ENUM y, f32 v1, f32 v2);

// =============== WIDGET ==============
struct Widget {
    enum WIDGET_TYPE wgtype;
    void *wgdata;
    u64 typesize;

    struct Rect rect;
    struct Rect orig_state;
    u64 uid;
    char class[100];
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
    byte is_focused;

    i64 int_xofs, int_yofs;
    byte scrollable;
};

void create_cont(
    struct Container *cont,
    enum WG_CONTAINER_POS storing,
    byte scroll
);

void container_cpy(
    struct Container *dst,
    struct Container *src
);

void free_container(
    struct Container *cont
);

void upd_container_focus(
    struct tgr_app *app,
    struct Widget *cont_wg,
    struct Mouse  *mouse
);

void upd_container(
    struct tgr_app *app,
    struct Widget *cont_wg,
    struct Mouse  *mouse
);

void update_positions(struct Widget *widget);

void add_widget(
    struct Widget *cont_wg,
    struct Widget to_add,
    u64           *uid,
    struct WidgetRelp relp
);

void get_widget(
    struct Widget *from,
    u64           uid,
    struct ExtCWidget **dest
);

void set_widget(
    struct Widget *from,
    u64 uid,
    struct ExtCWidget *wg
);

void rem_widget(
    struct Widget *cont_wg,
    u64           uid
);


struct Rect snap_rect(
    struct Rect parrent, 
    struct Rect widget,
    struct Rect og_rect,
    struct WidgetRelp relp, 
    enum WIDGET_TYPE type,
    enum WG_CONTAINER_POS parrelp, 
    i64 *offset_x, i64 *offset_y
);

void draw_container(
    struct tgr_app *app, 
    const struct Container *cont, 
    struct Rect rect
);
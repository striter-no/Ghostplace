#pragma once

#include "term.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <utf8.h>
#include <queue.h>

#define ALL_RST  "\033[0m"
#define FORE_RST "\033[39m"
#define BACK_RST "\033[49m"

extern int __TGR_MTB_CTRL_C_PRESSED; // ATOMIC ONLY

struct rgb {
    i16 r, g, b;
};

struct pixel {
    struct rgb color;
    struct rgb bgcolor;
    u32 unich;
    
    char prefix[15];
    char postfix[15];

    int fore_reset;
    int back_reset;
};

struct str_clr_specs {
    struct rgb *frg_specs;
    struct rgb *bg_specs;
    u64 frg_size;
    u64 bg_size;
};

struct tgr_app {
    struct terminal __raw_term;
    struct rgb background_clr;
    i32 fps;

    i32 FORCE_FPS;

    byte __frame_changed;
    i32 __frames;
    u64 __millis_passed;

    u64 TERM_WIDTH;
    u64 TERM_HEIGHT;
    u64 ticks;
    f64 deltaTime;
    
    struct queue inp_queue;
    struct pixel *pix_displ;
};

// =================== PIXEL ====================

byte pix_cmp(
    struct pixel *p1, 
    struct pixel *p2
);

byte px_in_bounds(
    struct tgr_app *app,
    i64 x, i64 y
);

struct pixel *tgr_tpx_get(
    struct tgr_app *app,
    i64 x, i64 y
);

void tgr_tpix_set(
    struct tgr_app *app,
    struct pixel pixel,
    i64 x, i64 y
);

void tgr_pixel(
    struct tgr_app *app,
    struct rgb color,

    i64 x, i64 y, byte bgrst
);

// ================== STRINGS ====================
void string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y
);

void rgb_string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y,
    struct rgb color
);

void spec_string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y,
    struct str_clr_specs specs
);

// ================== TGR SYSTEM ==================

void __tgr_ctrl_c_handler(int signum);

void tgr_fstop();

void tgr_run(
    struct tgr_app *app, 
    void (*update)(struct tgr_app *app)
);

void tgr_init(
    struct tgr_app *app
);

void tgr_end(
    struct tgr_app *app
);
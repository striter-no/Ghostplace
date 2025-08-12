#include <ghpl/console/tgr.h>

int __TGR_MTB_CTRL_C_PRESSED = 0;

void __tgr_ctrl_c_handler(){
    __sync_fetch_and_add(&__TGR_MTB_CTRL_C_PRESSED, 1);
}

long get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

long long current_time_ms() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;  // миллисекунды
}

void * __thread_tgr_inp_handler(void *args){
    struct tgr_app *app = args;
    byte *input = NULL; 
    u64 size = 0;
    
    while (!__TGR_MTB_CTRL_C_PRESSED){
        term_read(&input, &size);

        if (size > 0) {
            struct qbuffer buff;
            buff.bytes = malloc(size);
            if (buff.bytes) {
                memcpy(buff.bytes, input, size);
                buff.size = size;
                push_buffer(&app->inp_queue, &buff);
                free(buff.bytes);
            }
        }
    }
    free(input);
    pthread_exit(NULL);
}

void tgr_fstop(){
    __sync_fetch_and_add(&__TGR_MTB_CTRL_C_PRESSED, 1);
}

void tgr_run(
    struct tgr_app *app, 
    void (*update)(struct tgr_app *app)
){
    srand(time(NULL));
    signal(SIGINT, __tgr_ctrl_c_handler);
    term_write((int8_t*)"\033[?1049h", 8); // Change console screen
    term_write((int8_t*)"\033[?25l", 6); // Hide cursor

    pthread_t hid_thread;
    pthread_create(&hid_thread, NULL, __thread_tgr_inp_handler, (void *)app);

    while (!__TGR_MTB_CTRL_C_PRESSED){
        if (app->__millis_passed >= 1000){
            app->fps = app->__frames;
            app->__frames = 0;
            app->__millis_passed = 0;
        }

        u64 cbegin = current_time_ms();
        i64 nbegin = get_time_ns();

        size_t pnw, pnh;
        term_dem(&pnw, &pnh);
        pnw--;
        if (pnw != app->TERM_WIDTH || pnh != app->TERM_HEIGHT){
            app->pix_displ = (struct pixel*)realloc(app->pix_displ, sizeof(struct pixel) * pnw * pnh);
            app->last_px_displ = (struct pixel*)realloc(app->last_px_displ, sizeof(struct pixel) * pnw * pnh);

            for (u64 y = 0; y < pnh; y++){
                for (u64 x = 0; x < pnw; x++){
                    app->pix_displ[y * pnw + x] = (struct pixel){
                        .color.r = app->background_clr.r,
                        .color.g = app->background_clr.g,
                        .color.b = app->background_clr.b,
                        .bgcolor.r = app->background_clr.r,
                        .bgcolor.g = app->background_clr.g,
                        .bgcolor.b = app->background_clr.b,
                        .unich = ' ',
                        .fore_reset = 0,
                        .back_reset = 0,
                        .postfix = {0},
                        .prefix = {0}
                    };

                    app->last_px_displ[y * pnw + x] = app->pix_displ[y * pnw + x];
                }
            }

            app->TERM_WIDTH = pnw;
            app->TERM_HEIGHT = pnh;
            continue;
        }

        for (u64 y = 0; y < app->TERM_HEIGHT; y++){
            for (u64 x = 0; x < app->TERM_WIDTH; x++){
                tgr_tpix_set(app, (struct pixel){
                    .color.r = app->background_clr.r,
                    .color.g = app->background_clr.g,
                    .color.b = app->background_clr.b,
                    .bgcolor.r = app->background_clr.r,
                    .bgcolor.g = app->background_clr.g,
                    .bgcolor.b = app->background_clr.b,
                    .unich = ' ',
                    .fore_reset = 0,
                    .back_reset = 0,
                    .postfix = {0},
                    .prefix = {0}
                }, x, y);
            }
        }
        
        update(app);

        if (!app->__frame_changed)
            goto __tgr_upd_end;

        term_write((int8_t*)"\033[H", 3);
        size_t allbuff_s = 0;
        char *allbytes = NULL;
        for (u64 y = 0; y < app->TERM_HEIGHT; y++){
            u64 sx = 0, ex = app->TERM_WIDTH - 1;
            if (app->ticks == 0) goto __draw_row;

             int row_has_changes = 0;

            for (u64 x = 0; x < app->TERM_WIDTH; x++) {
                if (!pix_cmp(
                    &app->pix_displ[y * app->TERM_WIDTH + x],
                    &app->last_px_displ[y * app->TERM_WIDTH + x]
                )) {
                    sx = x;
                    row_has_changes = 1;
                    break;
                }
            }

            if (!row_has_changes) continue;

            for (u64 x = app->TERM_WIDTH - 1; x >= sx; x--) {
                if (!pix_cmp(
                    &app->pix_displ[y * app->TERM_WIDTH + x],
                    &app->last_px_displ[y * app->TERM_WIDTH + x]
                )) {
                    ex = x;
                    break;
                }
            } 

            char cursor_cmd[25] = {0};
            sprintf(cursor_cmd, "\033[%ld;%ldH", y + 1, sx + 1);
            allbytes = (char*)realloc(allbytes, allbuff_s + strlen(cursor_cmd));
            memcpy(allbytes + allbuff_s, cursor_cmd, strlen(cursor_cmd));
            allbuff_s += strlen(cursor_cmd);

            __draw_row: {;}
            for (u64 x = sx; x <= ex; x++){
                struct pixel curr = app->pix_displ[y * app->TERM_WIDTH + x];

                char buff[250] = {0};

                if (curr.prefix[0] != '\0')
                    strcat(buff, curr.prefix);

                if (curr.bgcolor.r != -1) {
                    char lbuf[90];
                    sprintf(
                        lbuf, 
                        "\033[48;2;%d;%d;%dm", 
                        curr.bgcolor.r, curr.bgcolor.g, curr.bgcolor.b
                    );
                    strcat(buff, lbuf);
                }

                if (curr.unich != ' ') {
                    uint8_t utf8_buffer[4];
                    ssize_t utf8_len = utf8proc_encode_char(curr.unich, utf8_buffer);
                    
                    if (utf8_len > 0) {
                        char lbuf[90];
                        sprintf(
                            lbuf, 
                            "\033[38;2;%d;%d;%dm", 
                            curr.color.r, curr.color.g, curr.color.b
                        );
                        strcat(buff, lbuf);
                        
                        size_t current_len = strlen(buff);
                        memcpy(buff + current_len, utf8_buffer, utf8_len);
                        buff[current_len + utf8_len] = '\0';
                    }
                } else {
                    size_t sz = strlen(buff);
                    buff[sz] = ' ';
                    buff[sz + 1] = '\0';
                }
                
                if (curr.fore_reset) strcat(buff, FORE_RST);
                if (curr.back_reset) strcat(buff, BACK_RST);

                if (curr.postfix[0] != '\0')
                    strcat(buff, curr.postfix);

                allbytes = (char*)realloc(allbytes, allbuff_s + strlen(buff));
                memcpy(allbytes + allbuff_s, buff, strlen(buff));
                allbuff_s += strlen(buff);

                int same = 1;
                for (u64 i = x; i < app->TERM_WIDTH - 1; i++){
                    if (app->pix_displ[y * app->TERM_WIDTH + i].unich != ' ' ||
                        app->pix_displ[y * app->TERM_WIDTH + i].bgcolor.r != -1
                    ){
                        same = 0;
                        break;
                    }
                }

                if (same){
                    break;
                }
            }
            if (y != app->TERM_HEIGHT - 1){
                allbytes = (char*)realloc(allbytes, allbuff_s + 1);
                allbytes[allbuff_s] = '\n';
                allbuff_s++;
            }
        }

        
        term_write((int8_t*)allbytes, allbuff_s);
        free(allbytes);
        app->__frame_changed = 0;

        __tgr_upd_end: {
           
            if (app->FORCE_FPS != -1){
                i64 ndt = get_time_ns() - nbegin;
                i64 sleep_time_ns = (1000000000L / app->FORCE_FPS) - ndt;
                
                struct timespec req={0};
            
                if (sleep_time_ns > 0) {
                    req.tv_sec = sleep_time_ns / 1000000000L;
                    req.tv_nsec = sleep_time_ns % 1000000000L;
                    nanosleep(&req, NULL);
                }
            }
            u64 dt = current_time_ms() - cbegin;
            
            app->deltaTime = dt / 1000.f;
            app->__millis_passed += dt;
            app->__frames++;
            app->ticks++;

            memcpy(app->last_px_displ, app->pix_displ, sizeof(struct pixel) * app->TERM_HEIGHT * app->TERM_WIDTH);
        }
    }
    pthread_join(hid_thread, NULL);

    term_write((int8_t*)"\033[?25h", 6);
    term_write((int8_t*)"\033[?1049l", 8); // Restore console screen
}

void tgr_init(
    struct tgr_app *app
){
    term_new(&app->__raw_term);
    app->inp_queue = create_queue();
    app->fps = 0;
    app->background_clr = (struct rgb){50, 50, 50};
    app->FORCE_FPS = -1;

    app->ticks = 0;
    app->__frames = 0;
    app->__millis_passed = 0;

    app->pix_displ = NULL;

    term_dem(
        &app->TERM_WIDTH, 
        &app->TERM_HEIGHT
    );
    app->TERM_WIDTH--;

    u64 size = sizeof(struct pixel) * app->TERM_WIDTH * app->TERM_HEIGHT;
    app->pix_displ = (struct pixel*)malloc(size);

    if (app->pix_displ == NULL){
        fprintf(stderr, "tgr_init:malloc(1):failed for %ld bytes", size);
        abort();
    }

    app->last_px_displ = (struct pixel*)malloc(size);

    if (app->last_px_displ == NULL){
        fprintf(stderr, "tgr_init:malloc(1):failed for %ld bytes", size);
        abort();
    }

    for (u64 y = 0; y < app->TERM_HEIGHT; y++){
        for (u64 x = 0; x < app->TERM_WIDTH; x++){
            app->pix_displ[y * app->TERM_WIDTH + x] = (struct pixel){
                .color.r = app->background_clr.r,
                .color.g = app->background_clr.g,
                .color.b = app->background_clr.b,
                .bgcolor.r = app->background_clr.r,
                .bgcolor.g = app->background_clr.g,
                .bgcolor.b = app->background_clr.b,
                .unich = ' ',
                .fore_reset = 0,
                .back_reset = 0,
                .postfix = {0},
                .prefix = {0}
            };
        }
    }

    app->__frame_changed = 0;
}

void tgr_end(
    struct tgr_app *app
){
    __TGR_MTB_CTRL_C_PRESSED = 0;
    clear_queue(&app->inp_queue);
    free(app->pix_displ);
    free(app->last_px_displ);
    term_reset(&app->__raw_term);
}

// STRINGS

void string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y
){
    u64 sz = utf32_strlen(string);
    for (u64 i = 0; i < sz; i++){
        struct pixel *pix = tgr_tpx_get(app, x + i, y);
        if (!pix) continue;
        if (!app->__frame_changed || pix->unich != string[i])
            app->__frame_changed = 1;
        else
            continue;

        pix->unich = string[i];

    }
}

void rgb_string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y,
    struct rgb color
){
    i64 sx = x;
    u64 sz = utf32_strlen(string);
    for (u64 i = 0; i < sz; i++){
        if (string[i] == '\n') {
            y++;
            x = sx;
            continue;
        }

        struct pixel *pix = tgr_tpx_get(app, x++, y);
        if (!pix) continue;

        if (!app->__frame_changed || pix->unich != string[i])
            app->__frame_changed = 1;
        else
            continue;

        pix->unich = string[i];
        pix->color = color;
        // if (sz == 0) pix->color = color;
        // if (sz - 1 == i) pix->fore_reset = 1;
    }
}

void spec_string_insert(
    struct tgr_app *app,
    const int32_t *string,

    i64 x, i64 y,
    struct str_clr_specs specs
){
    byte has_frg = specs.frg_size > 0;
    byte has_bg = specs.bg_size > 0;

    u64 sz = utf32_strlen(string);
    for (u64 i = 0; i < sz; i++){
        struct pixel *pix = tgr_tpx_get(app, x + i, y);
        if (!pix) continue;
        
        if (!app->__frame_changed || pix->unich != string[i])
            app->__frame_changed = 1;
        else
            continue;

        pix->unich = string[i];
        if (has_frg)
            pix->color = specs.frg_specs[i];
        if (has_bg)
            pix->bgcolor = specs.bg_specs[i];

        if (has_frg && (sz - 1 == i)) pix->fore_reset = 1; 
        if (has_bg && (sz - 1 == i)) pix->back_reset = 1;
    }
}

// PIXEL

byte pix_cmp(struct pixel *p1, struct pixel *p2){
    return p1->color.r == p2->color.r && 
           p1->color.g == p2->color.g && 
           p1->color.b == p2->color.b && 
           p1->bgcolor.r == p2->bgcolor.r && 
           p1->bgcolor.g == p2->bgcolor.g && 
           p1->bgcolor.b == p2->bgcolor.b && 
           p1->unich == p2->unich;
}

byte px_in_bounds(
    struct tgr_app *app,
    i64 x, i64 y
){
    return x >= 0 && (size_t)x < app->TERM_WIDTH && y >= 0 && (size_t)y < app->TERM_HEIGHT; 
}

void tgr_tpix_set(
    struct tgr_app *app,
    struct pixel pixel,
    i64 x, i64 y
){
    if (!px_in_bounds(app, x, y)) return;

    if (!app->__frame_changed || !pix_cmp(&app->pix_displ[
        y * app->TERM_WIDTH + x
    ], &pixel)){
        app->__frame_changed = 1;
    } else {
        return;
    }

    app->pix_displ[
        y * app->TERM_WIDTH + x
    ] = pixel;
}

void tgr_pixel(
    struct tgr_app *app,
    struct rgb color,

    i64 x, i64 y, byte bgrst
){
    if (!px_in_bounds(app, x, y)) return;

    struct rgb bgc = app->pix_displ[
        y * app->TERM_WIDTH + x
    ].bgcolor;
    if (!app->__frame_changed || !(bgc.r == color.r && bgc.g == color.g && bgc.b == color.b)){
        app->__frame_changed = 1;
    } else {
        return;
    }

    app->pix_displ[
        y * app->TERM_WIDTH + x
    ].bgcolor = color;
    app->pix_displ[
        y * app->TERM_WIDTH + x
    ].back_reset = bgrst;
}

struct pixel *tgr_tpx_get(
    struct tgr_app *app,
    i64 x, i64 y
){
    if (!px_in_bounds(app, x, y)) return NULL;
    return &app->pix_displ[app->TERM_WIDTH * y + x];
}
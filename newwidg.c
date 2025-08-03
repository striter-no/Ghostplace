#include <console/tgr.h>
#include <console/keyboard.h>
#include <console/mouse.h>

#include <web/widgets/widget.h>

struct Widget *main_cnt = NULL;
static struct Keyboard kb;
static struct Mouse mouse;

u64 imp_uid;

void update(struct tgr_app *app);

int main(){
    struct tgr_app app;
    tgr_init(&app);
    app.background_clr = (struct rgb){28, 28, 28};
    app.FORCE_FPS = 60;

    mouse = (struct Mouse){0};
    create_kboard(&kb);
    create_widget(&main_cnt, CONTAINER_WIDGET, (struct Rect){0, 0, app.TERM_WIDTH - 1, app.TERM_HEIGHT});
    create_cont(main_cnt->wgdata, CWG_VERTICLLY);

    u64 uid;
    // ===================

    for (int i = 0; i < 2; i++){
        struct Widget *subcont = NULL; create_widget(&subcont, CONTAINER_WIDGET, (struct Rect){0, 0, -1, -1});
        create_cont(subcont->wgdata, CWG_HORIZONTALLY);
        for (int k = 0; k < 3; k++){
            struct Widget *box = NULL; create_widget(&box, BOX_WIDGET, (struct Rect){0, 0, 10, 5} );
            *(struct Box *)(box->wgdata) = (struct Box){
                .color = (struct rgb){255, 0, 255},
                .type = BOX_DOUBLE
            };

            adjust_rect(box);
            add_widget(subcont, *box, &uid, (struct WidgetRelp){NORMAL_H, NORMAL_V});
            free_widget(box);
            free(box);
        }
        adjust_rect(subcont);
        
        add_widget(main_cnt, *subcont, &uid, (struct WidgetRelp){MIDDLE_H, NORMAL_V});
        free_widget(subcont);
        free(subcont);
    }

    struct Widget *text = NULL; create_widget(&text, TEXT_WIDGET, (struct Rect){0, 0, 10, -1} );
    *(struct Text *)(text->wgdata) = (struct Text){
        .base_clr = (struct rgb){0, 255, 0},
        .style = styles[NO_STYLE],
        .unicode_txt = NULL
    };
    utf8_conv("Test test test\ntest2\ntest test абвгд", &(((struct Text *)(text->wgdata))->unicode_txt));

    adjust_rect(text);
    add_widget(main_cnt, *text, &imp_uid, (struct WidgetRelp){MIDDLE_H, NORMAL_V});
    free_widget(text);
    free(text);

    struct Widget *img = NULL; create_widget(&img, IMAGE_WIDGET, (struct Rect){0, 0, -1, -1});
    *(struct Image *)(img->wgdata) = (struct Image){
        .base_clr = (struct rgb){255, 255, 255},
        .is_dense = 1,
        .img = {0}
    };
    img_load("./assets/cat.png", &(((struct Image*)(img->wgdata))->img), 0);

    adjust_rect(img);
    add_widget(main_cnt, *img, &uid, (struct WidgetRelp){RIGHT, NORMAL_V});
    free_widget(img);
    free(img);

    // RUN ===================
    enable_mouse(MOUSE_NORMAL_TRACKING);
    tgr_run(&app, update);
    tgr_end(&app);
    disable_mouse(MOUSE_NORMAL_TRACKING);

    // END ===================
    if (main_cnt){
        free_widget(main_cnt);
        free(main_cnt);
    }

    free_keyboard(&kb);
}


void update(struct tgr_app *app){
    struct qbuffer buffer;
    if (0 == pop_buffer(&app->inp_queue, &buffer)){
        clear_qbuffer(&buffer);
    }

    // WIDGETS =======================
    
    struct ExtCWidget *wg;
    get_widget(main_cnt, imp_uid, &wg);
    struct Text *txt = wg->widget.wgdata;
    txt->base_clr.r = 255;
    
    upd_contianer(main_cnt);
    draw_widget(app, main_cnt);

    // FPS =======================
    char fpsbuff[20];
    sprintf(fpsbuff, "%d", app->fps);

    int32_t *unistr;
    utf8_conv(fpsbuff, &unistr);
    rgb_string_insert(app, unistr, 0, 0, (struct rgb){150, 200, 150});
    free(unistr);
}
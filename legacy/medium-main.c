#include <console/tgr.h>
#include <console/keyboard.h>
#include <console/mouse.h>

#include <imgm/imgs.h>
#include <web/widgets/widget.h>

struct Widget *main_cnt = NULL;
static struct Keyboard kb;
static struct Mouse mouse;

void update(struct tgr_app *app){
    struct qbuffer buff = {0};
    byte has_bytes = 0 == pop_buffer(&app->inp_queue, &buff);
    if (!has_bytes) goto after_hid;

    byte has_input = kb_process_input(
        &kb, buff.bytes, buff.size
    );

    if (has_input){
        struct Key tmp;
        get_pressed_key(&kb, &tmp);

        if (key_cmp(tmp, keye("esc"))){
            tgr_fstop();
        }
    }

    byte mhas_input = process_mouse(
        &mouse, buff.bytes, buff.size
    );

    clear_qbuffer(&buff);

    // ================================ AFTER HID =================================
    after_hid:

    draw_widget(app, main_cnt);
    tgr_pixel(app, (struct rgb){
        255, 0, 255
    }, mouse.x, mouse.y, 0);

    struct Image img;
    img.base_clr = (struct rgb){255, 255, 255};
    img.is_dense = 0;

    struct stb_img *simg = &img.img;
    img_load("./assets/ghostplace.png", simg, 4);
    
    draw_image(app, &img, (struct Rect){app->TERM_WIDTH / 2 - simg->width, app->TERM_HEIGHT * 0.4, 100, 20});
    img_free(simg);

    struct Image g_img;
    g_img.base_clr = (struct rgb){200, 255, 220};
    g_img.is_dense = 0;

    struct stb_img *g_simg = &g_img.img;
    img_load("./assets/ghost.png", g_simg, 4);
    
    draw_image(app, &g_img, (struct Rect){app->TERM_WIDTH * 0.7, app->TERM_HEIGHT * 0.55, 100, 20});
    img_free(g_simg);

    // =================== FPS =======================
    char fpsbuff[20];
    sprintf(fpsbuff, "%d", app->fps);

    int32_t *unistr;
    utf8_conv(fpsbuff, &unistr);
    rgb_string_insert(app, unistr, 0, 0, (struct rgb){100, 50, 150});
    free(unistr);
}

int main(){
    struct tgr_app app;
    tgr_init(&app);
    app.background_clr = (struct rgb){28, 28, 28};
    app.FORCE_FPS = 60;

    mouse = (struct Mouse){0};
    create_kboard(&kb);
    
    u64 uid;
    struct Widget *box = NULL; create_widget(&box, 
        BOX_WIDGET, 
        (struct Rect){0, 0, app.TERM_WIDTH - 1, app.TERM_HEIGHT}
    );
    
    struct Box *box_ptr = box->wgdata;
    box_ptr->color = (struct rgb){70, 130, 200};
    box_ptr->type = BOX_ROUNDED;


    struct Widget *img = NULL; create_widget(&img, 
        IMAGE_WIDGET, 
        (struct Rect){0, 0, app.TERM_WIDTH - 1, app.TERM_HEIGHT}
    );

    struct Image *img_ptr = img->wgdata;
    struct stb_img *simg = &img_ptr->img;

    img_load("./assets/earth.png", simg, 0);
    img_crop(simg, simg->width * 0.2, 0, simg->width * 0.8, simg->height);
    img_ptr->base_clr = (struct rgb){100, 100, 100};
    img_ptr->is_dense = 0;
    ubyte *data = simg->data;


    struct Widget *text = NULL; create_widget(&text, 
        TEXT_WIDGET, 
        (struct Rect){app.TERM_WIDTH * 0.1, app.TERM_HEIGHT * 0.6, app.TERM_WIDTH / 2 - 5, 20}
    );
    struct Text *text_ptr = text->wgdata;
    text_ptr->base_clr = (struct rgb){255, 255, 255};
    text_ptr->style = styles[STYLE_CURSIVE];
    utf8_conv("Привет всем это демонстрация!\nAnd here goes ASCII\nIn software, a stack buffer overflow or stack buffer overrun occurs when a program writes to a memory address on the program's call stack outside of the intended data structure, which is usually a fixed-length buffer.[1][2] Stack buffer overflow bugs are caused when a program writes more data to a buffer located on the stack than what is actually allocated for that buffer. This almost always results in corruption of adjacent data on the stack, and in cases where the overflow was triggered by mistake, will often cause the program to crash or operate incorrectly. Stack buffer overflow is a type of the more general programming malfunction known as buffer overflow (or buffer overrun).[1] Overfilling a buffer on the stack is more likely to derail program execution than overfilling a buffer on the heap because the stack contains the return addresses for all active function calls.", &text_ptr->unicode_txt);
    int32_t *txt_data = text_ptr->unicode_txt;

    create_widget(&main_cnt, CONTAINER_WIDGET, (struct Rect){0, 0, app.TERM_WIDTH - 1, app.TERM_HEIGHT});
    create_cont(main_cnt->wgdata);

    add_widget(main_cnt, *box, &uid);
    add_widget(main_cnt, *img, &uid);
    add_widget(main_cnt, *text, &uid);

    free_widget(text);
    free(text);

    free_widget(img); // Already copied
    free(img);
    
    free_widget(box); // Already copied
    free(box);
    
    enable_mouse(MOUSE_NORMAL_TRACKING);
    tgr_run(&app, update);
    tgr_end(&app);
    disable_mouse(MOUSE_NORMAL_TRACKING);

    free_container(main_cnt->wgdata);
    free_widget(main_cnt);
    free(main_cnt);

    free_keyboard(&kb);
    stbi_image_free(data);
    free(txt_data);
}
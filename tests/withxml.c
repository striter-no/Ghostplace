#include <console/tgr.h>
#include <console/keyboard.h>
#include <console/mouse.h>

#include <web/parser.h>
#include <web/cssparser.h>

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

    u64 uid;
    // Languages ===================
    char *css;
    readfile("./assets/pages/test.gss", &css);
    
    struct css_block *css_blocks = parse_css(css);
    free(css);
    
    if (!css_blocks) {
        printf("Failed to parse CSS\n");
        return 1;
    }
    
    char *xml;
    readfile("./assets/pages/test.ghml", &xml);
    
    struct tag *xml_root = parse_xml(xml);
    free(xml);
    if (!xml_root) {
        printf("Failed to parse XML\n");
        return 1;
    }

    // ===================
    xml_addwidgets(&app, &main_cnt, xml_root, 0);
    css_parsewidgets(&main_cnt, css_blocks);
    free_tag(xml_root);
    free_css(css_blocks);
    
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
    mouse.scroll_h = 0;

    struct qbuffer buffer;
    if (0 == pop_buffer(&app->inp_queue, &buffer)){
        process_mouse(&mouse, buffer.bytes, buffer.size);
        clear_qbuffer(&buffer);
    }

    // WIDGETS =======================

    upd_container_focus(app, main_cnt, &mouse);
    upd_container(app, main_cnt, &mouse);
    update_positions(main_cnt);
    // main_cnt->rect = main_cnt->orig_state;
    draw_widget(app, main_cnt);

    // FPS =======================
    char fpsbuff[20];
    sprintf(fpsbuff, "%d", app->fps);

    int32_t *unistr;
    utf8_conv(fpsbuff, &unistr);
    rgb_string_insert(app, unistr, 0, 0, (struct rgb){150, 200, 150});
    free(unistr);
}
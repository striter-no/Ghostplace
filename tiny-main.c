#include <console/tgr.h>
#include <console/keyboard.h>
#include <imgm/imgs.h>

#include <web/widgets/widget.h>

struct Widget *main_cnt = NULL;
static struct Keyboard kb;

void update(struct tgr_app *app){

    byte has_input = kb_process_input(
        &kb, app->input, app->input_len
    );

    if (has_input){
        struct Key tmp;
        get_pressed_key(&kb, &tmp);

        if (key_cmp(tmp, keyc("в"))){
            app->FORCE_STOP = 1;
        }
    }

    draw_widget(app, main_cnt);

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

    create_kboard(&kb);

    u64 uid;
    struct Widget *box = NULL; create_widget(&box, 
        BOX_WIDGET, 
        (struct BoundingRect){0, 0, app.TERM_WIDTH, app.TERM_HEIGHT}
    );
    
    struct Box *box_ptr = box->wgdata;
    box_ptr->color = (struct rgb){70, 130, 200};
    box_ptr->type = BOX_ROUNDED;


    struct Widget *img = NULL; create_widget(&img, 
        IMAGE_WIDGET, 
        (struct BoundingRect){5, 5, 20, 20}
    );

    struct Image *img_ptr = img->wgdata;
    img_load("./assets/ghost.png", &img_ptr->img, 4);
    img_ptr->base_clr = (struct rgb){200, 200, 200};
    img_ptr->is_dense = 1;
    ubyte *data = img_ptr->img.data;   


    create_widget(&main_cnt, CONTAINER_WIDGET, (struct BoundingRect){0, 0, app.TERM_WIDTH - 1, app.TERM_HEIGHT});
    create_cont(main_cnt->wgdata);

    add_widget(main_cnt, *box, &uid);
    add_widget(main_cnt, *img, &uid);
    
    free_widget(img); // Already copied
    free(img);
    
    free_widget(box); // Already copied
    free(box);

    tgr_run(&app, update);
    tgr_end(&app);

    free_container(main_cnt->wgdata);
    free_widget(main_cnt);
    free(main_cnt);

    free_keyboard(&kb);
    stbi_image_free(data);
}
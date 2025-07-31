#include <console/tgr.h>
#include <imgm/imgs.h>
struct stb_img ghost, ghostlabel;

void update(struct tgr_app *app){
    img_insert(
        app, 
        &ghost, 
        floor(app->TERM_WIDTH / 2.f - ghost.width), 
        app->TERM_HEIGHT * 0.1f,
        (struct rgb){170, 170, 170}
    );

    img_insert(
        app, 
        &ghostlabel, 
        floor(app->TERM_WIDTH / 2.f - ghostlabel.width), 
        app->TERM_HEIGHT * 0.1f + ghost.height + 1,
        (struct rgb){170, 170, 170}
    );

    char fpsbuff[20];
    sprintf(fpsbuff, "%d", app->fps);

    int32_t *unistr;
    utf8_conv(fpsbuff, &unistr);
    rgb_string_insert(app, unistr, 0, 0, (struct rgb){100, 50, 150});
    free(unistr);
}

int main(){
    
    img_load("./assets/ghost.png", &ghost, 4);
    img_load("./assets/ghostplace.png", &ghostlabel, 4);

    struct tgr_app app;
    tgr_init(&app);
    app.FORCE_FPS = 60;

    tgr_run(&app, update);
    tgr_end(&app);

    img_free(&ghost);
    img_free(&ghostlabel);
}
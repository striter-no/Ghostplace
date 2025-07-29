#include <math.h>
#include "src/tgr.h"
#include "src/imgs.h"
struct stb_img ghost, ghostlabel;

void update(struct tgr_app *app){
    img_insert(
        app, 
        &ghost, 
        floor(app->TERM_WIDTH / 2.f - ghost.width), 
        app->TERM_HEIGHT * 0.1f
    );

    img_insert(
        app, 
        &ghostlabel, 
        floor(app->TERM_WIDTH / 2.f - ghostlabel.width), 
        app->TERM_HEIGHT * 0.1f + ghost.height + 1
    );

    char fpsbuff[20];
    sprintf(fpsbuff, "%d", app->fps);

    rgb_string_insert(app, fpsbuff, 0, 0, (struct rgb){.r = 100, .g = 50, .b = 150});
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
}
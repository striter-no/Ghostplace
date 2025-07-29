#include <math.h>
#include "src/tgr.h"

void update(struct tgr_app *app){
    for (u64 y = 0; y < app->TERM_HEIGHT; y++){
        for (u64 x = 0; x < app->TERM_WIDTH; x++){
            tgr_pixel(
                app, (struct rgb){
                    .r = (int)floor((float)y/app->TERM_HEIGHT * 255), 
                    .g = (int)floor((float)x/app->TERM_WIDTH * 255), 
                    .b = abs((int)floor(255 * sin(app->ticks * 0.005f )))
                }, 
                x, y
            );
        }
    }

    char fpsbuff[20];
    sprintf(fpsbuff, "fps: %d", app->FPS);

    rgb_string_insert(app, fpsbuff, 0, 0, (struct rgb){.r = 255, .g = 0, .b = 0});
}

int main(){
    struct tgr_app app;
    tgr_init(&app);
    app.FORCE_FPS = 60;

    tgr_run(&app, update);
    tgr_end(&app);
}
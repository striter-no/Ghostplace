#define STB_IMAGE_IMPLEMENTATION
#include "../src/imgs.h"

void img_load(const char *path, struct stb_img *img, int preffred_channels){
    img->data = stbi_load(
        path, 
        &img->width, 
        &img->height, 
        &img->channels, 
        preffred_channels
    );
}

void img_free(struct stb_img *img){
    stbi_image_free(img->data);
}

void get_px(struct stb_img *img, u64 x, u64 y, struct rgb *out){
    int index = (y * img->width + x) * 3;
    out->r = img->data[index + 0];
    out->g = img->data[index + 1];
    out->b = img->data[index + 2];
}

void get_pxa(struct stb_img *img, u64 x, u64 y, struct rgb *out, int *alpha){
    int index = (y * img->width + x) * 4;
    out->r = img->data[index + 0];
    out->g = img->data[index + 1];
    out->b = img->data[index + 2];
    *alpha = img->data[index + 3];
}

void img_insert(struct tgr_app *app, struct stb_img *img, u64 sx, u64 sy){
    for (u64 y = 0; y < img->height; y++){
        for (u64 x = 0; x < img->width * 2; x+=2){
            struct rgb clr;
            int alpha;

            get_pxa(img, x / 2, y, &clr, &alpha);
            if (alpha != 0){
                tgr_pixel(app, clr, sx + x, sy + y, 0);
                tgr_pixel(app, clr, sx + x + 1, sy + y, 1);
            }
        }
    }
}
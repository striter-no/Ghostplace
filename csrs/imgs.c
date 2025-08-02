#define STB_IMAGE_IMPLEMENTATION
#include <imgm/imgs.h>

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
    if (!in_img(img, x, y)) {
        out->r = -1;
        return;
    }

    int index = (y * img->width + x) * 3;
    out->r = img->data[index + 0];
    out->g = img->data[index + 1];
    out->b = img->data[index + 2];
}

void get_pxa(const struct stb_img *img, u64 x, u64 y, struct rgb *out, int *alpha){
    // printf("getpxa: %i %i/%i %i\n", x, y, img->width, img->height);
    
    if (!in_img(img, x, y)) {
        out->r = -1;
        return;
    }

    int index = (y * img->width + x) * 4;
    out->r = img->data[index + 0];
    out->g = img->data[index + 1];
    out->b = img->data[index + 2];
    *alpha = img->data[index + 3];
}

void set_px(struct stb_img *img, u64 x, u64 y, struct rgb clr){
    if (!in_img(img, x, y)) {
        return;
    }

    int index = (y * img->width + x) * 3;
    img->data[index + 0] = clr.r;
    img->data[index + 1] = clr.g;
    img->data[index + 2] = clr.b;
}

void set_pxa(struct stb_img *img, u64 x, u64 y, struct rgb clr, int alpha){
    if (!in_img(img, x, y)) {
        return;
    }

    int index = (y * img->width + x) * 4;
    img->data[index + 0] = clr.r;
    img->data[index + 1] = clr.g;
    img->data[index + 2] = clr.b;
    img->data[index + 3] = alpha;
}

void img_insert(
    struct tgr_app *app, 
    struct stb_img *img, 
    u64 sx, u64 sy,
    struct rgb baseclr
){
    for (u64 y = 0; y < img->height; y++){
        for (u64 x = 0; x < img->width * 2; x+=2){
            struct rgb clr;
            int alpha;

            get_pxa(img, x / 2, y, &clr, &alpha);
            clr.r *= baseclr.r / 255.f;
            clr.g *= baseclr.g / 255.f;
            clr.b *= baseclr.b / 255.f;
            if (alpha != 0){
                tgr_pixel(app, clr, sx + x, sy + y, 0);
                tgr_pixel(app, clr, sx + x + 1, sy + y, 1);
            }
        }
    }
}

void img_crop(struct stb_img *img, u64 fx, u64 fy, u64 tx, u64 ty){
    struct stb_img out;
    img_blank(&out, tx - fx, ty - fy, img->channels);

    byte isa = img->channels == 4;
    for (u64 y = fy; y < ty; y++){
        for (u64 x = fx; x < tx; x++){
            struct rgb orig; int alpha;
            if (isa) {
                get_pxa(img, x, y, &orig, &alpha);
                set_pxa(&out, x - fx, y - fy, orig, alpha);
            } else {
                get_px(img, x, y, &orig);
                set_px(&out, x - fx, y - fy, orig);
            }
        }
    }

    img_rmove(img, &out);
    img_free(&out);
}

void img_cpy(struct stb_img *to, struct stb_img *src){
    to->channels = src->channels;
    to->width = src->width;
    to->height = src->height;
    // if (to->data == NULL)
    to->data = malloc(src->width * src->height * src->channels);
    memcpy(to->data, src->data, src->width * src->height * src->channels);
}

void img_rmove(struct stb_img *to, struct stb_img *src){
    img_free(to);
    img_cpy(to, src);
}

void img_blank(struct stb_img *img, u64 w, u64 h, int channels){
    img->channels = channels;
    img->width = w;
    img->height = h;
    img->data = malloc(w * h * channels);
}

byte in_img(struct stb_img *img, u64 x, u64 y){
    return x >= 0 && x < img->width && y >= 0 && y < img->height;
}
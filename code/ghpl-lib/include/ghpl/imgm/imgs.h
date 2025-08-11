#pragma once

#include <ghpl/imgm/stb_image.h>
#include <ghpl/console/tgr.h>

struct stb_img {
    int width;
    int height;
    int channels;
    ubyte *data;
};

void img_crop(struct stb_img *img, u64 fx, u64 fy, u64 tx, u64 ty);

void img_load(const char *path, struct stb_img *img, int preffred_channels);
void img_free(struct stb_img *img);
void img_cpy(struct stb_img *to, struct stb_img *src);
void img_rmove(struct stb_img *to, struct stb_img *src);
void img_blank(struct stb_img *img, u64 w, u64 h, int channels);

void set_px(struct stb_img *img, u64 x, u64 y, struct rgb clr);
void set_pxa(struct stb_img *img, u64 x, u64 y, struct rgb clr, int alpha);
void get_px(struct stb_img *img, u64 x, u64 y, struct rgb *out);
void get_pxa(const struct stb_img *img, u64 x, u64 y, struct rgb *out, int *alpha);

byte in_img(struct stb_img *img, u64 x, u64 y);

void img_insert(
    struct tgr_app *app, 
    struct stb_img *img, 
    u64 sx, u64 sy,
    struct rgb baseclr
);
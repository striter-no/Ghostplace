#pragma once

#include <ghpl/imgm/stb_image.h>
#include <ghpl/console/tgr.h>

struct stb_img {
    int width;
    int height;
    int channels;
    ubyte *data;
};

void img_crop(struct stb_img *img, int fx, int fy, int tx, int ty);

void img_load(const char *path, struct stb_img *img, int preffred_channels);
void img_free(struct stb_img *img);
void img_cpy(struct stb_img *to, struct stb_img *src);
void img_rmove(struct stb_img *to, struct stb_img *src);
void img_blank(struct stb_img *img, int w, int h, int channels);

void set_px(struct stb_img *img, int x, int y, struct rgb clr);
void set_pxa(struct stb_img *img, int x, int y, struct rgb clr, int alpha);
void get_px(struct stb_img *img, int x, int y, struct rgb *out);
void get_pxa(const struct stb_img *img, int x, int y, struct rgb *out, int *alpha);

byte in_img(const struct stb_img *img, int x, int y);

void img_insert(
    struct tgr_app *app, 
    struct stb_img *img, 
    int sx, int sy,
    struct rgb baseclr
);
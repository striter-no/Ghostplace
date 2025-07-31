#pragma once

#include "stb_image.h"
#include "../console/tgr.h"

struct stb_img {
    int width;
    int height;
    int channels;
    ubyte *data;
};

void img_load(const char *path, struct stb_img *img, int preffred_channels);
void get_px(struct stb_img *img, u64 x, u64 y, struct rgb *out);
void get_pxa(const struct stb_img *img, u64 x, u64 y, struct rgb *out, int *alpha);
void img_free(struct stb_img *img);
void img_insert(
    struct tgr_app *app, 
    struct stb_img *img, 
    u64 sx, u64 sy,
    struct rgb baseclr
);
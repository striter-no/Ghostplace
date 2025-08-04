#include <web/widgets/base.h>

const struct txt_style styles[] = {
    {"\033[1m", "\033[22m"}, // bold
    {"\033[2m", "\033[22m"}, // dim
    {"\033[3m", "\033[23m"}, // cursive
    {"\033[4m", "\033[24m"}, // underline
    {"\033[5m", "\033[25m"}, // blink
    {"\033[7m", "\033[27m"}, // inverse
    {"\033[9m", "\033[29m"}, // strike
    {"", ""}                 // empty
};

void draw_image(
    struct tgr_app *app, 
    const struct Image *img_wdg, 
    struct Rect rect
){
    if (img_wdg->is_dense){
        draw_image_dense(app, img_wdg, rect);
        return;
    }

    const struct stb_img *img = &img_wdg->img;
    
    byte isa = img->channels == 4;
    u64 hy = min(img->height, rect.h);
    for (u64 y = 0; y < hy; y++){
        for (i64 x = 0; x < min(rect.w, img->width * 2); x+=2){
            struct rgb clr;
            int alpha = -1;

            if (isa) get_pxa(img, x / 2, y, &clr, &alpha);
            else get_px(img, x / 2, y, &clr);
            clr.r *= img_wdg->base_clr.r / 255.f;
            clr.g *= img_wdg->base_clr.g / 255.f;
            clr.b *= img_wdg->base_clr.b / 255.f;
            if (alpha != 0){
                tgr_pixel(app, clr, rect.x + x, rect.y + y, 0);
                if (rect.x + x + 1 > rect.x + rect.w) continue;
                tgr_pixel(app, clr, rect.x + x + 1, rect.y + y, 1);
            }
        }
    }
}

void draw_image_dense(
    struct tgr_app *app, 
    const struct Image *img_wdg, 
    struct Rect rect
){
    const struct stb_img *img = &img_wdg->img;
    
    u64 rheight = img->height;
    int is_ok = rheight % 2 == 0;

    rheight += !is_ok;
    u64 hy = min(rheight / 2, rect.h);
    
    i32 *i;
    utf8_conv((ubyte*)"▄", &i);
    byte isa = img->channels == 4;
    for (u64 y = 0; y < hy * 2; y += 2){
        for (i64 x = 0; x < min(img->width, rect.w); x++){
            struct pixel *px = tgr_tpx_get(app, rect.x + x, rect.y + y / 2);
            if (!px) continue;

            struct rgb clr_bg, clr_fr;
            int alpha_up = -1, alpha_down = -1;

            if (isa) get_pxa(img, x, y, &clr_bg, &alpha_up);
            else get_px(img, x, y, &clr_bg);

            if (alpha_up == 0) clr_bg.r = -1;
            else {
                clr_bg.r *= img_wdg->base_clr.r / 255.f;
                clr_bg.g *= img_wdg->base_clr.g / 255.f;
                clr_bg.b *= img_wdg->base_clr.b / 255.f;    
            }

            if (isa) get_pxa(img, x, y + 1, &clr_fr, &alpha_down);
            else get_px(img, x, y + 1, &clr_fr);

            if (alpha_down == 0) clr_fr.r = -1;
            else {
                clr_fr.r *= img_wdg->base_clr.r / 255.f;
                clr_fr.g *= img_wdg->base_clr.g / 255.f;
                clr_fr.b *= img_wdg->base_clr.b / 255.f;
            }

            px->color = clr_fr.r == -1 ? px->color: clr_fr;
            px->bgcolor = clr_bg.r == -1 ? px->color: clr_bg;
            px->fore_reset = 1;
            px->back_reset = 1;
            
            px->unich = i[0];
        }
    }

    free(i);
}

void draw_text(
    struct tgr_app *app, 
    const struct Text *text, 
    struct Rect rect
) {
    u64 max_chars = rect.w;
    u64 text_len = utf32_strlen(text->unicode_txt);
    // u64 num_chars = min(text_len, max_chars);
    
    i64 x = rect.x, y = rect.y;
    i64 bx = rect.x + rect.w, by = rect.y + rect.h; // borders
    for (u64 i = 0; i < text_len; i++) {
        uint32_t codepoint = (uint32_t)text->unicode_txt[i];
        
        if (codepoint == '\n'){
            if (y >= by) break;
            x = rect.x;
            y++;
            continue;
        }

        if (x >= bx){
            if (y >= by) break;
            x = rect.x;
            y++;
            // continue;
        }

        struct pixel *pix = tgr_tpx_get(app, x++, y);
        if (!pix) continue;

        if (!app->__frame_changed || pix->unich != codepoint) {
            app->__frame_changed = 1;
            pix->unich = codepoint;
            pix->color = text->base_clr;
            
            if (i == 0)
                strcpy(pix->prefix, text->style.effect);

            if (i == text_len - 1){
                pix->fore_reset = 1;
                strcpy(pix->postfix, text->style.reset);
            }
        }
    }
}

void draw_box(
    struct tgr_app *app, 
    const struct Box *box, 
    struct Rect rect
){
    i64 x = rect.x, y = rect.y;
    i64 bx = rect.x + rect.w, by = rect.y + rect.h; // borders

    if (rect.h < 2 || rect.w < 2) 
        return;

    const uint8_t *chars;
    switch (box->type){
        case BOX_ONE_LINE: 
            chars = (uint8_t*)__SYMBOLS_BOX_ONE; break;
        case BOX_DOUBLE: 
            chars = (uint8_t*)__SYMBOLS_BOX_DOUBLE; break;
        case BOX_ROUNDED: 
            chars = (uint8_t*)__SYMBOLS_BOX_ROUNDED; break;
        case BOX_PRIMITIVE: 
            chars = (uint8_t*)__SYMBOLS_PRIMITIVE; break;
    }

    if (chars == NULL) abort();

    int32_t *unichars;
    utf8_conv(chars, &unichars);

    struct pixel *pix;
    pix = tgr_tpx_get(app, x, y);
    if (pix){
        pix->color = box->color;
        pix->unich = unichars[0];
    }

    for (u64 i = x + 1; i < bx - 1; i++){
        struct pixel *pix = tgr_tpx_get(app, i, y);
        if (pix){
            pix->color = box->color;
            pix->unich = unichars[4];
        }
    } 

    pix = tgr_tpx_get(app, bx - 1, y);
    if (pix){
        pix->color = box->color;
        pix->unich = unichars[1];
    }

    y++;
    for (; y < by - 1; y++){
        pix = tgr_tpx_get(app, x, y);
        if (pix){
            pix->color = box->color;
            pix->unich = unichars[5];
            pix->fore_reset = 1;
        }

        pix = tgr_tpx_get(app, bx - 1, y);
        if (pix){
            pix->color = box->color;
            pix->unich = unichars[5];
            pix->fore_reset = 1;
        }
    }

    pix = tgr_tpx_get(app, x, y);
    if (pix){
        pix->color = box->color;
        pix->unich = unichars[2];
    }

    for (u64 i = x + 1; i < bx - 1; i++){
        struct pixel *pix = tgr_tpx_get(app, i, y);
        if (pix){
            pix->color = box->color;
            pix->unich = unichars[4];
        }
    }

    pix = tgr_tpx_get(app, bx - 1, y);
    if (pix){
        pix->color = box->color;
        pix->unich = unichars[3];
    }

    free(unichars);
}

void text_cpy(
    struct Text *dst,
    struct Text *src
){
    dst->base_clr = src->base_clr;
    dst->style = src->style;
    // if (dst->unicode_txt == NULL)
    dst->unicode_txt = (i32*)malloc(sizeof(i32) * (utf32_strlen(src->unicode_txt) + 1));
    memcpy(dst->unicode_txt, src->unicode_txt, sizeof(i32) * (utf32_strlen(src->unicode_txt) + 1));
}

void imgwg_cpy(
    struct Image *dst,
    struct Image *src
){
    dst->base_clr = src->base_clr;
    dst->is_dense = src->is_dense;
    img_cpy(&dst->img, &src->img);
}

void box_cpy(
    struct Box *dst,
    struct Box *src
){
    dst->color = src->color;
    dst->type = src->type;
}

void free_text_wg(
    struct Text *text
){
    free(text->unicode_txt);
    text->unicode_txt = NULL;
}


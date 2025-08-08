#pragma once

#include <web/widgets/widget.h>
#include <xml.h>

void xml_addwidgets(
    const char *path_to_dir,

    struct tgr_app *app,
    struct Widget **ptr,
    struct tag *root,
    int level
);

void init_widget(const char *path_to_dir, struct tag *tag, struct Widget **out);
void init_container(struct tag *tag, struct Widget **out);

void init_box(struct tag *tag, struct Widget **out);
void init_text(struct tag *tag, struct Widget **out);
void init_img(const char *path_to_dir, struct tag *tag, struct Widget **out);
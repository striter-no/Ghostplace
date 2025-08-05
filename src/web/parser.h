#pragma once

#include <web/widgets/widget.h>
#include <xml.h>

void init_widget(struct tag *tag, struct Widget **out);

void xml_addwidgets(
    struct tgr_app *app,
    struct Widget **ptr,
    struct tag *root,
    int level
);

void init_widget(struct tag *tag, struct Widget **out);
void init_container(struct tag *tag, struct Widget **out);

void init_box(struct tag *tag, struct Widget **out);
void init_text(struct tag *tag, struct Widget **out);
void init_img(struct tag *tag, struct Widget **out);
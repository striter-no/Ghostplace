#pragma once
#include <web/widgets/widget.h>
#include <css.h>

void __get_widgets(struct Widget *main_cnt, struct Widget ***widgets, u64 *widgets_sz);
void __get_relp(struct Widget *cntwg, u64 uid, struct WidgetRelp **out);
const char *__str_wgtype(enum WIDGET_TYPE type);

void css_parsewidgets(
    struct Widget **main_cnt,
    struct css_block *blocks
);
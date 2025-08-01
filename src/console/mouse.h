#pragma once
#include <int.h>
#include <console/keyboard.h>
#include <console/term.h>
#include <string.h>

enum MOUSE_BUTTONS {
    MOUSE_RIGHT_BTN,
    MOUSE_LEFT_BTN,
    MOUSE_MIDDLE_BTN,
    MOUSE_NO_BTN
};

struct Mouse {
    enum MOUSE_BUTTONS btn;
    byte scroll_h;
    u64 x, y;

    byte shifted;
    byte alted;
    byte ctrled;
};

void enable_mouse();
void disable_mouse();
byte process_mouse(struct Mouse *mouse, byte *bytes, u64 size);
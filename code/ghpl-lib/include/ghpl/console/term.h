#pragma once

#include <termios.h>
#include <stdint.h>
#include <stddef.h>
#include <ghpl/utils/int.h>

#define MAX_BUFFER 65536

struct terminal {
    struct termios old_attr;
    struct termios new_attr;
};

int term_new(struct terminal *term);
int term_dem(size_t *width, size_t *height);
int term_reset(struct terminal *term);
int term_write(byte *bytes, size_t num);
int term_read(byte **bytes, size_t *osize);

/*
"\x1d"      "ctrl + ]"
"\x1b"      "ctrl + ["
"\x09"      "tab"
"\x0a"      "nl"
"\x7f"      "bs"
"\x08"      "ctrl + bs"
"\x19"      "ctrl + y"
"\x12"      "ctrl + r"
"\x14"      "ctrl + t"
"\x0c"      "ctrl + l"
"\x0b"      "ctrl + k"
"\x0f"      "ctrl + o"
"\x10"      "ctrl + p"
"\x02"      "ctrl + b"
"\x18"      "ctrl + x"
"\x06"      "ctrl + f"
"\x01"      "ctrl + a"
"\x16"      "ctrl + v"
"\033"      "esc",   
"\033[A"    "up",  
"\033[B"    "down",
"\033[C"    "right"
"\033[D"    "left",
"\x1b[H"    "home",
"\033[F"    "end", 
"\033[2~"   "insert",
"\033[3~"   "delete",
"\033[5~"   "page_up"
"\033[6~"   "page_down"
"\033[Z"    "shift + tab",
"\033OP"    "f1"
"\033OQ"    "f2"
"\033OR"    "f3"
"\033OS"    "f4"
"\033[15~"  "f5"
"\033[17~"  "f6"
"\033[18~"  "f7"
"\033[19~"  "f8"
"\033[20~"  "f9"
"\033[21~"  "f10"
"\033[23~"  "f11"
"\033[24~"  "f12"
*/
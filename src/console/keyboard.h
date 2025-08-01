#pragma once
#include <int.h>
#include <utf8.h>
#include <table.h>
#include <stdio.h>
#include <debug.h>

struct ESC_SEQ {
    char esc[18];
    char name[15];
};

extern const struct ESC_SEQ __esc_map[];

struct Key {
    byte is_esc;
    char esc_name[15];

    u32 unich;
    byte alted;
    byte shifted;
    byte ctrled;
};

struct Keyboard {
    struct Table pressed_keys;
};

struct Key keyc(const char *unich);
struct Key ukeyc(u32 unich);
struct Key keye(const char *name);

void create_kboard(struct Keyboard *kb);
void free_keyboard(struct Keyboard *kb);

byte is_normal(i32 ch);
byte key_cmp(struct Key k1, struct Key k2);

void parse_bytes(ubyte *bytes, u64 size, struct Key *key);
byte kb_process_input(struct Keyboard *kb, ubyte *bytes, u64 size);
byte is_key_pressed(struct Keyboard *kb, struct Key key);
byte get_pressed_key(struct Keyboard *kb, struct Key *key);
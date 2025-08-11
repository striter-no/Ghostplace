#include <ghpl/console/keyboard.h>

const struct ESC_SEQ __esc_map[] = {
    {"\x1d", "ctrl + ]"},
    // {"\x1b", "ctrl + ["},
    {"\x09", "tab"},
    {"\x0a", "enter"},
    {"\x7f", "bs"},
    {"\x08", "ctrl + bs"},
    {"\x19", "ctrl + y"},
    {"\x12", "ctrl + r"},
    {"\x14", "ctrl + t"},
    {"\x0c", "ctrl + l"},
    {"\x0b", "ctrl + k"},
    {"\x0f", "ctrl + o"},
    {"\x10", "ctrl + p"},
    {"\x02", "ctrl + b"},
    {"\x18", "ctrl + x"},
    {"\x06", "ctrl + f"},
    {"\x01", "ctrl + a"},
    {"\x16", "ctrl + v"},
    {"\033", "esc"},
    {"\033[A", "up"},
    {"\033[B", "down"},
    {"\033[C", "right"},
    {"\033[D", "left"},
    {"\x1b[H", "home"},
    {"\033[F", "end"},
    {"\033[2~", "insert"},
    {"\033[3~", "delete"},
    {"\033[5~", "page_up"},
    {"\033[6~", "page_down"},
    {"\033[Z", "shift + tab"},
    {"\033OP", "f1"},
    {"\033OQ", "f2"},
    {"\033OR", "f3"},
    {"\033OS", "f4"},
    {"\033[15~", "f5"},
    {"\033[17~", "f6"},
    {"\033[18~", "f7"},
    {"\033[19~", "f8"},
    {"\033[20~", "f9"},
    {"\033[21~", "f10"},
    {"\033[23~", "f11"},
    {"\033[24~", "f12"}
};

void create_kboard(struct Keyboard *kb){
    kb->pressed_keys = create_table(sizeof(u64), sizeof(struct Key));    
}

void free_keyboard(struct Keyboard *kb){
    clear_table(&kb->pressed_keys);
}

struct Key ukeyc(u32 unich){
    struct Key o = {0};
    o.unich = unich;
    return o;
}

struct Key keyc(const char *unich){
    struct Key o = {0};
    int32_t *i;
    int res = utf8_conv(unich, &i);
    
    o.unich = res != 0 ? 0: i[0];
    free(i);
    return o;
}

struct Key keye(const char *name){
    struct Key o = {0};
    o.is_esc = 1;
    strcpy(o.esc_name, name);
    return o;
}

byte is_normal(i32 ch){
    return (ch >= 32 && ch <= 126) || ch >= 160;
}

byte key_cmp(struct Key k1, struct Key k2){
    return (
        k1.is_esc == k2.is_esc &&
        (0 == memcmp(k1.esc_name, k2.esc_name, 15)) &&
        k1.unich == k2.unich &&
        k1.alted == k2.alted &&
        k1.shifted == k2.shifted &&
        k1.ctrled == k2.ctrled
    );
}

int starts_with(const ubyte *buf, u64 buf_len, const char *prefix, size_t prefix_len) {
    return (buf_len >= prefix_len) && 
           (memcmp(buf, prefix, prefix_len) == 0);
}

void parse_bytes(ubyte *bytes, u64 size, struct Key *key) {
    *key = (struct Key){0};
    if (size == 0) return;

    static const char *shift_prefix = "\x1b[1;2"; 
    static const char *ctrl_prefix  = "\x1b[1;5";
    static const char *alt_prefix   = "\x1b[1;3";
    static const size_t prefix_len = 5;

    if (size > 0 && !is_normal(bytes[0])) {
        key->is_esc = 1;
        
        if (size >= prefix_len) {
            key->shifted = starts_with(bytes, size, shift_prefix, prefix_len);
            key->ctrled  = starts_with(bytes, size, ctrl_prefix, prefix_len);
            key->alted   = starts_with(bytes, size, alt_prefix, prefix_len);
            
            if (key->shifted || key->ctrled || key->alted) {
                bytes += 3;
                size -= 3; 
            }
        }

        for (u64 i = 0; i < sizeof(__esc_map) / sizeof(*__esc_map); i++) {
            if (size == strlen(__esc_map[i].esc) && 
                memcmp(bytes, __esc_map[i].esc, size) == 0) {
                strcpy(key->esc_name, __esc_map[i].name);
                break;
            }
        }
    }

    else if (size > 0) {
        key->is_esc = 0;
        int32_t *i;
        int res = utf8_nconv(bytes, &i, size);
        
        key->unich = res != 0 ? 0: i[0];
        free(i);
    }
}

byte kb_process_input(struct Keyboard *kb, ubyte *bytes, u64 size){
    struct Key pressed;
    parse_bytes(bytes, size, &pressed);

    if (pressed.is_esc && pressed.esc_name[0] == '\0')
        return 0;
    
    if (!pressed.is_esc && pressed.unich == '\0')
        return 0;

    u64 uid = rand() % x_u64;
    
    int i = table_add(&kb->pressed_keys, &uid, &pressed);
    if (i !=0 ){
        printf("\n%i stats\n", i);
        abort();
    }

    return 1;
}

byte is_key_pressed(struct Keyboard *kb, struct Key key){
    // If key in table
    // remove key
    struct Table *tb = &kb->pressed_keys;
    printf("\n%u\n", tb->size);
    abort();
    for (u64 i = 0; i < tb->size; i++){
        struct Key tmpk;
        table_at(
            tb,
            (ubyte*)tb->keys + i * tb->key_size,
            &tmpk
        );
        printf("\n%u\n", tmpk.unich);
        abort();
        // byte isit = key_cmp(tmpk, key);
        if (tmpk.unich == key.unich) {
            table_rem(tb, (ubyte*)tb->keys + i * tb->key_size);
            return 1;
        }
    }

    return 0;
}

byte get_pressed_key(struct Keyboard *kb, struct Key *key){
    // Get first key
    // remove first key
    struct Table *tb = &kb->pressed_keys;
    if (tb->size == 0) return 0;

    table_at(tb, (ubyte*)tb->keys, key);
    table_rem(tb, (ubyte*)tb->keys);

    return 1;
}
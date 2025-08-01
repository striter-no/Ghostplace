#include <console/mouse.h>


void enable_mouse(){
    term_write("\033[?1003h", 8);
    term_write("\033[?1006h", 8);
}

void disable_mouse(){
    term_write("\033[?1006l", 8);
    term_write("\033[?1003l", 8);
}

byte process_mouse(struct Mouse *mouse, byte *bytes, u64 size) {
    if (size < 5) return 0;
    if (bytes[0] != '\033' || bytes[1] != '[' || bytes[2] != '<') 
        return 0;

    // Копируем с завершающим \0
    byte *cpbuff = malloc(size + 1);
    memcpy(cpbuff, bytes, size);
    cpbuff[size] = '\0';

    // Разбиваем строку
    char *parts[3] = {0};
    char *rest = cpbuff + 3; // Пропускаем \033[<
    parts[0] = strtok_r(rest, ";", &rest);
    parts[1] = strtok_r(NULL, ";", &rest);
    parts[2] = strtok_r(NULL, "", &rest);

    // Проверка частей
    if (!parts[0] || !parts[1] || !parts[2]) {
        free(cpbuff);
        return 0;
    }

    // Парсим данные
    int btn = atoi(parts[0]);
    mouse->x = atoi(parts[1]) - 1; // Терминалы используют 1-индексацию

    // Удаляем M/m из координаты Y
    size_t y_len = strlen(parts[2]);
    if (y_len > 0 && (parts[2][y_len-1] == 'M' || parts[2][y_len-1] == 'm')) {
        parts[2][y_len-1] = '\0';
    }
    mouse->y = atoi(parts[2]) - 1;

    mouse->shifted = (btn & 0x08) != 0; // Shift: бит 3
    mouse->alted   = (btn & 0x10) != 0; // Alt: бит 4
    mouse->ctrled  = (btn & 0x20) != 0; // Ctrl: бит 5

    // Обработка колесика (с учётом модификаторов!)
    if ((btn & 0x7F) == 64) { // Колесо вверх (64 + модификаторы)
        mouse->scroll_h = -1;
        mouse->btn = MOUSE_NO_BTN;
    } 
    else if ((btn & 0x7F) == 65) { // Колесо вниз (65 + модификаторы)
        mouse->scroll_h = 1;
        mouse->btn = MOUSE_NO_BTN;
    } 
    // Обработка кнопок
    else {
        mouse->scroll_h = 0;
        switch (btn & 0x07) { // Очищаем биты модификаторов
            case 0: mouse->btn = MOUSE_LEFT_BTN; break;
            case 1: mouse->btn = MOUSE_MIDDLE_BTN; break;
            case 2: mouse->btn = MOUSE_RIGHT_BTN; break;
            default: mouse->btn = MOUSE_NO_BTN;
        }
    }

    free(cpbuff);
    return 1;
}
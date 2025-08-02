#pragma once
#include <utf8proc.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

/**
 * Преобразует UTF-8 строку в массив code points (int32_t)
 * 
 * @param utf8_cs Входная строка в кодировке UTF-8 (null-terminated)
 * @param wide_cs Указатель на указатель, куда будет записан адрес выделенной памяти
 * @return 0 при успехе, ненулевое значение при ошибке
 */
int utf8_conv(const uint8_t *utf8_cs, int32_t **wide_cs);

int utf8_nconv(const uint8_t *utf8_cs, int32_t **wide_cs, u64 byte_len);

/**
 * Преобразует массив code points (int32_t) в UTF-8 строку
 * 
 * @param wide_cs Входной массив code points (null-terminated)
 * @param utf8_cs Указатель на указатель, куда будет записан адрес выделенной памяти
 * @return 0 при успехе, ненулевое значение при ошибке
 */
int cstr_conv(const int32_t *wide_cs, uint8_t **utf8_cs);

size_t utf32_count(const int32_t *s, int32_t c);

// Аналог strlen для UTF-32
size_t utf32_strlen(const int32_t *s);

// Аналог strcpy для UTF-32
int32_t* utf32_strcpy(int32_t *dest, const int32_t *src);

// Аналог strcmp для UTF-32
int utf32_strcmp(const int32_t *s1, const int32_t *s2);

// Аналог strchr для UTF-32
int32_t* utf32_strchr(const int32_t *s, int32_t c);

int32_t *utf32_strncpy(int32_t *dest, const int32_t *src, size_t n);

/**
 * Потокобезопасный вариант utf32_strtok
 * 
 * @param str UTF-32 строка для разбиения
 * @param delim UTF-32 строка, содержащая разделители
 * @param saveptr Указатель на переменную для сохранения состояния между вызовами
 * @return Указатель на следующий токен или NULL, если токены закончились
 */
int32_t *utf32_strtok_r(int32_t *str, const int32_t *delim, int32_t **saveptr);
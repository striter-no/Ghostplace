#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <stdint.h>
#include <sys/stat.h>

size_t enum_directories(const char *path, char ***dirs);
size_t enum_files(const char *path, char ***files);
int readfile(const char *path, const char *mode, uint8_t **output, size_t *length);
int writefile(const char *path, const char *mode, const uint8_t *data, size_t length);
int mkdir_p(const char *path, mode_t mode);
char fileexists(const char *path);

void path_sanitize(char *path);
char *basepath(const char *path);

void free_list_cstr(char **dirs, size_t count);

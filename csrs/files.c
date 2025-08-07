#include <files.h>

void path_sanitize(char *path){
    if (path[strlen(path) - 1] == '/'){
        path[strlen(path) - 1] = '\0';
    }
}

char *basepath(const char *path) {
    char *result;
    size_t len;
    const char *start, *end;
    
    if (path == NULL)
        return NULL;
    
    if ((len = strlen(path)) == 0)
        return NULL;
    
    end = path + len - 1;
    while (end > path && (*end == '/' || *end == '\\'))
        end--;
    
    if (end < path) {
        result = malloc(2 * sizeof(char));
        if (result == NULL) return NULL;
        strcpy(result, "/");
        return result;
    }
    
    start = end;
    while (start > path && *(start - 1) != '/' && *(start - 1) != '\\')
        start--;
    
    size_t name_len = end - start + 1;
    result = malloc((name_len + 1) * sizeof(char));
    if (result == NULL)
        return NULL;
    
    strncpy(result, start, name_len);
    result[name_len] = '\0';
    
    return result;
}

size_t enum_directories(const char *path, char ***dirs) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    
    *dirs = NULL;
    
    dir = opendir(path);
    if (dir == NULL) {
        return 0; // Ошибка открытия каталога
    }
    
    size_t count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        if (entry->d_type == DT_DIR) {
            count++;
        } else if (entry->d_type == DT_UNKNOWN) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                count++;
            }
        }
    }
    
    if (count == 0) {
        closedir(dir);
        return 0;
    }
    
    rewinddir(dir);
    
    *dirs = (char**)malloc(count * sizeof(char*));
    if (*dirs == NULL) {
        closedir(dir);
        return 0;
    }
    
    size_t index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        int is_dir = 0;
        if (entry->d_type == DT_DIR) {
            is_dir = 1;
        } else if (entry->d_type == DT_UNKNOWN) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            if (stat(full_path, &path_stat) == 0 && S_ISDIR(path_stat.st_mode)) {
                is_dir = 1;
            }
        }
        
        if (is_dir) {
            (*dirs)[index] = (char*)malloc((strlen(entry->d_name) + 1) * sizeof(char));
            if ((*dirs)[index] == NULL) {
                for (size_t i = 0; i < index; i++) {
                    free((*dirs)[i]);
                }
                free(*dirs);
                *dirs = NULL;
                closedir(dir);
                return 0;
            }
            strcpy((*dirs)[index], entry->d_name);
            index++;
        }
    }
    
    closedir(dir);
    return count;
}

size_t enum_files(const char *path, char ***files) {
    DIR *dir;
    struct dirent *entry;
    struct stat path_stat;
    
    // Инициализируем выходной параметр
    *files = NULL;
    
    // Открываем каталог
    dir = opendir(path);
    if (dir == NULL) {
        return 0; // Ошибка открытия каталога
    }
    
    // Сначала подсчитаем количество файлов
    size_t count = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        // Если тип известен и это файл
        if (entry->d_type == DT_REG) {
            count++;
        } 
        // Если тип неизвестен, используем stat для проверки
        else if (entry->d_type == DT_UNKNOWN) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            if (stat(full_path, &path_stat) == 0 && S_ISREG(path_stat.st_mode)) {
                count++;
            }
        }
    }
    
    // Если файлов нет, возвращаем 0
    if (count == 0) {
        closedir(dir);
        return 0;
    }
    
    // Возвращаемся к началу каталога
    rewinddir(dir);
    
    // Выделяем память для массива указателей на строки
    *files = malloc(count * sizeof(char*));
    if (*files == NULL) {
        closedir(dir);
        return 0;
    }
    
    // Заполняем массив именами файлов
    size_t index = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        int is_file = 0;
        if (entry->d_type == DT_REG) {
            is_file = 1;
        } else if (entry->d_type == DT_UNKNOWN) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            if (stat(full_path, &path_stat) == 0 && S_ISREG(path_stat.st_mode)) {
                is_file = 1;
            }
        }
        
        if (is_file) {
            // Выделяем память для копии имени и копируем
            (*files)[index] = malloc((strlen(entry->d_name) + 1) * sizeof(char));
            if ((*files)[index] == NULL) {
                // При ошибке выделения памяти освобождаем уже выделенные ресурсы
                for (size_t i = 0; i < index; i++) {
                    free((*files)[i]);
                }
                free(*files);
                *files = NULL;
                closedir(dir);
                return 0;
            }
            strcpy((*files)[index], entry->d_name);
            index++;
        }
    }
    
    closedir(dir);
    return count;
}

void free_list_cstr(char **dirs, size_t count) {
    if (dirs == NULL) return;
    
    for (size_t i = 0; i < count; i++) {
        free(dirs[i]);
    }
    free(dirs);
}

int readfile(const char *path, const char *mode, uint8_t **output, size_t *file_size){
    FILE *file = fopen(path, mode);

    fseek(file, 0, SEEK_END);
    (*file_size) = ftell(file);
    fseek(file, 0, SEEK_SET);

    (*output) = (char*)malloc((*file_size) + 1);
    if (!(*output)) {
        fclose(file); perror("malloc");
        return 2;
    }

    size_t bytes_read = fread((*output), 1, (*file_size), file);
    if (bytes_read != (*file_size)) {
        fprintf(stderr, "Read error: read %zu from %ld bytes\n", bytes_read, (*file_size));
        free((*output));
        fclose(file);
        return 3;
    }
    (*output)[(*file_size)] = '\0';
    
    fclose(file);
    return 0;
}

int writefile(const char *path, const char *mode, const uint8_t *data, size_t length) {
    FILE *file = fopen(path, mode);
    
    if (!file) {
        perror("fopen");
        return 1;
    }
    
    if (data != NULL) {
        size_t bytes_written = fwrite(data, 1, length, file);
        
        if (bytes_written != length) {
            fprintf(stderr, "Write error: written %zu from %zu bytes\n", 
                    bytes_written, length);
            fclose(file);
            return 2;
        }
    }
    
    if (fflush(file) != 0 || fclose(file) != 0) {
        perror("file operations");
        return 2;
    }
    
    return 0;
}

int mkdir_p(const char *path, mode_t mode) {
    char temp_path[1024];
    char *p = NULL;
    size_t len;
    
    if (path == NULL || strlen(path) == 0)
        return -1;
    
    strncpy(temp_path, path, sizeof(temp_path) - 1);
    temp_path[sizeof(temp_path) - 1] = '\0';
    
    len = strlen(temp_path);
    
    if (temp_path[len - 1] == '/')
        temp_path[len - 1] = '\0';
    
    p = temp_path;
    if (*p == '/')
        p++;

    while ((p = strchr(p, '/')) != NULL) {
        *p = '\0';
        
        if (mkdir(temp_path, mode) != 0 && errno != EEXIST)
            return -1;
        
        *p = '/';
        p++;
    }
    
    if (mkdir(temp_path, mode) != 0 && errno != EEXIST)
        return -1;
    return 0;
}

char fileexists(const char *path) {
    struct stat buffer;
    
    if (path == NULL)
        return 0;
    
    if (stat(path, &buffer) == 0)
        return 1;

    return 0;
}
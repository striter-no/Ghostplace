#include <webnet/proto.h>


void proto_msg_free(struct proto_msg *msg){
    free(msg->path);
    free(msg->content);
    msg->path = NULL;
    msg->content = NULL;
}

/*
TYPE cont_type ver
path cont_size
cont
*/
void proto_constr(
    struct proto_msg *msg,
    u64 proto_ver,
    enum PROTO_MSG_TYPE type,
    enum PROTO_CONT_TYPE ctype
){
    msg->proto_ver = proto_ver;
    msg->type = type;

    msg->path = (char*)malloc(1);
    msg->path[0] = '\0';
    
    msg->conttype = ctype;
    msg->content = NULL;
    msg->cont_size = 0;
}

static u32 count_dig_num(u32 num){
    if (num >= 0 && num <= 9) return 1;
    
    u32 i = 0;
    while (num > 0) {
        num /= 10;
        i++;
    }
    return i;
}

void proto_print(struct proto_msg *msg){
    switch (msg->type){
        case GET: printf("GET "); break;
        case POST: printf("POST "); break;
    }

    switch (msg->conttype){
        case TEXT_CONT: printf("text "); break;
        case IMAGE_CONT: printf("img "); break;
        case BIN_CONT: printf("bin "); break;
    }

    printf("%d\n", msg->proto_ver);
    printf("%s ", msg->path);
    printf("%zu\n", msg->cont_size);
    
    char *buffer = (char*)malloc(msg->cont_size + 1);
    memcpy(buffer, msg->content, msg->cont_size);
    buffer[msg->cont_size] = '\0';
    printf("%s\n", buffer);

    free(buffer);
}

/*
MSG_TYPE CONTTYPE PROTO_VER PATH CONT_SIZE\n
CONTENT
*/
void proto_serial(
    struct proto_msg *inp,
    struct qbuffer *buff
){
    u64 umsg_type = inp->type,
        ucont_type = inp->conttype,
        uproto_ver = inp->proto_ver,
        ucont_size = inp->cont_size;
    
    size_t all_sizes = count_dig_num(umsg_type) + 1 + /* for ' ' */
                       count_dig_num(ucont_type) + 1 + /* for ' '*/
                       count_dig_num(uproto_ver) + 1 + /* for ' '*/
                       count_dig_num(ucont_size) + 1; /* for '\n'*/
    
    all_sizes += strlen(inp->path) + 1;

    create_qbuffer(buff, all_sizes + ucont_size);
    char *header = (char*)malloc(all_sizes + 1);
    printf("[debug] all size: %d\n", all_sizes);
    sprintf(header, "%d %d %d %s %d\n", umsg_type, ucont_type, uproto_ver, inp->path, ucont_size);
    
    memcpy(buff->bytes, header, all_sizes);
    memcpy(buff->bytes + all_sizes, inp->content, ucont_size);
    free(header);
}

/*
MSG_TYPE CONTTYPE PROTO_VER PATH CONT_SIZE\n
CONTENT
*/
void proto_deserial(
    struct proto_msg *out,
    struct qbuffer *buff
){
    // Инициализируем выходную структуру
    memset(out, 0, sizeof(struct proto_msg));
    
    // Проверяем минимальный размер буфера (даже для пустого заголовка)
    if (buff->size < 10) {
        fprintf(stderr, "[proto][error] buffer too small for header\n");
        return;
    }
    
    // Ищем конец заголовка (первый '\n')
    size_t header_end = 0;
    while (header_end < buff->size && buff->bytes[header_end] != '\n') {
        header_end++;
    }
    
    // Если не найден конец заголовка
    if (header_end >= buff->size || buff->bytes[header_end] != '\n') {
        fprintf(stderr, "[proto][error] header delimiter not found\n");
        return;
    }
    
    // Создаем временный буфер для заголовка (без \n)
    char *header = (char *)malloc(header_end + 1);
    if (!header) {
        fprintf(stderr, "[proto][error] memory allocation failed\n");
        return;
    }
    
    // Копируем заголовок и добавляем терминатор
    memcpy(header, buff->bytes, header_end);
    header[header_end] = '\0';
    
    // Разбираем заголовок
    char *token;
    char *saveptr;
    
    // MSG_TYPE
    token = strtok_r(header, " ", &saveptr);
    if (!token) {
        fprintf(stderr, "[proto][error] missing MSG_TYPE\n");
        free(header);
        return;
    }
    out->type = (enum PROTO_MSG_TYPE)atoi(token);
    
    // CONTTYPE
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) {
        fprintf(stderr, "[proto][error] missing CONTTYPE\n");
        free(header);
        return;
    }
    out->conttype = (enum PROTO_CONT_TYPE)atoi(token);
    
    // PROTO_VER
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) {
        fprintf(stderr, "[proto][error] missing PROTO_VER\n");
        free(header);
        return;
    }
    out->proto_ver = atoi(token);
    
    // PATH
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) {
        fprintf(stderr, "[proto][error] missing PATH\n");
        free(header);
        return;
    }
    out->path = strdup(token);
    
    // CONT_SIZE
    token = strtok_r(NULL, " ", &saveptr);
    if (!token) {
        fprintf(stderr, "[proto][error] missing CONT_SIZE\n");
        free(header);
        return;
    }
    out->cont_size = (size_t)atoi(token);
    
    // Проверяем, что данные контента присутствуют
    size_t content_start = header_end + 1; // +1 для \n
    if (content_start + out->cont_size > buff->size) {
        fprintf(stderr, "[proto][error] content size mismatch (expected %zu, available %zu)\n",
                out->cont_size, buff->size - content_start);
        free(header);
        free(out->path);
        memset(out, 0, sizeof(struct proto_msg));
        return;
    }
    
    // Выделяем память для контента и копируем
    out->content = (uint8_t*)malloc(out->cont_size);
    if (!out->content) {
        fprintf(stderr, "[proto][error] content memory allocation failed\n");
        free(header);
        free(out->path);
        memset(out, 0, sizeof(struct proto_msg));
        return;
    }
    
    memcpy(out->content, buff->bytes + content_start, out->cont_size);
    
    // Очищаем временные ресурсы
    free(header);
}

void proto_copy(
    struct proto_msg *dest,
    struct proto_msg *src
){
    dest->cont_size = src->cont_size;
    dest->conttype = src->conttype;
    dest->proto_ver = src->proto_ver;
    dest->type = src->type;

    dest->content = (uint8_t*)malloc(src->cont_size);
    memcpy(dest->content, src->content, src->cont_size);

    dest->path = (char*)malloc(strlen(src->path) + 1);
    strcpy(dest->path, src->path);
}
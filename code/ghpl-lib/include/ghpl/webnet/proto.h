#pragma once
#include <ghpl/utils/int.h>
#include <ghpl/utils/queue.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

enum PROTO_MSG_TYPE {
    GET, POST
};

enum PROTO_CONT_TYPE {
    TEXT_CONT, IMAGE_CONT, BIN_CONT
};

struct proto_msg {
    u64 proto_ver;
    enum PROTO_MSG_TYPE type;
    enum PROTO_CONT_TYPE conttype;

    char   *path;
    ubyte  *content;
    size_t cont_size;
};

void proto_msg_free(struct proto_msg *msg);
void proto_constr(
    struct proto_msg *msg,
    u64 proto_ver,
    enum PROTO_MSG_TYPE type,
    enum PROTO_CONT_TYPE ctype
);

void proto_print(struct proto_msg *msg);

/*
MSG_TYPE CONTTYPE PROTO_VER PATH CONT_SIZE\n
CONTENT
*/
void proto_serial(
    struct proto_msg *inp,
    struct qbuffer *buff
);

/*
MSG_TYPE CONTTYPE PROTO_VER PATH CONT_SIZE\n
CONTENT
*/
int proto_deserial(
    struct proto_msg *out,
    struct qbuffer *buff
);

void proto_copy(
    struct proto_msg *dest,
    struct proto_msg *src
);
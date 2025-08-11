#pragma once

#include <ghpl/utils/files.h>
#include <ghpl/webnet/site.h>
#include <ghpl/webnet/net/tcpserv.h>

struct router {
    pthread_t detached_thr;
    struct TCP_server server;
    char *store_dir;

    struct site *sites;
    size_t sites_num;
};

void *__router_detached_tcp(void *args);

void create_router(
    struct router *router,
    const char *store_dir,

    const char *bind_ip,
    int port, 
    size_t cli_max
);

void destroy_router(struct router *router);

void run_router(struct router *router, void (*router_calllback)(struct qbuffer *input, struct qbuffer *output));

void save_sites_db(
    struct site *inp,
    size_t sites_num,

    const char *main_dirpath
);

void load_sites_db(
    struct site **out,
    size_t *sites_num,

    const char *main_dirpath
);
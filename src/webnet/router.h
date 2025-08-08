#pragma once
// #include <sqlite3.h>

#include <files.h>
#include "site.h"
#include "net/tcpserv.h"

struct router {
    pthread_t detached_thr;
    struct TCP_server server;

    char *domain;
    char *store_dir;

    struct site *sites;
    size_t sites_num;
};

void *__router_detached_tcp(void *args){
    struct TCP_server *serv = (struct TCP_server *)args;
    tcp_loop(serv);
    pthread_exit(NULL);
}

void create_router(
    struct router *router,
    const char *domain,
    const char *store_dir,

    const char *bind_ip,
    int port
){
    tcp_create(&router->server, bind_ip, port);
    router->sites = NULL;
    router->sites_num = 0;

    router->domain = strdup(domain);
    router->store_dir = strdup(store_dir);
}

void destroy_router(struct router *router){
    tcp_end_server(&router->server);
    free(router->domain);
    free(router->store_dir);
    router->domain = NULL;
    router->store_dir = NULL;

    for (size_t i = 0; i < router->sites_num; i++)
        destroy_site(&router->sites[i]);
    
    free(router->sites);
    router->sites = NULL;
    router->sites_num = 0;
}

void router_calllback(
    struct qbuffer *input,
    struct qbuffer *output
){
    copy_qbuffer(output, input);
}

void run_router(struct router *router){
    pthread_create(&router->detached_thr, NULL, __router_detached_tcp, (void*)&router->server);
    pthread_detach(router->detached_thr);

    sleep(2);
    struct TCP_server *serv = &router->server;
    while (router->server.running){
        pthread_mutex_lock(&serv->climtx);

        for (size_t i = 0; i < serv->act_clients_n; i++){
            struct __TCP_serv_cli *cli = &serv->clients[i];
            struct qbuffer ibuff, obuff;

            int has_inp = pop_buffer(&cli->input_queue, &ibuff);
            if (has_inp != 0) continue;

            printf("[log] buffer: %s\n", ibuff.bytes);
            router_calllback(&ibuff, &obuff);
            push_buffer(&cli->output_queue, &obuff);

            clear_qbuffer(&ibuff);
            clear_qbuffer(&obuff);
        }

        pthread_mutex_unlock(&serv->climtx);
    }
}

void save_sites_db(
    struct site *inp,
    size_t sites_num,

    const char *main_dirpath
){
    for (size_t i = 0; i < sites_num; i++){
        save_site(&inp[i], main_dirpath);
    }
}

void load_sites_db(
    struct site *out,
    size_t *sites_num,

    const char *main_dirpath
){
    char **dirs = NULL;
    size_t dir_num = enum_directories(main_dirpath, &dirs);
    
    *sites_num = dir_num;
    out = (struct site*)malloc(sizeof(struct site) * dir_num);
    
    for (size_t i = 0; i < dir_num; i++){
        load_site(&out[i], main_dirpath, dirs[i]);
    }

    free_list_cstr(dirs, dir_num);
}
#include <webnet/router.h>

static size_t strinx(const char *str, const char *tofind) {
    if (!str || !tofind) return (size_t)-1; // проверка на NULL
    
    for (size_t i = 0; str[i] != '\0'; i++) {
        size_t j = 0;
        while (tofind[j] != '\0' && str[i + j] == tofind[j]) {
            j++;
        }
        if (tofind[j] == '\0') {
            return i;
        }
    }
    return (size_t)-1;
}


void *__router_detached_tcp(void *args){
    struct TCP_server *serv = (struct TCP_server *)args;
    tcp_loop(serv);
    pthread_exit(NULL);
}

void create_router(
    struct router *router,
    const char *store_dir,

    const char *bind_ip,
    int port, 
    size_t cli_max
){
    tcp_create(&router->server, bind_ip, port);
    tcp_listen(&router->server, cli_max);
    router->sites = NULL;
    router->sites_num = 0;
    router->store_dir = strdup(store_dir);
}

void destroy_router(struct router *router){
    free(router->store_dir);
    router->store_dir = NULL;

    for (size_t i = 0; i < router->sites_num; i++)
        destroy_site(&router->sites[i]);
    
    free(router->sites);
    router->sites = NULL;
    router->sites_num = 0;
}

void run_router(struct router *router, void (*router_calllback)(struct qbuffer *input, struct qbuffer *output)){
    pthread_create(&router->detached_thr, NULL, __router_detached_tcp, (void*)&router->server);
    sleep(2);
    
    struct TCP_server *serv = &router->server;
    while (router->server.running){
        pthread_mutex_lock(&serv->climtx);

        for (size_t i = 0; i < serv->act_clients_n; i++){
            struct __TCP_serv_cli *cli = &serv->clients[i];
            struct qbuffer ibuff, obuff;

            int has_inp = pop_buffer(&cli->input_queue, &ibuff);
            if (has_inp != 0) continue;

            // !*printf("[log] buffer: %s\n", ibuff.bytes);
            router_calllback(&ibuff, &obuff);
            push_buffer(&cli->output_queue, &obuff);

            clear_qbuffer(&ibuff);
            clear_qbuffer(&obuff);
        }

        pthread_mutex_unlock(&serv->climtx);
    }

    tcp_end_server(&router->server);
    pthread_join(router->detached_thr, NULL);
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
    struct site **out,
    size_t *sites_num,

    const char *main_dirpath
){
    char **dirs = NULL;
    size_t dir_num = enum_directories(main_dirpath, &dirs);
    
    *sites_num = dir_num;
    (*out) = (struct site*)malloc(sizeof(struct site) * dir_num);
    
    for (size_t i = 0; i < dir_num; i++){
        load_site(&(*out)[i], main_dirpath, dirs[i]);
    }

    free_list_cstr(dirs, dir_num);
}
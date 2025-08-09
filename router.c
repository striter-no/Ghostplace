#include <webnet/router.h>
#include <strutils.h>

#define UPD_SECONDS_DELAY 60 * 60

pthread_mutex_t sites_db_mtx = PTHREAD_MUTEX_INITIALIZER;

struct site *sites_db = NULL;
size_t sites_num = 0;

void callback(
    struct qbuffer *inp,
    struct qbuffer *out
){
    struct proto_msg inp_msg, out_msg;
    int status = proto_deserial(&inp_msg, inp);
    if (status != 0){
        printf("[warning][callback] deserial failure\n");
        out_msg = (struct proto_msg){
            .type = GET,
            .path = strdup("/"),
            .content = strdup("proto-err"),
            .cont_size = strlen("proto-err") + 1,
            .conttype = TEXT_CONT,
            .proto_ver = 0
        };

        proto_serial(&out_msg, out);
        proto_msg_free(&out_msg);
        
        return;
    }

    char **tokens = NULL;
    size_t ntoks = toksplit(inp_msg.path, "/", &tokens);
    if (ntoks == 0){
        printf("[warning][callback] ntoks == 0\n");
        out_msg = (struct proto_msg){
            .type = GET,
            .path = strdup("/"),
            .content = strdup("proto-err"),
            .cont_size = strlen("proto-err") + 1,
            .conttype = TEXT_CONT,
            .proto_ver = 0
        };

        proto_serial(&out_msg, out);
        proto_msg_free(&out_msg);
        
        return;
    }

    const char *domain = tokens[0];
    char *real_path = malloc(strlen(inp_msg.path) - strlen(domain) + 2);
    strcpy(real_path, inp_msg.path + strlen(domain));
    free(inp_msg.path);
    inp_msg.path = real_path;

    printf("\n[log] requested site \"%s\" | path \"%s\"\n", domain, inp_msg.path);    

    struct site *requested_site;
    pthread_mutex_lock(&sites_db_mtx);
    find_site(sites_db, sites_num, domain, &requested_site);
    pthread_mutex_unlock(&sites_db_mtx);

    if (strcmp(inp_msg.path, "/") == 0) {
        compose_enum(requested_site, &out_msg);
    } else {
        compose_by_path(requested_site, inp_msg.path, &out_msg);
    }
    proto_serial(&out_msg, out);

    proto_msg_free(&inp_msg);
    proto_msg_free(&out_msg);
    free_list_cstr(tokens, ntoks);
}

void *update_db_thread(void *args){
    struct router *router = (struct router*)args;
    while (!router->server.running){
        sleep(1);
    } // busylooping

    printf("[log] starting to updating sites (from \"%s\" dir)\n", router->store_dir);
    while (router->server.running){
        pthread_mutex_lock(&sites_db_mtx);
        printf("[log] updating sites from %zu active...\n", sites_num);
        load_sites_db(&sites_db, &sites_num, router->store_dir);
        printf("[log] updated sites DB, %zu sites active\n", sites_num);
        pthread_mutex_unlock(&sites_db_mtx);

        sleep(UPD_SECONDS_DELAY);
    }
    printf("[log] stopped updating sites\n");

    pthread_exit(NULL);
}

int main(int argc, char *argv[]){
    if (argc < 3){
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }

    if (strlen(argv[1]) >= 20){
        fprintf(stderr, "[error] argv[1] (ip) is more than 19 simbols");
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }

    if (atoi(argv[2]) == 0){
        fprintf(stderr, "[error] argv[2] (port) can not be equal to zero (or be non-digital)");
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }
    struct router router;

    create_router(
        &router,
        "./assets/sites",
        argv[1],
        atoi(argv[2]),
        20
    );
    
    pthread_t upd_thr;
    pthread_create(&upd_thr, NULL, update_db_thread, (void*)&router);
    pthread_detach(upd_thr);
    
    run_router(&router, callback);

    for (size_t i = 0; i < sites_num; i++){
        // !*printf("[log] site: %s\n", sites_db[i].domain_name);
        destroy_site(&sites_db[i]);
    }
    free(sites_db);
    destroy_router(&router);

    
    pthread_mutex_destroy(&sites_db_mtx);
}
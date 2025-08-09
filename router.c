#include <webnet/router.h>
#include <strutils.h>

#define UPD_SECONDS_DELAY 10

pthread_mutex_t sites_db_mtx = PTHREAD_MUTEX_INITIALIZER;

struct site *sites_db = NULL;
size_t sites_num = 0;

void callback(
    struct qbuffer *inp,
    struct qbuffer *out
){
    struct proto_msg inp_msg, out_msg;
    proto_deserial(&inp_msg, inp);

    char **tokens = NULL;
    size_t ntoks = toksplit(inp_msg.path, "/", &tokens);
    if (ntoks == 0){
        // **printf("[warning][callback] ntoks == 0\n");
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
    
    while (router->server.running){
        pthread_mutex_lock(&sites_db_mtx);
        load_sites_db(&sites_db, &sites_num, router->store_dir);
        pthread_mutex_unlock(&sites_db_mtx);

        sleep(UPD_SECONDS_DELAY);
    }

    pthread_exit(NULL);
}

int main(){
    struct router router;
    
    pthread_t upd_thr;
    pthread_create(&upd_thr, NULL, update_db_thread, (void*)&router);

    create_router(
        &router,
        "./assets/sites",
        "127.0.0.1",
        8520,
        20
    );
    
    run_router(&router, callback);

    for (size_t i = 0; i < sites_num; i++){
        // !*printf("[log] site: %s\n", sites_db[i].domain_name);
        destroy_site(&sites_db[i]);
    }
    free(sites_db);
    destroy_router(&router);

    pthread_join(upd_thr, NULL);
    pthread_mutex_destroy(&sites_db_mtx);
}
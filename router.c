#include <webnet/router.h>
#include <strutils.h>

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
        printf("[warning][callback] ntoks == 0\n");
        return;
    }

    const char *domain = tokens[0];
    char *real_path = malloc(strlen(inp_msg.path) - strlen(domain) + 2);
    strcpy(real_path, inp_msg.path + strlen(domain));
    free(inp_msg.path);
    inp_msg.path = real_path;

    printf("\n[log] requested site \"%s\" | path \"%s\"\n", domain, inp_msg.path);    

    struct site *requested_site;
    find_site(sites_db, sites_num, domain, &requested_site);

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

int main(){
    struct router router;

    create_router(
        &router,
        "ghost",
        "./assets/sites",
        "127.0.0.1",
        8520
    );

    load_sites_db(&sites_db, &sites_num, "./assets/sites");
    run_router(&router, callback);

    for (size_t i = 0; i < sites_num; i++){
        // printf("[log] site: %s\n", sites_db[i].domain_name);
        destroy_site(&sites_db[i]);
    }
    free(sites_db);
    destroy_router(&router);
}
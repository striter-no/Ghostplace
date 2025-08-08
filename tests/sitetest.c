#include <webnet/site.h>

int main(){
    // 1. Loading site from files in directory
    struct site site;
    load_site(&site, "./assets/sites", "ghost.main");
    
    // 2. Composing site to protocol messages
    struct proto_msg *msgs = NULL;
    compose_site(&site, &msgs);
    destroy_site(&site);

    // 3. Getting messages total count
    size_t msgs_size = 0, has_gss = 0;
    sscanf(msgs[0].content, "%d %d", &msgs_size, &has_gss);
    
    // 4. Decomposing site from messages
    struct site new_site;
    decompose_site(&new_site, msgs);

    // 5. Saving to directory
    new_site.domain_name[0] = 'I';
    save_site(&new_site, "./assets/sites");
    destroy_site(&new_site);

    for (size_t i = 0; i < msgs_size + 1; i++){
        proto_msg_free(&msgs[i]);
    }
    free(msgs);
}
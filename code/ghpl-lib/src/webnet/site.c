#include <ghpl/webnet/site.h>
#include <ghpl/utils/strutils.h>

void save_site(
    struct site *site,
    const char *dirpath
){
    path_sanitize(dirpath);

    char *fullpath = (char*)malloc(strlen(dirpath) + strlen(site->domain_name) + 100);
    for (size_t i = 0; i < strlen(dirpath) + strlen(site->domain_name) + 100; i++) fullpath[i] = '\0';

    strcat(fullpath, dirpath);
    fullpath[strlen(dirpath)] = '/';
    strcat(fullpath + strlen(dirpath) + 1, site->domain_name);
    
    mkdir_p(fullpath, 0755);
    strcat(fullpath + strlen(dirpath) + 1 + strlen(site->domain_name), "/index.ghml");
    
    writefile(fullpath, "w", site->ghml_content, strlen(site->ghml_content) + 1);
    fullpath[strlen(dirpath) + 1 + strlen(site->domain_name)] = '\0';
    if (site->has_gss){
        strcat(fullpath + strlen(dirpath) + 1 + strlen(site->domain_name), "/styles.gss");
        writefile(fullpath, "w", site->gss_content, strlen(site->gss_content) + 1);
    }
    
    fullpath[strlen(dirpath) + 1 + strlen(site->domain_name)] = '\0';
    strcat(fullpath + strlen(dirpath) + 1 + strlen(site->domain_name), "/assets");
    mkdir(fullpath, 0755);
    for (size_t i = 0; i < site->assets_n; i++){
        struct site_asset *asset = &site->assets[i];

        fullpath[strlen(dirpath) + 1 + strlen(site->domain_name) + strlen("/assets")] = '/';
        fullpath[strlen(dirpath) + 1 + strlen(site->domain_name) + strlen("/assets") + 1] = '\0';
        strcat(fullpath + strlen(dirpath) + 1 + strlen(site->domain_name) + strlen("/assets"), asset->name);
        // !*printf("[log] asset path: %s\n", fullpath);

        writefile(fullpath, "wb", asset->content, asset->cont_len);
    }

    free(fullpath);
}

int load_site(
    struct site *site,
    const char *main_dirpath,
    const char *site_domain
){
    path_sanitize(main_dirpath);
    path_sanitize(site_domain);

    if (!fileexists(main_dirpath)){
        return -1;
    }

    char *fullpath = (char*)malloc(strlen(main_dirpath) + strlen(site_domain) + 100);
    for (size_t i = 0; i < strlen(main_dirpath) + strlen(site_domain) + 100; i++) fullpath[i] = '\0';

    strcat(fullpath, main_dirpath);
    fullpath[strlen(main_dirpath)] = '/';
    strcat(fullpath + strlen(main_dirpath) + 1, site_domain);

    char *ghml_content = NULL;
    char *gss_content = NULL;
    
    strcat(fullpath + strlen(main_dirpath) + 1 + strlen(site_domain), "/index.ghml");
    if (!fileexists(fullpath)){
        free(fullpath);
        return -2;
    }

    size_t size;
    readfile(fullpath, "r", (uint8_t**)&ghml_content, &size);

    fullpath[strlen(main_dirpath) + 1 + strlen(site_domain)] = '\0';
    strcat(fullpath + strlen(main_dirpath) + 1 + strlen(site_domain), "/styles.gss");
    // !*printf("[log] gss path: %s\n", fullpath);
    if (fileexists(fullpath)){
        size_t size;
        readfile(fullpath, "r", (uint8_t**)&gss_content, &size);
        // !*printf("[log] just read gss\n");
    }

    fullpath[strlen(main_dirpath) + 1 + strlen(site_domain)] = '\0';
    strcat(fullpath + strlen(main_dirpath) + 1 + strlen(site_domain), "/assets");
    
    char **file_names = NULL;
    size_t assets_n = enum_files(fullpath, &file_names);
    
    struct site_asset *assets = (struct site_asset*)malloc(sizeof(struct site_asset) * assets_n);
    if (assets == NULL){
        free(fullpath);
        return -3;
    }

    for (size_t i = 0; i < assets_n; i++){
        assets[i].type = IMAGE_CONT;
        assets[i].name = strdup(file_names[i]);

        
        fullpath[strlen(main_dirpath) + 1 + strlen(site_domain) + strlen("/assets")] = '/';
        fullpath[strlen(main_dirpath) + 1 + strlen(site_domain) + strlen("/assets") + 1] = '\0';
        strcat(fullpath + strlen(main_dirpath) + 1 + strlen(site_domain) + strlen("/assets"), file_names[i]);
        // !*printf("[log] asset path: %s\n", fullpath);
        readfile(fullpath, "rb", &assets[i].content, &assets[i].cont_len);
        fullpath[strlen(main_dirpath) + 1 + strlen(site_domain) + strlen("/assets")] = '\0';
    }
    free_list_cstr(file_names, assets_n);

    site->domain_name = strdup(site_domain);
    site->ghml_content = ghml_content;
    site->gss_content = gss_content;
    site->has_gss = gss_content != NULL;
    site->assets = assets;
    site->assets_n = assets_n;
    free(fullpath);

    return 0;
}

void update_site_db(
    const char *main_dirpath,
    struct site *sites,
    size_t *sites_num
){
    char **dirs = NULL; 
    size_t dirs_n = enum_directories(main_dirpath, &dirs);
    if (sites){
        free(sites);
        sites = NULL;
    }
    
    sites = (struct site*)malloc(sizeof(struct site) * dirs_n);
    for (size_t i = 0; i < dirs_n; i++){
        load_site(&sites[i], main_dirpath, dirs[i]);
    }

    free_list_cstr(dirs, dirs_n);
}

void find_site(
    struct site *sites,
    size_t sites_num,

    const char *domain_name,
    struct site **out
){
    for (size_t i = 0; i < sites_num; i++){
        if (strcmp(sites[i].domain_name, domain_name) == 0){
            *out = &sites[i];
            return;
        }
    }

    *out = NULL;
}

int decompose_site(
    struct site *site,
    struct proto_msg *msgs,
    size_t msgs_size
){
    char was_ghml = 0;
    char was_gss = 0;
    int  num_assets = 0;
    struct site_asset *assets = NULL;

    for (size_t i = 0; i < msgs_size; i++){
        struct proto_msg *msg = &msgs[i];
        char *path = NULL;
        
        if (msg->path[0] == '/'){
            path = malloc(strlen(msg->path));
            strcpy(path, msg->path + 1);

            free(msg->path);
            msg->path = path;
        }

        // **printf("[log][decompose] path: \"%s\"\n", msg->path);

        if (strcmp(msg->path, "index.ghml") == 0){
            site->ghml_content = strdup(msg->content);
            was_ghml = 1;
            continue;
        }

        if (strcmp(msg->path, "styles.gss") == 0){
            site->gss_content = strdup(msg->content);
            was_gss = 1;
            continue;
        }

        if (str_startsw(msg->path, "assets/")){
            assets = (struct site_asset *)realloc(assets, sizeof(struct site_asset) * (++num_assets));
            struct site_asset *asset = &assets[num_assets - 1];
            
            asset->cont_len = msg->cont_size;
            asset->type = msg->conttype;
            asset->name = basepath(msg->path);
            asset->content = (uint8_t *)malloc(msg->cont_size);
            memcpy(asset->content, msg->content, msg->cont_size);
        }
    }

    if (!was_ghml){
        if (assets) 
            free(assets);
        return -1;
    }

    site->assets = assets;
    site->assets_n = num_assets;
    site->has_gss = was_gss;
    site->domain_name = strdup("undef");

    return 0;
}

void compose_enum(
    struct site *site,
    struct proto_msg *msg
){
    msg->type = GET;
    msg->conttype = TEXT_CONT;
    msg->path = strdup("/");
    msg->proto_ver = 0;
    msg->content = strdup("");
    msg->cont_size = 1;
    
    smart_strcat((char**)&msg->content, "index.ghml\n");
    if (site->has_gss)
        smart_strcat((char**)&msg->content, "styles.gss\n");
    
    for (size_t i = 0; i < site->assets_n; i++){
        char lbuff[200] = "assets/";
        strcat(lbuff, site->assets[i].name);
        size_t old_size = strlen(lbuff);
        lbuff[old_size] = '\n';
        lbuff[old_size + 1] = '\0';
        smart_strcat((char**)&msg->content, lbuff);
    }

    msg->cont_size = strlen(msg->content) + 1;
}

int get_get_messages(
    const struct proto_msg *a_enum_msg,
    struct proto_msg **messages,
    size_t *messages_n
){
    char **tokens = NULL;
    size_t ntoks = toksplit(a_enum_msg->content, "\n", &tokens);

    if (ntoks == 0) {
        free_list_cstr(tokens, ntoks);
        return -1;
    }

    (*messages) = (struct proto_msg*)calloc(ntoks, sizeof(struct proto_msg));
    for (size_t i = 0; i < ntoks; i++){
        struct proto_msg *msg = &(*messages)[i];
        msg->type = GET;
        msg->conttype = TEXT_CONT;
        msg->proto_ver = 0;
        msg->content = strdup("");
        msg->cont_size = 1;

        msg->path = strdup(tokens[i]);
    }

    *messages_n = ntoks;
    free_list_cstr(tokens, ntoks);
    return 0;
}

int compose_by_path(
    struct site *site,
    const char *path,

    struct proto_msg *msg
){
    char **tokens = NULL;
    size_t ntoks = toksplit(path, "/", &tokens);
    
    if (ntoks == 0){
        return -1;
    }

    msg->type = GET;
    msg->conttype = TEXT_CONT;
    msg->path = strdup(path);
    msg->proto_ver = 0;
    msg->content = NULL;
    msg->cont_size = 1;

    if (strcmp(tokens[0], "index.ghml") == 0){
        set_proto_content(msg, site->ghml_content, strlen(site->ghml_content) + 1);
        free_list_cstr(tokens, ntoks);
        return 0;
    }

    if (strcmp(tokens[0], "styles.gss") == 0){
        if (!site->has_gss) {
            free_list_cstr(tokens, ntoks);
            return -3;
        }
        
        set_proto_content(msg, site->gss_content, strlen(site->gss_content) + 1);
        free_list_cstr(tokens, ntoks);
        return 0;
    }

    if (strcmp(tokens[0], "assets") == 0){
        if (site->assets_n == 0){
            free_list_cstr(tokens, ntoks);
            return -4;
        }
        
        for (size_t i = 0; i < site->assets_n; i++){
            if (strcmp(site->assets[i].name, tokens[1]) == 0){
                struct site_asset *asset = &site->assets[i];
                set_proto_content(msg, asset->content, asset->cont_len);
                
                free_list_cstr(tokens, ntoks);
                return 0;
            }
        }
        
        free_list_cstr(tokens, ntoks);
        return -5;
    }

    free_list_cstr(tokens, ntoks);
    return -2;
}

void set_proto_content(
    struct proto_msg *msg,
    ubyte *buffer,
    size_t size
){
    msg->content = (ubyte*)malloc(size);
    msg->cont_size = size;
    memcpy(msg->content, buffer, size);
}

void destroy_site(
    struct site *site
){

    for (size_t i = 0; i < site->assets_n; i++)
        destroy_asset(&site->assets[i]);
    free(site->assets);

    free(site->gss_content);
    free(site->ghml_content);
    free(site->domain_name);
    site->ghml_content = NULL;
    site->gss_content = NULL;
    site->domain_name = NULL;
}

void destroy_asset(
    struct site_asset *asset
){
    free(asset->name);
    free(asset->content);
    asset->name = 0;
    asset->content = 0;
}
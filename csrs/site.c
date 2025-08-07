#include <webnet/site.h>

void load_asset(
    struct site_asset *asset,
    const char *full_path
){
    
}

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
        printf("[log] asset path: %s\n", fullpath);

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
    printf("[log] gss path: %s\n", fullpath);
    if (fileexists(fullpath)){
        size_t size;
        readfile(fullpath, "r", (uint8_t**)&gss_content, &size);
        printf("[log] just read gss\n");
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
        printf("[log] asset path: %s\n", fullpath);
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

/*
GET text 0
domain ...
4

GET text 0
domain ...
{index.ghml}

GET text 0
domain ...
{styles.gss}

GET img 0
domain ...
{asset1.png}

GET img 0
domain ...
{asset2.png}
*/
int compose_site(
    struct site *site,
    struct proto_msg **msgs
){
    size_t msgs_num = 2 + site->has_gss + site->assets_n;
    
    size_t counter = 0;
    (*msgs) = (struct proto_msg*)calloc(msgs_num, sizeof(struct proto_msg));
    (*msgs)[counter++] = (struct proto_msg){
        .type = GET,
        .conttype = TEXT_CONT,
        .path = strdup(site->domain_name),
        .proto_ver = 0,
        .content = NULL,
        .cont_size = 0
    };
    char files_buff[20];
    sprintf(files_buff, "%d %d", msgs_num - 1, site->has_gss);
    set_proto_content(&(*msgs)[counter - 1], (uint8_t*)files_buff, strlen(files_buff) + 1);

    (*msgs)[counter++] = (struct proto_msg){
        .type = GET,
        .conttype = TEXT_CONT,
        .path = strdup(site->domain_name),
        .proto_ver = 0,
        .content = NULL,
        .cont_size = 0
    };
    set_proto_content(&(*msgs)[counter - 1], site->ghml_content, strlen(site->ghml_content) + 1);

    if (site->has_gss){
        (*msgs)[counter++] = (struct proto_msg){
            .type = GET,
            .conttype = TEXT_CONT,
            .path = strdup(site->domain_name),
            .proto_ver = 0,
            .content = NULL,
            .cont_size = 0
        };
        set_proto_content(&(*msgs)[counter - 1], site->gss_content, strlen(site->gss_content) + 1);
    }

    for (size_t i = 0; i < site->assets_n; i++){
        struct site_asset *asset = &site->assets[i];
        if (asset->type == IMAGE_CONT){

            char *local_domain = strdup(site->domain_name);
            if (local_domain[strlen(local_domain)-1] != '/') {
                // Добавляем слэш к домену один раз при инициализации
                char *temp = malloc(strlen(local_domain) + 2);
                if (temp) {
                    strcpy(temp, local_domain);
                    strcat(temp, "/");
                    free(local_domain);
                    local_domain = temp;
                }
            }

            char *asset_path = (char*)calloc(strlen(local_domain) + strlen(asset->name) + 2, sizeof(char)); // +2 for '/' and '\0'
            strcat(asset_path, local_domain); // need to be ended with '/'
            strcat(asset_path, asset->name);
            asset_path[strlen(local_domain) + strlen(asset->name)] = '/';
            asset_path[strlen(local_domain) + strlen(asset->name)+ 1] = '\0';
            
            free(local_domain);
            (*msgs)[counter++] = (struct proto_msg){
                .type = GET,
                .conttype = IMAGE_CONT,
                .path = asset_path,
                .proto_ver = 0,
                .content = NULL,
                .cont_size = 0
            };
        } else {
            for (size_t j = 0; j < counter; j++) {
                proto_msg_free(&(*msgs)[j]);
            }
            free(*msgs);
            *msgs = NULL;

            fprintf(stderr, "[site][compose][error] now I cannot process this type of asset\n");
            return -1;
        }
        set_proto_content(&(*msgs)[counter - 1], asset->content, asset->cont_len);
    }

    return 0;
}

static void extract_parts_strtok(const char *asset_path, char **domain, char **asset_name) {
    
    char *path_copy = strdup(asset_path);
    if (!path_copy) {
        *domain = *asset_name = NULL;
        return;
    }
    
    char *saveptr;
    char *token;
    char *last_token = NULL;
    
    token = strtok_r(path_copy, "/", &saveptr);
    while (token) {
        last_token = token;
        token = strtok_r(NULL, "/", &saveptr);
    }
    
    *asset_name = last_token ? strdup(last_token) : NULL;
    
    const char *last_slash = strrchr(asset_path, '/');
    if (last_slash && *asset_name) {
        size_t domain_len = last_slash - asset_path;
        *domain = malloc(domain_len + 1);
        if (*domain) {
            strncpy(*domain, asset_path, domain_len);
            (*domain)[domain_len] = '\0';
        }
    } else {
        *domain = NULL;
    }
    
    free(path_copy);
}

int decompose_site(
    struct site *site,
    struct proto_msg *msgs
){
    *site = (struct site){0};
    
    site->domain_name = strdup(msgs[0].path);
    
    size_t msgs_size = 0, has_gss = 0;
    sscanf(msgs[0].content, "%d %d", &msgs_size, &has_gss); // 316

    site->ghml_content = (char*)malloc(msgs[1].cont_size);
    memcpy(site->ghml_content, msgs[1].content, msgs[1].cont_size);

    site->has_gss = has_gss;
    if (has_gss){
        site->gss_content = (char*)malloc(msgs[2].cont_size);
        memcpy(site->gss_content, msgs[2].content, msgs[2].cont_size);
    }

    // assets
    site->assets_n = msgs_size - (1 + has_gss);
    site->assets = (struct site_asset*)malloc(site->assets_n * sizeof(struct site_asset));
    for (size_t i = 0; i < site->assets_n; i++){
        struct site_asset *asset = &site->assets[i];
        struct proto_msg *msg = &msgs[2 + has_gss + i];

        asset->cont_len = msg->cont_size;
        asset->content = (uint8_t*)malloc(msg->cont_size);
        memcpy(asset->content, msg->content, asset->cont_len);

        char *domain = NULL;
        char *asset_name = NULL;
        
        extract_parts_strtok(msg->path, &domain, &asset_name);
        asset->name = asset_name;
        asset->type = msg->conttype;

        free(domain);
    }
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
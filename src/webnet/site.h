#pragma once
#include "proto.h"
#include <files.h>

struct site_asset {
    enum PROTO_CONT_TYPE type;
    char *name;
    ubyte *content;
    size_t cont_len;
};

struct site {
    char *domain_name;
    
    char *ghml_content;
    char *gss_content;
    byte has_gss;

    struct site_asset *assets;
    size_t assets_n;
};

void save_site(
    struct site *site,
    const char *dirpath
);

int load_site(
    struct site *site,
    const char *main_dirpath,
    const char *site_dirname
);

void update_site_db(
    const char *main_dirpath,
    struct site *sites,
    size_t *sites_num
);

void find_site(
    struct site *sites,
    size_t sites_num,

    const char *domain_name,
    struct site **out
);

void compose_enum(
    struct site *site,
    struct proto_msg *msg
);

int decompose_site(
    struct site *site,
    struct proto_msg *msgs,
    size_t msgs_size
);

int get_get_messages(
    const struct proto_msg *a_enum_msg,
    struct proto_msg **messages,
    size_t *messages_n
);

int compose_by_path(
    struct site *site,
    const char *path,

    struct proto_msg *msg
);

void set_proto_content(
    struct proto_msg *msg,
    ubyte *buffer,
    size_t size
);

void destroy_site(
    struct site *site
);

void destroy_asset(
    struct site_asset *asset
);
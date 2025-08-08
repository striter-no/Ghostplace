#pragma once
#include <webnet/net/tcpcli.h>
#include <webnet/site.h>

void request_site(
    struct site *out,
    struct TCP_client *cli,
    const char *main_path
);
#pragma once
#include <ghpl/webnet/net/tcpcli.h>
#include <ghpl/webnet/site.h>

void request_site(
    struct site *out,
    struct TCP_client *cli,
    const char *main_path
);
#pragma once
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <poll.h>
#include <errno.h>
#include <queue.h>
#include <unistd.h> // read(), write(), close()
#define TCP_MAX_BUFFER 4096 

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

struct TCP_client {
    int sockfd;
    char running;
    struct sockaddr_in servaddr;

    struct queue input_queue;
    struct queue output_queue;
};

void tcp_cli_create(struct TCP_client *cli);
void tcp_cli_conect(struct TCP_client *cli, const char *ip, int port);
void tcp_cli_disconn(struct TCP_client *cli);
void tcp_cli_run(struct TCP_client *cli);
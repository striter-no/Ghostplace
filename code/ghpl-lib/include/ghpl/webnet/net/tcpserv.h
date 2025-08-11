#pragma once
#include <ghpl/webnet/net/tcputils.h>
#include <ghpl/utils/queue.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h> // read(), write(), close()
#define TCP_MAX_BUFFER 1024

#ifndef min
#define min(a, b) ((a) > (b) ? (b) : (a))
#endif

#define MAX_EXPECTING_DATA 1024 * 1024 * 5
#define TOTAL_TIMEOUT_MS 3 * 1000

struct __TCP_serv_cli {
    pthread_t sthread;
    int connfd;
    struct queue input_queue;
    struct queue output_queue;

    struct sockaddr_in sock;
    char *ip;
    int port;
};

struct TCP_server {
    struct sockaddr_in servaddr;
    int sockfd;

    size_t cli_max;
    ssize_t act_clients_n;
    ssize_t allocated_clients;
    char first_client;

    int running;
    pthread_mutex_t climtx;
    struct __TCP_serv_cli *clients;
};

void *__TCP_serv_cli_thread(void *args);

void tcp_create(
    struct TCP_server *serv,
    const char *ip,
    int port
);

void tcp_listen(struct TCP_server *serv, int cli_max);
void tcp_loop(struct TCP_server *serv);
void tcp_end_server(struct TCP_server *serv);
void tcp_end_client(struct __TCP_serv_cli *cli);
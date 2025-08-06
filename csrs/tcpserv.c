#include <webnet/tcpserv.h>

struct __thread_args {
    struct __TCP_serv_cli *cli;
    ssize_t *act_clients;
    int *running;
};

void *__TCP_serv_cli_thread(void *voidargs){
    struct __thread_args *args = voidargs;
    struct __TCP_serv_cli *cli = args->cli;
    
    char local_running = 1;
    while (*args->running && local_running){
        struct pollfd pfd = {
            .fd = cli->connfd,
            .events = POLLIN
        };
        
        int ret = poll(&pfd, 1, 0);
        
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            fprintf(stderr, "[error] poll failed: %s\n", strerror(errno));
            break;
        }
        
        if (ret == 0) {
            goto answer;
            continue;
        }
        
        // Проверяем тип события
        if (pfd.revents & (POLLHUP | POLLERR)) {
            printf("[log] client disconnected (POLLHUP/POLLERR)\n");
            break;
        }
        
        if (!(pfd.revents & POLLIN)) {
            goto answer;
            continue;
        }

        uint8_t *buffer = NULL; size_t buff_size = 0;
        while (1){
            uint8_t inp_buffer[TCP_MAX_BUFFER] = {0};
            ssize_t got_bytes = 0;

            bzero(inp_buffer, TCP_MAX_BUFFER);

            printf("[log] reading...\n");
            got_bytes = read(cli->connfd, inp_buffer, sizeof(inp_buffer));
            if (got_bytes == 0){
                printf("[log] client disconnected\n");
                local_running = 0;
                goto __thr_exit;
                break;
            }

            if (got_bytes < 0){
                fprintf(stderr, "[warning][read] error while reading from client socket (%d bytes)\n", got_bytes);
                fprintf(stderr, "[log][__TCP_serv_cli_thread] breaking from loop\n");
                local_running = 0;
                goto __thr_exit;
                break;
            }

            uint8_t *localb = realloc(buffer, buff_size + got_bytes);
            if (localb == NULL){
                fprintf(stderr, "[non-crit-err][read] error while reallocating buffer in client socket (%d bytes, %d total)\n", got_bytes, buff_size + got_bytes);
                free(buffer);
                goto answer;
            }
            buffer = localb;
            memcpy(buffer + buff_size, inp_buffer, got_bytes);

            buff_size += got_bytes;
            if (got_bytes <= TCP_MAX_BUFFER)
                break;
        }

        // inp_buffer[got_bytes] = '\0';
        // printf("[log] got this bytes: %s\n", inp_buffer);

        struct qbuffer ibuf;
        create_qbuffer(&ibuf, buff_size + 1);
        memcpy(ibuf.bytes, buffer, buff_size);
        ibuf.bytes[buff_size] = '\0';
        free(buffer);

        push_buffer(&cli->input_queue, &ibuf);
        clear_qbuffer(&ibuf);

        answer:{
            struct qbuffer obuf;
            int has_data = pop_buffer(&cli->output_queue, &obuf);
            if (has_data != 0)
                continue;
            
            size_t total_written = 0;
            while (1){
                size_t to_write = min(obuf.size - total_written, TCP_MAX_BUFFER);
                ssize_t written = write(cli->connfd, obuf.bytes + total_written, to_write);
                if (written < 0){
                    fprintf(stderr, "[warning][read] error while writing to socket (%d bytes)\n", obuf.size);
                } else {
                    total_written += to_write;
                    if (to_write < TCP_MAX_BUFFER) 
                        break;
                }
            }
            clear_qbuffer(&obuf);
        }

        __thr_exit:{;}
    }

    __sync_fetch_and_sub(args->act_clients, 1);
    printf("[log] active clients: %d\n", *args->act_clients);
    
    free(args);
    pthread_exit(NULL);
}

void tcp_create(
    struct TCP_server *serv,
    const char *ip,
    int port
){
    serv->sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (serv->sockfd == -1) { 
        fprintf(stderr, "[tcp_serv][error] socket creation failed\n"); 
        exit(-1); 
    }

    bzero(&serv->servaddr, sizeof(serv->servaddr)); 

    serv->servaddr.sin_family = AF_INET; 
    serv->servaddr.sin_addr.s_addr = inet_addr(ip); 
    serv->servaddr.sin_port = htons(port); 

    if ((bind(serv->sockfd, (struct sockaddr*)&serv->servaddr, sizeof(serv->servaddr))) != 0) { 
        fprintf(stderr, "[tcp_serv][error] socket bind failed\n"); 
        exit(-2);
    }
    
    serv->act_clients_n = 0;
    serv->allocated_clients = 0;
    serv->clients = NULL;
    serv->running = 0;
    pthread_mutex_init(&serv->climtx, NULL);
}

void tcp_listen(struct TCP_server *serv, int cli_max){
    serv->cli_max = cli_max;
    if (listen(serv->sockfd, cli_max) != 0){
        fprintf(stderr, "[tcp_serv][error] listening failed\n"); 
        exit(-3);
    }
}

void tcp_loop(struct TCP_server *serv){
    serv->first_client = 1;
    __sync_fetch_and_add(&serv->running, 1);
    while ((serv->act_clients_n > 0 || serv->first_client) && serv->running) {
        struct pollfd pfd = {
            .fd = serv->sockfd,
            .events = POLLIN
        };

        int ret = poll(&pfd, 1, 100);
        if (ret == -1) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[error][tcp_server][loop] poll() failed\n");
            break;
        } else if (ret == 0) continue;

        if (!(pfd.revents & POLLIN)) continue;
        
        struct __TCP_serv_cli newcli;
        unsigned int len = sizeof(newcli.sock);
        newcli.connfd = accept(serv->sockfd, (struct sockaddr*)&newcli.sock, &len);
        printf("[log] accepted new client\n");

        if (newcli.connfd < 0){
            fprintf(stderr, "[warning][tcp_serv][loop] acception failed\n");
            continue;
        }

        struct __TCP_serv_cli *buff = realloc(
            serv->clients, (serv->act_clients_n + 1) * sizeof(struct __TCP_serv_cli)
        );

        if (buff == NULL){
            close(newcli.connfd);
            fprintf(stderr, "[tcp_serv][loop][failure] realloc failed\n");
            continue;
        }

        serv->clients = buff;
        struct __TCP_serv_cli *ptrcl = &serv->clients[serv->act_clients_n];
        *ptrcl = (struct __TCP_serv_cli){
            .connfd = newcli.connfd,
            .ip = strdup(inet_ntoa(newcli.sock.sin_addr)),
            .port = ntohs(newcli.sock.sin_port),
            .input_queue = create_queue(),
            .output_queue = create_queue()
        };

        // КРИТИЧЕСКОЕ ИСПРАВЛЕНИЕ: выделяем память для аргументов
        struct __thread_args *targs = malloc(sizeof(struct __thread_args));
        if (!targs) {
            close(newcli.connfd);
            tcp_end_client(ptrcl);
            continue;
        }
        
        targs->act_clients = &serv->act_clients_n;
        targs->cli = ptrcl;
        targs->running = &serv->running;

        pthread_create(&ptrcl->sthread, NULL, __TCP_serv_cli_thread, targs);
        pthread_detach(ptrcl->sthread);

        serv->first_client = 0;
        __sync_fetch_and_add(&serv->act_clients_n, 1);
        __sync_fetch_and_add(&serv->allocated_clients, 1);
    }
    __sync_fetch_and_sub(&serv->running, 1);
}
void tcp_end_server(struct TCP_server *serv){
    
    shutdown(serv->sockfd, SHUT_RDWR);
    
    while (serv->act_clients_n > 0) {
        usleep(10000); // 10ms
    }
    
    for (size_t i = 0; i < serv->allocated_clients; i++){
        tcp_end_client(&serv->clients[i]);
    }
    
    if (serv->clients){
        free(serv->clients);
        serv->clients = NULL;
        serv->allocated_clients = 0;
    }
    
    pthread_mutex_destroy(&serv->climtx);
    close(serv->sockfd);
}

void tcp_end_client(struct __TCP_serv_cli *cli){
    close(cli->connfd);

    free(cli->ip);
    clear_queue(&cli->input_queue);
    clear_queue(&cli->output_queue);
}
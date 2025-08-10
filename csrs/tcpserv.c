#include <webnet/net/tcpserv.h>

struct __thread_args {
    struct __TCP_serv_cli *cli;
    ssize_t *act_clients;
    int *running;
};

static int __read_polling(int sockfd){
    struct pollfd pfd = {
        .fd = sockfd,
        .events = POLLIN
    };
    
    int ret = poll(&pfd, 1, 0);
    
    if (ret == -1) {
        if (errno == EINTR) {
            return 2;
        }
        fprintf(stderr, "[error] poll failed: %s\n", strerror(errno));
        return -1;
    }
    
    if (ret == 0) {
        return 2;
    }
    
    // Проверяем тип события
    if (pfd.revents & (POLLHUP | POLLERR)) {
        // **printf("[log] client down (POLLHUP/POLLERR)\n");
        return -2;
    }
    
    if (!(pfd.revents & POLLIN)) {
        return 2;
    }

    return 1;
}

void *__TCP_serv_cli_thread(void *voidargs){
    struct __thread_args *args = voidargs;
    struct __TCP_serv_cli *cli = args->cli;
    
    char local_running = 1;
    while (*args->running && local_running){

        int pollstatus = __read_polling(cli->connfd);
        if (pollstatus != 1) {
            if (pollstatus < 0) break;
            if (pollstatus == 2) goto answer;
            continue;
        }

        uint32_t net_size = 0;
        ssize_t header_read = full_read(cli->connfd, &net_size, sizeof(net_size));

        if (header_read == 0) {
            printf("[log] client disconnected\n");
            local_running = 0;
            goto __thr_exit;
        }
        
        if (header_read < 0 || header_read < sizeof(net_size)) {
            fprintf(stderr, "[error] failed to read full header: %s\n", strerror(errno));
            local_running = 0;
            goto __thr_exit;
        }

        uint32_t need_to_read = ntohl(net_size);
        printf("[log] expecting %u bytes of data\n", need_to_read);

        uint8_t *buffer = NULL; size_t buff_size = 0;
        while (1){
            uint8_t inp_buffer[TCP_MAX_BUFFER] = {0};
            ssize_t got_bytes = 0;

            bzero(inp_buffer, TCP_MAX_BUFFER);

            // **printf("[log] reading...\n");
            got_bytes = read(cli->connfd, inp_buffer, sizeof(inp_buffer));
            if (got_bytes == 0){
                // **printf("[log] client disconnected\n");
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
            
            if (buff_size >= need_to_read){
                break;
            }
        }

        // inp_buffer[got_bytes] = '\0';
        printf("[log] got %d out of %d bytes\n", buff_size, need_to_read);

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
            
            uint32_t net_size = htonl(obuf.size);
            if (full_write(cli->connfd, &net_size, sizeof(net_size)) < 0) {
                fprintf(stderr, "[error] failed to send header: %s\n", strerror(errno));
                clear_qbuffer(&obuf);
                continue;
            }
            
            // Отправляем данные
            if (full_write(cli->connfd, obuf.bytes, obuf.size) < 0) {
                fprintf(stderr, "[error] failed to send data: %s\n", strerror(errno));
            }

            clear_qbuffer(&obuf);
        }

        __thr_exit:{;}
    }

    __sync_fetch_and_sub(args->act_clients, 1);
    // **printf("[log] active clients: %d\n", *args->act_clients);
    
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
    
    int opt = 1;
    if (setsockopt(serv->sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        fprintf(stderr, "[tcp_serv][error] setsockopt failed\n"); 
        exit(-3); 
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
    while (serv->running) { // (serv->act_clients_n > 0 || serv->first_client) && 
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
        // **printf("[log] accepted new client\n");

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
    __sync_fetch_and_sub(&serv->running, 1);
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
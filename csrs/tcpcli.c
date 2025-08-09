#include <webnet/net/tcpcli.h>

void tcp_cli_create(struct TCP_client *cli){
    cli->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cli->sockfd == -1) {
        fprintf(stderr, "[tcp_client][error] socket creation failed\n");
        exit(-1);
    }
    bzero(&cli->servaddr, sizeof(cli->servaddr));

    cli->input_queue = create_queue();
    cli->output_queue = create_queue();
    cli->running = 0;
}

void tcp_cli_conect(struct TCP_client *cli, const char *ip, int port){
    cli->servaddr.sin_family = AF_INET;
    cli->servaddr.sin_addr.s_addr = inet_addr(ip);
    cli->servaddr.sin_port = htons(port);

    if (connect(cli->sockfd, (struct sockaddr*)&cli->servaddr, sizeof(cli->servaddr)) != 0) {
        fprintf(stderr, "[tcp_client][error] connection with the server failed\n");
        exit(-2);
    }
}

void tcp_cli_disconn(struct TCP_client *cli){
    __sync_fetch_and_sub(&cli->running, 1);

    usleep(10000);
    close(cli->sockfd);
    clear_queue(&cli->input_queue);
    clear_queue(&cli->output_queue);
}

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
        // **printf("[log] server down (POLLHUP/POLLERR)\n");
        return -2;
    }
    
    if (!(pfd.revents & POLLIN)) {
        return 2;
    }

    return 1;
}

void tcp_cli_run(struct TCP_client *cli){
    cli->running = 1;
    char local_running = 1;
    while (cli->running && local_running){

        int pollstatus = __read_polling(cli->sockfd);
        if (pollstatus != 1) {
            // Обработка ошибок и отключения
            if (pollstatus < 0) break;
            if (pollstatus == 2) goto answer;
            continue;
        }

        uint8_t *buffer = NULL; size_t buff_size = 0;
        while (1){
            uint8_t inp_buffer[TCP_MAX_BUFFER] = {0};
            ssize_t got_bytes = 0;

            bzero(inp_buffer, TCP_MAX_BUFFER);

            // **printf("[log] reading...\n");
            got_bytes = read(cli->sockfd, inp_buffer, sizeof(inp_buffer));
            if (got_bytes == 0){
                // **printf("[log] server disconnected\n");
                local_running = 0;
                goto __thr_exit;
                break;
            }

            if (got_bytes < 0){
                fprintf(stderr, "[warning][read] error while reading from server socket (%d bytes)\n", got_bytes);
                fprintf(stderr, "[log][__TCP_serv_cli_thread] breaking from loop\n");
                local_running = 0;
                goto __thr_exit;
                break;
            }

            uint8_t *localb = realloc(buffer, buff_size + got_bytes);
            if (localb == NULL){
                fprintf(stderr, "[non-crit-err][read] error while reallocating buffer in server socket (%d bytes, %d total)\n", got_bytes, buff_size + got_bytes);
                free(buffer);
                goto answer;
            }
            buffer = localb;
            memcpy(buffer + buff_size, inp_buffer, got_bytes);

            buff_size += got_bytes;
            if (got_bytes < TCP_MAX_BUFFER){
                break;
            }
        }

        // inp_buffer[got_bytes] = '\0';
        // **printf("[log] got %d bytes\n", buff_size);

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
                ssize_t written = write(cli->sockfd, obuf.bytes + total_written, to_write);
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

        __thr_exit: {}
    }

    cli->running = 0;
}

void await_pop(struct queue *q, struct qbuffer *buff){
    while (1){
        int has_inp = pop_buffer(q, buff);
        if (has_inp != 0) continue;
        break;
    }
}
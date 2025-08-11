#include <webnet/net/tcpserv.h>

void *detached(void *args){
    struct TCP_server *serv = args;
    tcp_loop(serv);
    pthread_exit(NULL);
}

int main(){
    struct TCP_server serv;
    tcp_create(&serv, "127.0.0.1", 9002);
    tcp_listen(&serv, 1);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&serv);
    pthread_detach(main_thread);

    sleep(2);
    while (serv.running){
        pthread_mutex_lock(&serv.climtx);

        for (size_t i = 0; i < serv.act_clients_n; i++){
            struct __TCP_serv_cli *cli = &serv.clients[i];
            struct qbuffer ibuff, obuff;

            int has_inp = pop_buffer(&cli->input_queue, &ibuff);
            if (has_inp != 0) continue;

            // **printf("[out] buffer: %s\n", ibuff.bytes);
            copy_qbuffer(&obuff, &ibuff);
            push_buffer(&cli->output_queue, &obuff);

            clear_qbuffer(&ibuff);
            clear_qbuffer(&obuff);
        }

        pthread_mutex_unlock(&serv.climtx);
    }

    tcp_end_server(&serv);
}
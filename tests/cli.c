#include <webnet/net/tcpcli.h>

void *detached(void *args){
    struct TCP_client *cli = args;
    tcp_cli_run(cli);
    pthread_exit(NULL);
}

int main(){
    struct TCP_client cli;
    tcp_cli_create(&cli);
    tcp_cli_conect(&cli, "127.0.0.1", 9002);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&cli);
    pthread_detach(main_thread);

    struct qbuffer msg;
    const char buff[] = "Hello world!";
    create_qbuffer(&msg, strlen(buff) + 1);
    memcpy(msg.bytes, buff, strlen(buff) + 1);

    push_buffer(&cli.output_queue, &msg);
    clear_qbuffer(&msg);

    sleep(2);
    while (cli.running){
        struct qbuffer ibuff, obuff;

        int has_inp = pop_buffer(&cli.input_queue, &ibuff);
        if (has_inp != 0) continue;

        printf("[out] buffer: %s\n", ibuff.bytes);
        copy_qbuffer(&obuff, &ibuff);
        push_buffer(&cli.output_queue, &obuff);

        clear_qbuffer(&ibuff);
        clear_qbuffer(&obuff);

    }

    tcp_cli_disconn(&cli);    
}
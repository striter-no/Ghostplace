#include <webnet/net/tcpcli.h>
#include <webnet/site.h>

void *detached(void *args){
    struct TCP_client *cli = args;
    tcp_cli_run(cli);
    pthread_exit(NULL);
}

int main(){
    struct TCP_client cli;
    tcp_cli_create(&cli);
    tcp_cli_conect(&cli, "127.0.0.1", 8520);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&cli);

    struct qbuffer qmsg;
    struct proto_msg enum_ask = {
        .type = GET,
        .conttype = TEXT_CONT,
        .path = strdup("/"),
        .content = strdup(""),
        .cont_size = 1,
        .proto_ver = 0
    };

    proto_serial(&enum_ask, &qmsg);
    proto_msg_free(&enum_ask);

    push_buffer(&cli.output_queue, &qmsg);
    

    struct proto_msg *msgs = NULL;
    size_t got_msgs = 0;

    sleep(2);
    if (cli.running){
        printf("[log] awaiting for messages\n");
        struct qbuffer ibuff;
        await_pop(&cli.input_queue, &ibuff);

        struct proto_msg enum_msg;
        proto_deserial(&enum_msg, &ibuff);
        clear_qbuffer(&ibuff);
        proto_print(&enum_msg);

        struct proto_msg *get_msgs = NULL;
        size_t msgs_size = 0;
        int out = get_get_messages(&enum_msg, &get_msgs, &msgs_size);
        proto_msg_free(&enum_msg);

        printf("[log]{%d} requesting %d messages\n", out, msgs_size);
        for (size_t i = 0; i < msgs_size; i++){
            struct qbuffer obuff;
            proto_serial(&get_msgs[i], &obuff);
            push_buffer(&cli.output_queue, &obuff);
            clear_qbuffer(&obuff);

            struct qbuffer ibuff;
            await_pop(&cli.input_queue, &ibuff);
            printf("[log] got %d message\n", i + 1);
            
            msgs = (struct proto_msg*)realloc(msgs, sizeof(struct proto_msg) * (++got_msgs));
            proto_deserial(&msgs[i], &ibuff);
            proto_print(&msgs[i]);
            clear_qbuffer(&ibuff);
        }

        for (size_t i = 0; i < msgs_size; i++)
            proto_msg_free(&get_msgs[i]);
        free(get_msgs);
    }
    tcp_cli_disconn(&cli);

    printf("[log] all got\n");

    pthread_join(main_thread, NULL);
    clear_qbuffer(&qmsg);
    for (size_t i = 0; i < got_msgs; i++){
        proto_msg_free(&msgs[i]);
    }
    free(msgs);    
}
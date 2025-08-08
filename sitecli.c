#include <webnet/net/tcpcli.h>
#include <webnet/site.h>

void *detached(void *args){
    struct TCP_client *cli = args;
    tcp_cli_run(cli);
    pthread_exit(NULL);
}

void await_pop(struct queue *q, struct qbuffer *buff){
    while (1){
        int has_inp = pop_buffer(q, buff);
        if (has_inp != 0) continue;
        break;
    }
}

int main(){
    struct TCP_client cli;
    tcp_cli_create(&cli);
    tcp_cli_conect(&cli, "127.0.0.1", 8520);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&cli);

    struct qbuffer qmsg;
    struct proto_msg pmsg = {
        .type = GET,
        .conttype = TEXT_CONT,
        .path = strdup("/"),
        .content = strdup(""),
        .cont_size = 1,
        .proto_ver = 0
    };

    proto_serial(&pmsg, &qmsg);
    proto_msg_free(&pmsg);

    push_buffer(&cli.output_queue, &qmsg);
    

    struct proto_msg *msgs = NULL;
    size_t got_msgs = 0;

    sleep(2);
    if (cli.running){
        printf("[log] awaiting for messages\n");
        struct qbuffer ibuff;
        await_pop(&cli.input_queue, &ibuff);
        msgs = (struct proto_msg*)realloc(msgs, sizeof(struct proto_msg) * (++got_msgs));
        proto_deserial(&msgs[got_msgs - 1], &ibuff);
        clear_qbuffer(&ibuff);

        size_t msgs_size = 0, has_gss = 0;
        sscanf(msgs[0].content, "%d %d", &msgs_size, &has_gss);

        printf("[log] incoming %d messages\n", msgs_size);

        for (size_t i = 1; i < msgs_size; i++){
            struct qbuffer ibuff;
            push_buffer(&cli.output_queue, &qmsg);
            await_pop(&cli.input_queue, &ibuff);
            printf("[log] got %d message\n", i);
            
            msgs = (struct proto_msg*)realloc(msgs, sizeof(struct proto_msg) * (++got_msgs));
            proto_deserial(&msgs[got_msgs - 1], &ibuff);
            proto_print(&msgs[got_msgs - 1]);
            clear_qbuffer(&ibuff);
        }
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
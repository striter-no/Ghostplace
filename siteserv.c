#include <webnet/net/tcpserv.h>
#include <webnet/site.h>

void *detached(void *args){
    struct TCP_server *serv = args;
    tcp_loop(serv);
    pthread_exit(NULL);
}

int main(){
    struct site site;
    load_site(&site, "./assets/sites", "ghost.main");
    
    // 2. Composing site to protocol messages
    // struct proto_msg *msgs = NULL;
    // compose_site(&site, &msgs);
    

    // size_t msgs_size = 0, has_gss = 0;
    // sscanf(msgs[0].content, "%d %d", &msgs_size, &has_gss);

    struct TCP_server serv;
    tcp_create(&serv, "127.0.0.1", 8520);
    tcp_listen(&serv, 1);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&serv);

    sleep(2);
    while (serv.running){
        pthread_mutex_lock(&serv.climtx);

        for (size_t i = 0; i < serv.act_clients_n; i++){
            struct __TCP_serv_cli *cli = &serv.clients[i];
            struct qbuffer ibuff;
            struct qbuffer obuff;

            int has_inp = pop_buffer(&cli->input_queue, &ibuff);
            if (has_inp != 0) continue;

            struct proto_msg inp_msg;
            proto_deserial(&inp_msg, &ibuff);

            if (inp_msg.type != GET) continue;
            if (strcmp(inp_msg.path, "/") == 0){
                printf("[log] enum requested\n");
                struct proto_msg out;
                compose_enum(&site, &out);
                proto_serial(&out, &obuff);
                proto_msg_free(&out);
            } else {
                printf("[log] path requested: %s\n", inp_msg.path);
                struct proto_msg out;
                compose_by_path(&site, inp_msg.path, &out);
                proto_print(&out);
                printf("\n---------------------------\n");
                proto_serial(&out, &obuff);
                proto_msg_free(&out);
            }
            
            proto_msg_free(&inp_msg);
            
            push_buffer(&cli->output_queue, &obuff);
            
            clear_qbuffer(&obuff);
            clear_qbuffer(&ibuff);
        }

        pthread_mutex_unlock(&serv.climtx);
    }
    tcp_end_server(&serv);

    pthread_join(main_thread, NULL);
    destroy_site(&site);
    // for (size_t i = 0; i < msgs_size + 1; i++){
    //     proto_msg_free(&msgs[i]);
    // }
    // free(msgs);
}
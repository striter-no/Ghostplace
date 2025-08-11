#include <ghpl/webnet/ghcli.h>

void request_site(
    struct site *site_out,
    struct TCP_client *cli,
    const char *main_path // a.k.a domain
){

    struct qbuffer qmsg;
    struct proto_msg enum_ask = {
        .type = GET,
        .conttype = TEXT_CONT,
        .path = NULL,
        .content = (uint8_t*)strdup(""),
        .cont_size = 1,
        .proto_ver = 0
    };

    enum_ask.path = malloc(strlen(main_path) + 2);
    memcpy(enum_ask.path, main_path, strlen(main_path));
    enum_ask.path[strlen(main_path)] = '/';
    enum_ask.path[strlen(main_path) + 1] = '\0';

    proto_serial(&enum_ask, &qmsg);
    proto_msg_free(&enum_ask);

    push_buffer(&cli->output_queue, &qmsg);
    clear_qbuffer(&qmsg);
    
    struct proto_msg *msgs = NULL;
    size_t got_msgs = 0;

    printf("[log] awaiting for messages\n");
    struct qbuffer ibuff;
    await_pop(&cli->input_queue, &ibuff);

    struct proto_msg enum_msg;
    proto_deserial(&enum_msg, &ibuff);
    clear_qbuffer(&ibuff);
    proto_print(&enum_msg);

    struct proto_msg *get_msgs = NULL;
    size_t msgs_size = 0;
    int out = get_get_messages(&enum_msg, &get_msgs, &msgs_size);
    proto_msg_free(&enum_msg);

    printf("[log]{%d} requesting %ld messages\n", out, msgs_size);
    for (size_t i = 0; i < msgs_size; i++){
        struct qbuffer obuff;
        char path_buffer[150] = {0};
        strcat(path_buffer, main_path);
        path_buffer[strlen(path_buffer)] = '/';
        strcat(path_buffer, get_msgs[i].path);
        free(get_msgs[i].path);
        get_msgs[i].path = strdup(path_buffer);

        printf("[log] requesting by path: %s\n", get_msgs[i].path);
        proto_serial(&get_msgs[i], &obuff);
        push_buffer(&cli->output_queue, &obuff);
        clear_qbuffer(&obuff);

        struct qbuffer ibuff;
        await_pop(&cli->input_queue, &ibuff);
        printf("[log] got %ld message\n", i + 1);
        
        msgs = (struct proto_msg*)realloc(msgs, sizeof(struct proto_msg) * (++got_msgs));
        proto_deserial(&msgs[i], &ibuff);
        proto_print(&msgs[i]);
        clear_qbuffer(&ibuff);
    }

    printf("[log] all got\n");

    for (size_t i = 0; i < msgs_size; i++)
        proto_msg_free(&get_msgs[i]);
    free(get_msgs);

    out = decompose_site(site_out, msgs, got_msgs);
    printf("[log]{%d} decomposing site\n", out);

    free(site_out->domain_name);
    site_out->domain_name = strdup(main_path);

    for (size_t i = 0; i < got_msgs; i++)
        proto_msg_free(&msgs[i]);
    free(msgs);
}
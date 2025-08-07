#include <webnet/proto.h>

int main(){
    struct proto_msg msg;
    proto_constr(&msg, 0, POST, TEXT_CONT);
    
    const char cmsg[] = "Hello world";
    msg.content = (uint8_t*)realloc(msg.content, strlen(cmsg) + 1);
    msg.cont_size = strlen(cmsg) + 1;
    memcpy(msg.content, cmsg, strlen(cmsg) + 1);

    const char cpath[] = "/";
    msg.path = (char*)realloc(msg.path, strlen(cpath) + 1);
    strcpy(msg.path, cpath);

    struct qbuffer ibuf;
    struct proto_msg omsg;
    proto_serial(&msg, &ibuf);
    proto_deserial(&omsg, &ibuf);

    proto_print(&omsg);

    clear_qbuffer(&ibuf);
    proto_msg_free(&omsg);
    proto_msg_free(&msg);
}
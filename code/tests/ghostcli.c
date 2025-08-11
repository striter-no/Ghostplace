#include <webnet/ghcli.h>

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
    sleep(2);

    if (cli.running){
        struct site site;
        request_site(&site, &cli, "ghost.main");
        destroy_site(&site);
    }

    tcp_cli_disconn(&cli);
    pthread_join(main_thread, NULL);
}
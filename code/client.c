#include <ghpl/console/tgr.h>
#include <ghpl/console/keyboard.h>
#include <ghpl/console/mouse.h>

#include <ghpl/web/parser.h>
#include <ghpl/web/cssparser.h>
#include <ghpl/webnet/ghcli.h>

char *site_domain = NULL;

struct Widget *main_cnt = NULL;
static struct Keyboard kb;
static struct Mouse mouse;

char router_ip[20] = {0};
int  router_port = 9000;

void *detached(void *args);
void update(struct tgr_app *app);
int update_to_site(struct tgr_app *app, const char *domain_name);

int main(int argc, char *argv[]){
    // TUI INIT

    if (argc < 3){
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }

    if (strlen(argv[1]) >= 20){
        fprintf(stderr, "[error] argv[1] (ip) is more than 19 simbols");
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }

    if (atoi(argv[2]) == 0){
        fprintf(stderr, "[error] argv[2] (port) can not be equal to zero (or be non-digital)");
        fprintf(stderr, "[error] usage: %s ip port\n", argv[0]);
        exit(-1);
    }

    strcat(router_ip, argv[1]);
    router_port = atoi(argv[2]);

    struct tgr_app app;
    tgr_init(&app);
    app.background_clr = (struct rgb){28, 28, 28};
    // app.FORCE_FPS = 60;

    mouse = (struct Mouse){0};
    create_kboard(&kb);

    update_to_site(&app, "ghost.main");

    // RUN ===================
    enable_mouse(MOUSE_NORMAL_TRACKING);
    tgr_run(&app, update);
    tgr_end(&app);
    disable_mouse(MOUSE_NORMAL_TRACKING);

    // END ===================
    if (main_cnt){
        free_widget(main_cnt);
        free(main_cnt);
    }

    free_keyboard(&kb);    
}

int update_to_site(struct tgr_app *app, const char *domain_name){
    struct TCP_client tcpcli;

    // TCP INIT
    tcp_cli_create(&tcpcli);
    tcp_cli_conect(&tcpcli, router_ip, router_port);
    
    pthread_t main_thread;
    pthread_create(&main_thread, NULL, detached, (void*)&tcpcli);
    sleep(2); // wating for main_thread to start
    
    
    mkdir_p(".tmp_sites", 0755);
    
    struct site site;
    request_site(&site, &tcpcli, domain_name);
    char tmp_site_name[100] = {0};
    sprintf(tmp_site_name, ".tmp_sites/%s", domain_name);
    // **printf("[log] tmp site name: %s\n", tmp_site_name);

    save_site(&site, ".tmp_sites");

    struct css_block *css_blocks = parse_css(site.gss_content);
    
    if (!css_blocks) {
        destroy_site(&site);
        printf("Failed to parse GSS\n");
        return 1;
    }
    
    struct tag *xml_root = parse_xml(site.ghml_content);
    if (!xml_root) {
        destroy_site(&site);
        printf("Failed to parse GHML\n");
        return 2;
    }

    destroy_site(&site);
    // ===================
    xml_addwidgets(tmp_site_name, app, &main_cnt, xml_root, 0);
    css_parsewidgets(&main_cnt, css_blocks);
    free_tag(xml_root);
    free_css(css_blocks);

    // TCP end
    tcp_cli_disconn(&tcpcli);
    pthread_join(main_thread, NULL);

    return 0;
}

void update(struct tgr_app *app){
    static char is_in_search = 0;
    mouse.scroll_h = 0;

    struct qbuffer buffer;
    if (0 == pop_buffer(&app->inp_queue, &buffer)){
        process_mouse(&mouse, (int8_t*)buffer.bytes, buffer.size);
        // CTRL + F to go to some site
        char has_kbi = kb_process_input(&kb, buffer.bytes, buffer.size);
        if (has_kbi){
            struct Key key;
            get_pressed_key(&kb, &key);
            if (key_cmp(key, keye("ctrl + f"))){
                is_in_search = !is_in_search;
            } else if (is_in_search && key_cmp(key, keye("enter"))){
                update_to_site(app, site_domain);
                
                
                free(site_domain);
                site_domain = NULL;
                is_in_search = 0;
            } else if (is_in_search && key_cmp(key, keye("bs"))){
                // tgr_fstop();
                size_t dom_size = site_domain ? strlen(site_domain): 0;
                if (dom_size != 0){
                    site_domain[dom_size - 1] = '\0';
                }
            } else if (is_in_search && key.unich <= 126) {
                size_t dom_size = site_domain ? strlen(site_domain): 0;
                site_domain = (char*)realloc(site_domain, dom_size + 2);
                site_domain[dom_size] = key.unich;
                site_domain[dom_size + 1] = '\0';
            }
        }
        
        clear_qbuffer(&buffer);
    }


    // WIDGETS =======================
    if (main_cnt){
        upd_container_focus(app, main_cnt, &mouse);
        upd_container(app, main_cnt, &mouse);
        update_positions(main_cnt);
        // main_cnt->rect = main_cnt->orig_state;
        draw_widget(app, main_cnt);
    }

    if (is_in_search){
        for (size_t i = 0; i < app->TERM_WIDTH - 1; i++){
            tgr_pixel(app, (struct rgb){255, 255, 255}, i, app->TERM_HEIGHT - 1, 0);
        }

        if (site_domain){
            int32_t *unistr;
            utf8_conv((uint8_t*)site_domain, &unistr);
            string_insert(app, unistr, 0, app->TERM_HEIGHT - 1);
            free(unistr);
        }
    }

    // FPS =======================
    char fpsbuff[60];
    sprintf(fpsbuff, "%ldx%ld\n%d", app->TERM_WIDTH, app->TERM_HEIGHT, app->fps);

    int32_t *unistr;
    utf8_conv((uint8_t*)fpsbuff, &unistr);
    rgb_string_insert(app, unistr, 0, 0, (struct rgb){150, 200, 150});
    free(unistr);
}

void *detached(void *args){
    struct TCP_client *cli = args;
    tcp_cli_run(cli);
    pthread_exit(NULL);
}

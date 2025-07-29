#include "../src/term.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>

int term_new(struct terminal *term){
    if (term == NULL) return -1;

    tcgetattr(0, &term->old_attr);
    term->new_attr = term->old_attr;
    term->new_attr.c_lflag &= ~ICANON;
    term->new_attr.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &term->new_attr);
    
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    return 0;
}

int term_dem(size_t *width, size_t *height){
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    *width = ws.ws_col;
    *height = ws.ws_row;
    return 0;
}

int term_reset(struct terminal *term){
    if (term == NULL) return -1;

    tcsetattr(0, TCSANOW, &term->old_attr);
    return 0;
}

int term_write(byte *bytes, size_t num) {
    size_t written = 0;
    while (written < num) {
        ssize_t r = write(STDOUT_FILENO, bytes + written, num - written);
        if (r == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return -1;
        }
        if (r == 0) {
            errno = EIO;
            return -1;
        }
        written += r;
    }
    return 0;
}

int term_read(byte **bytes, size_t *osize) {
    struct pollfd fds[1];
    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    int ret = poll(fds, 1, 0);
    if (ret == -1) return -1;
    if (ret == 0 || !(fds[0].revents & POLLIN)) {
        *osize = 0;
        return 1;
    }

    size_t available = 1024;
    byte *buffer = realloc(*bytes, available);
    if (!buffer) return -3;

    ssize_t rsize = read(STDIN_FILENO, buffer, available);
    if (rsize < 0) return -4;

    *bytes = buffer;
    *osize = rsize;

    return rsize > 0 ? 0: 1;
} 
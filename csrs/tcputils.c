#include <webnet/net/tcputils.h>

ssize_t full_read(int fd, void *buf, size_t count) {
    size_t total_read = 0;
    while (total_read < count) {
        ssize_t bytes_read = read(fd, (char *)buf + total_read, count - total_read);
        if (bytes_read <= 0) {
            if (bytes_read == 0) return total_read; // Конец файла
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) 
                continue; // Прервано сигналом или нет данных
            return -1; // Ошибка
        }
        total_read += bytes_read;
    }
    return total_read;
}

ssize_t full_write(int fd, const void *buf, size_t count) {
    size_t total_written = 0;
    while (total_written < count) {
        ssize_t bytes_written = write(fd, (const char *)buf + total_written, count - total_written);
        if (bytes_written <= 0) {
            if (bytes_written < 0 && (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK))
                continue;
            return -1;
        }
        total_written += bytes_written;
    }
    return total_written;
}
#include <queue.h>

void clear_queue(struct queue *line) {
    if (!line) return;
    for (size_t i = 0; i < line->size; i++) {
        free(line->pipes[i].bytes);
    }
    free(line->pipes);
    line->pipes = NULL;
    line->size = 0;
}

int push_buffer(struct queue *line, struct qbuffer *buffer) {
    if (!line || !buffer) return -1;
    
    struct qbuffer b = {0};
    b.bytes = malloc(buffer->size);
    if (!b.bytes) return -1;
    memcpy(b.bytes, buffer->bytes, buffer->size);
    b.size = buffer->size;

    struct qbuffer *pipes = realloc(line->pipes, sizeof(struct qbuffer) * (line->size + 1));
    if (!pipes) {
        free(b.bytes);
        return -1;
    }
    line->pipes = pipes;
    line->pipes[line->size++] = b;
    return 0;
}


int pop_buffer(struct queue *line, struct qbuffer *b) {
    if (!line || !b || line->size == 0)
        return 1;

    *b = line->pipes[0];

    // Сдвигаем оставшиеся элементы в начало
    if (line->size > 1) {
        memmove(line->pipes, line->pipes + 1, (line->size - 1) * sizeof(struct qbuffer));
    }

    line->size--;

    // Опционально: уменьшаем размер массива
    if (line->size > 0) {
        struct qbuffer *new_pipes = realloc(line->pipes, sizeof(struct qbuffer) * line->size);
        if (!new_pipes) {
            // Не критично, продолжаем работать с текущим размером
        } else {
            line->pipes = new_pipes;
        }
    } else {
        free(line->pipes);
        line->pipes = NULL;
    }

    return 0;
}

int forward_queue(struct queue *src, struct queue *dest) {
    struct qbuffer b;
    if (pop_buffer(src, &b) != 0) return -1;
    if (push_buffer(dest, &b) != 0) {
        free(b.bytes); // Освобождаем ресурсы
        return -2;
    }
    return 0;
}
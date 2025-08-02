#include <queue.h>

struct queue create_queue(){
    struct queue o = {0};
    pthread_mutex_init(&o.mtx, NULL);
    return o;
}

void clear_queue(struct queue *queue) {
    if (!queue) return;
    for (size_t i = 0; i < queue->size; i++) {
        free(queue->buffers[i].bytes);
    }
    free(queue->buffers);
    queue->buffers = NULL;
    queue->size = 0;
    pthread_mutex_destroy(&queue->mtx);
}

int push_buffer(struct queue *queue, struct qbuffer *buffer) {
    if (!queue || !buffer) return -1;
    pthread_mutex_lock(&queue->mtx);
    
    struct qbuffer b = {0};
    b.bytes = malloc(buffer->size);
    if (!b.bytes) return -1;
    memcpy(b.bytes, buffer->bytes, buffer->size);
    
    b.size = buffer->size;

    struct qbuffer *buffers = realloc(queue->buffers, sizeof(struct qbuffer) * (queue->size + 1));
    
    if (!buffers) {
        free(b.bytes);
        return -1;
    }
    
    queue->buffers = buffers;
    queue->buffers[queue->size++] = b;
    pthread_mutex_unlock(&queue->mtx);
    return 0;
}


int pop_buffer(struct queue *queue, struct qbuffer *b) {
    pthread_mutex_lock(&queue->mtx);
    
    if (!queue || !b || queue->size == 0) {
        pthread_mutex_unlock(&queue->mtx);
        return 1;
    }

    // Копируем данные
    *b = queue->buffers[0];
    
    // Удаляем элемент из очереди
    if (queue->size > 1) {
        memmove(queue->buffers, queue->buffers + 1, (queue->size - 1) * sizeof(struct qbuffer));
    }
    queue->size--;

    // Опционально: уменьшаем размер массива
    if (queue->size > 0) {
        // struct qbuffer *new_buffers = realloc(queue->buffers, sizeof(struct qbuffer) * queue->size);
        // if (!new_buffers) {
        //     // Не критично, продолжаем работать с текущим размером
        // } else {
        //     queue->buffers = new_buffers;
        // }
    } else {
        free(queue->buffers);
        queue->buffers = NULL;
    }

    pthread_mutex_unlock(&queue->mtx);
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

void clear_qbuffer(struct qbuffer *buff){
    free(buff->bytes);
    buff->bytes = 0;
    buff->size = 0;
}
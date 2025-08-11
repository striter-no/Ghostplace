#pragma once
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

struct qbuffer {
    uint8_t *bytes;
    size_t size;
};

struct queue {
    pthread_mutex_t mtx;

    struct qbuffer *buffers;
    size_t size;
};

struct queue create_queue();
void clear_queue(struct queue *queue);
int push_buffer(struct queue *queue, struct qbuffer *buffer);
int pop_buffer(struct queue *queue, struct qbuffer *b);
int forward_queue(struct queue *src, struct queue *dest);

void copy_qbuffer(struct qbuffer *dst, struct qbuffer *src);
void create_qbuffer(struct qbuffer *out, size_t bytes);
void clear_qbuffer(struct qbuffer *buff);
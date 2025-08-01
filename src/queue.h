#pragma once
#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct qbuffer {
    uint8_t *bytes;
    size_t size;
};

struct queue {
    struct qbuffer *pipes;
    size_t size;
};

void clear_queue(struct queue *queue);
int push_buffer(struct queue *queue, struct qbuffer *buffer);
int pop_buffer(struct queue *queue, struct qbuffer *b);
int forward_queue(struct queue *src, struct queue *dest);
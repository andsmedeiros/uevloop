
#include "circular-queue.h"

void cqueue_init(cqueue_t *queue, void **buffer, uintptr_t size_log2n){
    queue->buffer = buffer;
    queue->size = 1<<size_log2n;
    queue->mask = queue->size - 1;
    cqueue_clear(queue, false);
}

void cqueue_clear(cqueue_t *queue, bool clear_buffer){
    queue->tail = 0;
    queue->count = 0;
    if(clear_buffer){
        queue->buffer = NULL;
        queue->size = 0;
        queue->mask = 0;
    }
}

bool cqueue_push(cqueue_t *queue, void *element){
    if(cqueue_is_full(queue)) return false;

    const uintptr_t head = (++queue->count + queue->tail) & queue->mask;
    queue->buffer[head] = element;
    return true;
}

void *cqueue_pop(cqueue_t *queue){
    if(cqueue_is_empty(queue)) return NULL;

    queue->count--;
    queue->tail = (queue->tail + 1) & queue->mask;
    void *element = queue->buffer[queue->tail];
    queue->buffer[queue->tail] = NULL;
    return element;
}

void *cqueue_peek_tail(cqueue_t *queue){
    if(cqueue_is_empty(queue)) return NULL;

    return queue->buffer[(queue->tail + 1) & queue->mask];
}

void *cqueue_peek_head(cqueue_t *queue){
    if(cqueue_is_empty(queue)) return NULL;

    return queue->buffer[(queue->tail + queue->count) & queue->mask];
}

bool cqueue_is_full(cqueue_t *queue){
    return queue->size <= queue->count;
}

bool cqueue_is_empty(cqueue_t *queue){
    return queue->count == 0;
}

uintptr_t cqueue_count(cqueue_t *queue){
    return queue->count;
}

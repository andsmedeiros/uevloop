#include "uevloop/utils/circular-queue.h"

/// \cond
#include <stdlib.h>
/// \endcond

void uel_cqueue_init(uel_cqueue_t *queue, void **buffer, uintptr_t size_log2n){
    queue->buffer = buffer;
    queue->size = 1<<size_log2n;
    queue->mask = queue->size - 1;
    uel_cqueue_clear(queue, false);
}

void uel_cqueue_clear(uel_cqueue_t *queue, bool clear_buffer){
    queue->tail = 0;
    queue->count = 0;
    if(clear_buffer){
        queue->buffer = NULL;
        queue->size = 0;
        queue->mask = 0;
    }
}

bool uel_cqueue_push(uel_cqueue_t *queue, void *element){
    if(uel_cqueue_is_full(queue)) return false;

    const uintptr_t head = (++queue->count + queue->tail) & queue->mask;
    queue->buffer[head] = element;
    return true;
}

void *uel_cqueue_pop(uel_cqueue_t *queue){
    if(uel_cqueue_is_empty(queue)) return NULL;

    queue->count--;
    queue->tail = (queue->tail + 1) & queue->mask;
    void *element = queue->buffer[queue->tail];
    queue->buffer[queue->tail] = NULL;
    return element;
}

void *uel_cqueue_peek_tail(uel_cqueue_t *queue){
    if(uel_cqueue_is_empty(queue)) return NULL;

    return queue->buffer[(queue->tail + 1) & queue->mask];
}

void *uel_cqueue_peek_head(uel_cqueue_t *queue){
    if(uel_cqueue_is_empty(queue)) return NULL;

    return queue->buffer[(queue->tail + queue->count) & queue->mask];
}

bool uel_cqueue_is_full(uel_cqueue_t *queue){
    return queue->size <= queue->count;
}

bool uel_cqueue_is_empty(uel_cqueue_t *queue){
    return queue->count == 0;
}

uintptr_t uel_cqueue_count(uel_cqueue_t *queue){
    return queue->count;
}

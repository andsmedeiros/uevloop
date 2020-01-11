#include "object-pool.h"

void objpool_init(
    objpool_t *pool,
    uintptr_t size_log2n,
    uintptr_t item_size,
    uint8_t *buffer,
    void **queue_buffer
){
    pool->buffer = buffer;
    cqueue_init(&pool->queue, queue_buffer, size_log2n);
    uintptr_t i;
    for(i = 0; i < pool->queue.size; i++){
        cqueue_push(&pool->queue, (void *)(&pool->buffer[i * item_size]));
    }
}

void *objpool_acquire(objpool_t *pool){
    return cqueue_pop(&pool->queue);
}

bool objpool_release(objpool_t *pool, void *element){
    return cqueue_push(&pool->queue, element);
}

bool objpool_is_empty(objpool_t *pool){
    return cqueue_is_empty(&pool->queue);
}

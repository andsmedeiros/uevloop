#include "object-pool.h"

void uel_objpool_init(
    uel_objpool_t *pool,
    uintptr_t size_log2n,
    uintptr_t item_size,
    uint8_t *buffer,
    void **queue_buffer
){
    pool->buffer = buffer;
    uel_cqueue_init(&pool->queue, queue_buffer, size_log2n);
    uintptr_t i;
    for(i = 0; i < pool->queue.size; i++){
        uel_cqueue_push(&pool->queue, (void *)(&pool->buffer[i * item_size]));
    }
}

void *uel_objpool_acquire(uel_objpool_t *pool){
    return uel_cqueue_pop(&pool->queue);
}

bool uel_objpool_release(uel_objpool_t *pool, void *element){
    return uel_cqueue_push(&pool->queue, element);
}

bool uel_objpool_is_empty(uel_objpool_t *pool){
    return uel_cqueue_is_empty(&pool->queue);
}

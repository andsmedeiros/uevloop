#include "uevloop/utils/automatic-pool.h"

void uel_autoptr_dealloc(uel_autoptr_t *autoptr){
    uel_objpool_release(autoptr->source, autoptr);
}

void uel_autopool_init(
    uel_autopool_t *pool,
    size_t size_log2n,
    size_t item_size,
    uint8_t *object_buffer,
    uel_autoptr_t *autoptr_buffer,
    void **queue_buffer
){
    for (size_t i = 0; i < (1<<size_log2n); i++) {
        autoptr_buffer[i].object = (void *)(object_buffer + i * item_size);
        autoptr_buffer[i].source = &pool->autoptr_pool;
    }
    uel_objpool_init(
        &pool->autoptr_pool,
        size_log2n,
        sizeof(uel_autoptr_t),
        (uint8_t *)autoptr_buffer,
        queue_buffer
    );
}

uel_autoptr_t *uel_autopool_alloc(uel_autopool_t *pool){
    return uel_objpool_acquire(&pool->autoptr_pool);
}

bool uel_autopool_is_empty(uel_autopool_t *pool){
    return uel_objpool_is_empty(&pool->autoptr_pool);
}

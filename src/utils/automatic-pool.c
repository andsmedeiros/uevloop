#include "uevloop/utils/automatic-pool.h"

static void *nop(uel_closure_t *closure) { return NULL; }

void uel_autoptr_dealloc(uel_autoptr_t autoptr) {
    struct uel_autoptr *ptr = (struct uel_autoptr *)autoptr;
    uel_closure_invoke(&ptr->source->destructor, *autoptr);
    uel_objpool_release(&ptr->source->autoptr_pool, autoptr);
}

void uel_autopool_init(
    uel_autopool_t *pool,
    size_t size_log2n,
    size_t item_size,
    uint8_t *object_buffer,
    struct uel_autoptr *autoptr_buffer,
    void **queue_buffer
){
    for (size_t i = 0; i < (1<<size_log2n); i++) {
        autoptr_buffer[i].object = (void *)(object_buffer + i * item_size);
        autoptr_buffer[i].source = pool;
    }
    uel_objpool_init(
        &pool->autoptr_pool,
        size_log2n,
        sizeof(struct uel_autoptr),
        (uint8_t *)autoptr_buffer,
        queue_buffer
    );
    pool->constructor = uel_closure_create(nop, NULL, NULL);
    pool->destructor = uel_closure_create(nop, NULL, NULL);
}

uel_autoptr_t uel_autopool_alloc(uel_autopool_t *pool){
    uel_autoptr_t autoptr =
        (uel_autoptr_t)uel_objpool_acquire(&pool->autoptr_pool);
    uel_closure_invoke(&pool->constructor, *autoptr);
    return autoptr;
}

bool uel_autopool_is_empty(uel_autopool_t *pool){
    return uel_objpool_is_empty(&pool->autoptr_pool);
}

void uel_autopool_set_constructor(uel_autopool_t *pool, uel_closure_t constructor) {
    pool->constructor = constructor;
}

void uel_autopool_set_destructor(uel_autopool_t *pool, uel_closure_t destructor) {
    pool->destructor = destructor;
}

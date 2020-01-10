#ifndef OBJECT_POOL_H
#define	OBJECT_POOL_H

#include "circular-queue.h"
#include <stdlib.h>
#include <stdint.h>

typedef struct objpool objpool_t;
struct objpool {
    uint8_t *buffer;
    cqueue_t queue;
};

void objpool_init(
    objpool_t *pool,
    size_t size_log2n,
    size_t item_size,
    uint8_t *buffer,
    void **queue_buffer
);

void *objpool_acquire(objpool_t *pool);
bool objpool_release(objpool_t *pool, void *element);

bool objpool_is_empty(objpool_t *pool);


#define DECLARE_OBJPOOL_BUFFERS(type, size_log2n, id)           \
    uint8_t id##_pool_buffer[(1<<size_log2n) * sizeof(type)];   \
    void *id##_pool_queue_buffer[1<<size_log2n]

#define OBJPOOL_BUFFERS(id)                                     \
    id##_pool_buffer, id##_pool_queue_buffer

#define OBJPOOL_BUFFERS_IN(id, obj)                             \
    obj.id##_pool_buffer, obj.id##_pool_queue_buffer

#define OBJPOOL_BUFFERS_AT(id, obj)                             \
    obj->id##_pool_buffer, obj->id##_pool_queue_buffer

#endif	/* OBJECT_POOL_H */

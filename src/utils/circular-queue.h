#ifndef CIRCULAR_QUEUE_H
#define	CIRCULAR_QUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct cqueue cqueue_t;
struct cqueue {
    void **buffer;
    uintptr_t size;
    uintptr_t mask;
    uintptr_t tail;
    uintptr_t count;
};

void cqueue_init(cqueue_t *queue, void **buffer, uintptr_t size_log2n);
void cqueue_clear(cqueue_t *queue, bool clear_buffer);

bool cqueue_push(cqueue_t *queue, void *element);
void *cqueue_pop(cqueue_t *queue);
void *cqueue_peek_tail(cqueue_t *queue);
void *cqueue_peek_head(cqueue_t *queue);

bool cqueue_is_full(cqueue_t *queue);
bool cqueue_is_empty(cqueue_t *queue);

#endif	/* CIRCULAR_QUEUE_H */

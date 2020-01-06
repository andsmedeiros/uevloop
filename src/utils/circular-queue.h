/* 
 * File:   circular-queue.h
 * Author: kazeshi
 *
 * Created on January 3, 2020, 11:17 AM
 */

#ifndef CIRCULAR_QUEUE_H
#define	CIRCULAR_QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct cqueue cqueue_t;
struct cqueue {
    void **buffer;
    size_t size;
    size_t mask;
    size_t tail;
    size_t count;
};

void cqueue_init(cqueue_t *queue, void **buffer, size_t size_log2n);
void cqueue_clear(cqueue_t *queue, bool clear_buffer);

bool cqueue_push(cqueue_t *queue, void *element);
void *cqueue_pop(cqueue_t *queue);
void *cqueue_peek_tail(cqueue_t *queue);
void *cqueue_peek_head(cqueue_t *queue);

bool cqueue_is_full(cqueue_t *queue);
bool cqueue_is_empty(cqueue_t *queue);

#endif	/* CIRCULAR_QUEUE_H */


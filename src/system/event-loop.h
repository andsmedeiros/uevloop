#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <stdbool.h>
#include "../utils/circular-queue.h"
#include "../utils/object-pool.h"
#include "../utils/closure.h"
#include "../system/event.h"

typedef struct evloop evloop_t;
struct evloop{
    objpool_t *event_pool;
    cqueue_t *event_queue;
    cqueue_t *reschedule_queue;
};

void evloop_init(
    evloop_t *event_loop,
    objpool_t *event_pool,
    cqueue_t *event_queue,
    cqueue_t *reschedule_queue
);
void evloop_run(evloop_t *event_loop);
void evloop_enqueue_event(evloop_t *event_loop, event_t *event);
void evloop_enqueue_closure(evloop_t *event_loop, closure_t *closure);

#endif /* end of include guard: EVENT_LOOP_H */

#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include <stdint.h>
#include "event.h"
#include "../utils/linked-list.h"
#include "../utils/closure.h"
#include "../utils/object-pool.h"
#include "../utils/circular-queue.h"


typedef struct scheduler scheduler_t;
struct scheduler{
    /* This linked list holds events/timers scheduled to be run in the future.*/
    /* Timers are inserted sorted by its due time, so the list is always in */
    /* execution order. */
    /* Expired timers may be returned here if they are repeating. */
    llist_t timer_list;

    objpool_t *llist_node_pool;
    objpool_t *event_pool;

    cqueue_t *event_queue;
    cqueue_t *reschedule_queue;

    /* Internal timer. Must be updated via evloop_update_timer() */
    volatile uint32_t timer;
};

void sch_init(
    scheduler_t *scheduler,
    objpool_t *llist_node_pool,
    objpool_t *event_pool,
    cqueue_t *event_queue,
    cqueue_t *reschedule_queue
);
void sch_run_later(
    scheduler_t *scheduler,
    uint16_t  timeout_in_ms,
    closure_t closure
);
void sch_run_at_intervals(
    scheduler_t *scheduler,
    uint16_t interval_in_ms,
    bool immediate,
    closure_t closure
);

void sch_manage_timers(scheduler_t *scheduler);
void sch_reschedule(scheduler_t *scheduler, event_t *timer);
void sch_update_timer(scheduler_t *scheduler, uint32_t timer);

#endif	/* SCHEDULER_H */

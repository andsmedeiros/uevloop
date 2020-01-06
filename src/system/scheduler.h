/* 
 * File:   scheduler.h
 * Author: kazeshi
 *
 * Created on January 3, 2020, 10:48 AM
 */

#ifndef SCHEDULER_H
#define	SCHEDULER_H

#include <stdint.h>
#include "pools.h"
#include "../utils/circular-queue.h"
#include "../utils/linked-list.h"
#include "../utils/closure.h"

#define SCHEDULER_EVENT_QUEUE_SIZE_LOG2N (5)

#define SCHEDULER_EVENT_QUEUE_SIZE (1<<SCHEDULER_EVENT_QUEUE_SIZE_LOG2N)

typedef struct scheduler scheduler_t;
struct scheduler{
    
    /* This queue holds events ready for processing on the next run loop */
    void *event_queue_buffer[SCHEDULER_EVENT_QUEUE_SIZE];
    cqueue_t event_queue;
    
    /* This linked list holds events/timers scheduled to be run in the future.*/
    /* Timers are inserted sorted by its due time, so the list is always in */
    /* execution order. */
    /* Expired timers may be returned here if they are repeating. */
    llist_t timer_list;
    
    /* Holds execution time in milliseconds. */
    /* Must be updated at 1kHz by an external timer set at sch_init() */
    uint32_t timer;
    
    /* Easy reference to the consumed object pools. */
    objpool_t *event_pool;
    objpool_t *llist_node_pool;
};

void sch_tick(scheduler_t *scheduler);
void sch_init(
    scheduler_t *scheduler, 
    pools_t *pools, 
    void (*on_timer_tick)(closure_t)
);
void sch_enqueue(scheduler_t *scheduler, closure_t closure);
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

#endif	/* SCHEDULER_H */


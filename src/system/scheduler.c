#include "scheduler.h"
#include "event.h"
#include "../portability/critical-section.h"

static void *is_past_due_time(closure_t *closure){
    uint32_t current_time = *(uint32_t *)closure->context;
    llist_node_t *node = (llist_node_t *)closure->params;
    event_t *event = (event_t *)node->value;
    bool fit_for_removal = event->timer.due_time <= current_time;
    return (void *)fit_for_removal;
}

static void *place_in_order(closure_t *closure){
    uint32_t due_time = *(uint32_t *)closure->context;
    llist_node_t **nodes = (llist_node_t **)closure->params;

    bool fits = false;
    if(nodes[1] == NULL){
        fits = true;
    }else if(nodes[0] == NULL){
        event_t *next = (event_t *)nodes[1]->value;
        fits = next->timer.due_time > due_time;
    }else{
        event_t *prev = (event_t *)nodes[0]->value;
        event_t *next = (event_t *)nodes[1]->value;

        fits = prev->timer.due_time <= due_time &&
            next->timer.due_time > due_time;
    }

    return (void *)(uintptr_t)fits;
}

static void enqueue_timer(scheduler_t *scheduler, event_t *timer){
    llist_node_t *node = syspools_acquire_llist_node(scheduler->pools);
    node->value = (void *)timer;
    closure_t in_order = closure_create(
        place_in_order,
        (void *)&timer->timer.due_time,
        NULL
    );
    llist_insert_at(&scheduler->timer_list, node, &in_order);
}

static void schedule(scheduler_t *scheduler, event_t *timer){
    UEVLOOP_CRITICAL_ENTER;
    cqueue_push(scheduler->schedule_queue, (void *)timer);
    UEVLOOP_CRITICAL_EXIT;
}

void sch_init(
    scheduler_t *scheduler,
    syspools_t *pools,
    cqueue_t *event_queue,
    cqueue_t *schedule_queue
){
    llist_init(&scheduler->timer_list);
    scheduler->pools = pools;
    scheduler->event_queue = event_queue;
    scheduler->schedule_queue = schedule_queue;
    scheduler->timer = 0;
}

void sch_run_later(scheduler_t *scheduler, uint16_t  timeout_in_ms, closure_t closure){
    event_t *event = syspools_acquire_event(scheduler->pools);
    event_config_timer(event, timeout_in_ms, false, false, &closure, scheduler->timer);
    schedule(scheduler, event);
}

void sch_run_at_intervals(
    scheduler_t *scheduler,
    uint16_t interval_in_ms,
    bool immediate,
    closure_t closure
){
    event_t *event = syspools_acquire_event(scheduler->pools);
    event_config_timer(event, interval_in_ms, true, immediate, &closure, scheduler->timer);
    if(immediate){
        event->timer.due_time = scheduler->timer;
    }
    schedule(scheduler, event);
}

void sch_manage_timers(scheduler_t *scheduler){
    event_t *event;
    while((event = (event_t *)cqueue_pop(scheduler->schedule_queue)) != NULL){
        enqueue_timer(scheduler, event);
    }

    closure_t closure =
        closure_create(&is_past_due_time, (void *)&scheduler->timer, NULL);
    llist_t expired_timers = llist_remove_until(&scheduler->timer_list, &closure);
    llist_node_t *current = expired_timers.tail, *previous;
    while(current != NULL){
        event_t *timer = (event_t *)current->value;
        previous = current;
        current = current->next;
        syspools_release_llist_node(scheduler->pools, previous);
        cqueue_push(scheduler->event_queue, (void *)timer);
    }
}

void sch_update_timer(scheduler_t *scheduler, uint32_t timer){
    scheduler->timer = timer;
}
